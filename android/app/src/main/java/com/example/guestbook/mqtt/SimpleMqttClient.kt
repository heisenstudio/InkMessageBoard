package com.example.guestbook.mqtt

import java.io.ByteArrayOutputStream
import java.io.DataInputStream
import java.io.EOFException
import java.io.IOException
import java.net.SocketTimeoutException
import java.nio.charset.StandardCharsets
import java.util.concurrent.atomic.AtomicBoolean
import javax.net.ssl.SSLSocket
import javax.net.ssl.SSLSocketFactory

data class SimpleMqttConfig(
    val host: String,
    val port: Int,
    val clientId: String,
    val username: String,
    val password: String,
    val keepAliveSeconds: Int = 60,
    val willTopic: String? = null,
    val willPayload: String? = null,
    val willRetain: Boolean = false,
    val willQos: Int = 1,
)

class SimpleMqttClient(
    private val config: SimpleMqttConfig,
    private val listener: Listener,
) {
    interface Listener {
        fun onConnected()
        fun onDisconnected(reason: String?)
        fun onMessage(topic: String, payload: String)
        fun onError(error: Throwable)
    }

    private val running = AtomicBoolean(false)
    private val writeLock = Any()
    private var worker: Thread? = null
    private var socket: SSLSocket? = null
    private var input: DataInputStream? = null
    private var packetId = 1
    private var lastWriteMs = 0L

    fun connect(subscriptions: List<String>) {
        if (!running.compareAndSet(false, true)) return
        worker = Thread({
            try {
                openSocket()
                sendConnect()
                readConnAck()
                if (subscriptions.isNotEmpty()) {
                    sendSubscribe(subscriptions)
                    readSubAck()
                }
                listener.onConnected()
                readLoop()
            } catch (error: Throwable) {
                if (running.get()) {
                    listener.onError(error)
                    listener.onDisconnected(error.message)
                }
            } finally {
                closeSocket()
                running.set(false)
            }
        }, "guestbook-mqtt").apply {
            isDaemon = true
            start()
        }
    }

    fun publish(topic: String, payload: String, qos: Int = 1, retain: Boolean = false): Boolean {
        if (!running.get()) return false
        return try {
            val payloadBytes = payload.toByteArray(StandardCharsets.UTF_8)
            val body = ByteArrayOutputStream()
            writeMqttString(body, topic)
            val publishQos = qos.coerceIn(0, 1)
            if (publishQos == 1) {
                writeShort(body, nextPacketId())
            }
            body.write(payloadBytes)

            val fixedHeader = 0x30 or (publishQos shl 1) or if (retain) 0x01 else 0x00
            writePacket(fixedHeader, body.toByteArray())
            true
        } catch (error: Throwable) {
            listener.onError(error)
            disconnect()
            false
        }
    }

    fun disconnect() {
        if (!running.getAndSet(false)) return
        try {
            writePacket(0xE0, byteArrayOf())
        } catch (_: Throwable) {
        }
        closeSocket()
    }

    private fun openSocket() {
        val created = SSLSocketFactory.getDefault().createSocket(config.host, config.port) as SSLSocket
        created.soTimeout = 1000
        created.startHandshake()
        socket = created
        input = DataInputStream(created.inputStream)
    }

    private fun sendConnect() {
        val body = ByteArrayOutputStream()
        writeMqttString(body, "MQTT")
        body.write(4)

        val hasWill = !config.willTopic.isNullOrBlank() && config.willPayload != null
        var flags = 0x02
        if (hasWill) {
            flags = flags or 0x04 or ((config.willQos.coerceIn(0, 2)) shl 3)
            if (config.willRetain) flags = flags or 0x20
        }
        if (config.password.isNotEmpty()) flags = flags or 0x40
        if (config.username.isNotEmpty()) flags = flags or 0x80
        body.write(flags)

        writeShort(body, config.keepAliveSeconds)
        writeMqttString(body, config.clientId)
        if (hasWill) {
            writeMqttString(body, config.willTopic.orEmpty())
            writeMqttString(body, config.willPayload.orEmpty())
        }
        if (config.username.isNotEmpty()) writeMqttString(body, config.username)
        if (config.password.isNotEmpty()) writeMqttString(body, config.password)

        writePacket(0x10, body.toByteArray())
    }

    private fun readConnAck() {
        val packet = readPacket()
        if (packet.type != 2 || packet.body.size < 2) {
            throw IOException("MQTT CONNACK missing")
        }
        val code = packet.body[1].toInt() and 0xFF
        if (code != 0) {
            throw IOException("MQTT connect failed: $code")
        }
    }

    private fun sendSubscribe(topics: List<String>) {
        val body = ByteArrayOutputStream()
        writeShort(body, nextPacketId())
        topics.forEach { topic ->
            writeMqttString(body, topic)
            body.write(1)
        }
        writePacket(0x82, body.toByteArray())
    }

    private fun readSubAck() {
        val packet = readPacket()
        if (packet.type != 9 || packet.body.size < 3) {
            throw IOException("MQTT SUBACK missing")
        }
        val returnCodes = packet.body.drop(2).map { it.toInt() and 0xFF }
        if (returnCodes.any { it == 0x80 }) {
            throw IOException("MQTT subscribe failed")
        }
    }

    private fun readLoop() {
        while (running.get()) {
            try {
                val packet = readPacket()
                when (packet.type) {
                    3 -> handlePublish(packet.header, packet.body)
                    13 -> Unit
                    else -> Unit
                }
            } catch (_: SocketTimeoutException) {
                maybePing()
            } catch (_: EOFException) {
                if (running.get()) listener.onDisconnected("connection closed")
                running.set(false)
            }
        }
        listener.onDisconnected(null)
    }

    private fun handlePublish(header: Int, body: ByteArray) {
        if (body.size < 2) return
        var offset = 0
        val topicLength = readUnsignedShort(body, offset)
        offset += 2
        if (offset + topicLength > body.size) return
        val topic = String(body, offset, topicLength, StandardCharsets.UTF_8)
        offset += topicLength

        val qos = (header shr 1) and 0x03
        var incomingPacketId = 0
        if (qos > 0) {
            if (offset + 2 > body.size) return
            incomingPacketId = readUnsignedShort(body, offset)
            offset += 2
        }

        val payload = String(body, offset, body.size - offset, StandardCharsets.UTF_8)
        listener.onMessage(topic, payload)
        if (qos == 1) {
            val ack = byteArrayOf(
                (incomingPacketId shr 8).toByte(),
                incomingPacketId.toByte(),
            )
            writePacket(0x40, ack)
        }
    }

    private fun maybePing() {
        val now = System.currentTimeMillis()
        if (now - lastWriteMs >= (config.keepAliveSeconds * 1000L / 2L)) {
            writePacket(0xC0, byteArrayOf())
        }
    }

    private fun readPacket(): Packet {
        val stream = input ?: throw IOException("MQTT input closed")
        val header = stream.readUnsignedByte()
        val remainingLength = readRemainingLength(stream)
        val body = ByteArray(remainingLength)
        stream.readFully(body)
        return Packet(header, header shr 4, body)
    }

    private fun writePacket(header: Int, body: ByteArray) {
        val stream = socket?.outputStream ?: throw IOException("MQTT output closed")
        synchronized(writeLock) {
            stream.write(header)
            stream.write(encodeRemainingLength(body.size))
            stream.write(body)
            stream.flush()
            lastWriteMs = System.currentTimeMillis()
        }
    }

    private fun nextPacketId(): Int {
        val id = packetId
        packetId += 1
        if (packetId > 0xFFFF) packetId = 1
        return id
    }

    private fun closeSocket() {
        try {
            socket?.close()
        } catch (_: Throwable) {
        }
        input = null
        socket = null
    }

    private data class Packet(val header: Int, val type: Int, val body: ByteArray)
}

private fun writeMqttString(output: ByteArrayOutputStream, value: String) {
    val bytes = value.toByteArray(StandardCharsets.UTF_8)
    writeShort(output, bytes.size)
    output.write(bytes)
}

private fun writeShort(output: ByteArrayOutputStream, value: Int) {
    output.write((value shr 8) and 0xFF)
    output.write(value and 0xFF)
}

private fun readUnsignedShort(bytes: ByteArray, offset: Int): Int {
    return ((bytes[offset].toInt() and 0xFF) shl 8) or (bytes[offset + 1].toInt() and 0xFF)
}

private fun encodeRemainingLength(length: Int): ByteArray {
    val output = ByteArrayOutputStream()
    var value = length
    do {
        var encoded = value % 128
        value /= 128
        if (value > 0) encoded = encoded or 0x80
        output.write(encoded)
    } while (value > 0)
    return output.toByteArray()
}

private fun readRemainingLength(input: DataInputStream): Int {
    var multiplier = 1
    var value = 0
    var encoded: Int
    do {
        encoded = input.readUnsignedByte()
        value += (encoded and 127) * multiplier
        multiplier *= 128
        if (multiplier > 128 * 128 * 128 * 128) {
            throw IOException("MQTT remaining length malformed")
        }
    } while ((encoded and 128) != 0)
    return value
}
