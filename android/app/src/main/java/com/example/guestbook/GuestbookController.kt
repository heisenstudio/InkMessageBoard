package com.example.guestbook

import android.content.Context
import android.content.SharedPreferences
import android.os.Handler
import android.os.Looper
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import com.example.guestbook.mqtt.SimpleMqttClient
import com.example.guestbook.mqtt.SimpleMqttConfig
import org.json.JSONObject
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import java.util.UUID
import java.util.concurrent.Executors

data class GuestbookConfig(
    val host: String = "your-emqx-cloud-host.emqxsl.com",
    val port: String = "8883",
    val username: String = "",
    val password: String = "",
    val deviceId: String = "001",
    val clientId: String = "guestbook-phone-${UUID.randomUUID().toString().take(8)}",
) {
    val resolvedPort: Int
        get() = port.toIntOrNull()?.coerceIn(1, 65535) ?: 8883

    val msgTopic: String
        get() = "ink/$deviceId/msg_to_esp32"

    val cmdTopic: String
        get() = "ink/$deviceId/cmd_to_esp32"

    val statusTopic: String
        get() = "ink/$deviceId/status_to_phone"
}

data class DeviceStatus(
    val online: Boolean? = null,
    val battery: Int? = null,
    val rssi: Int? = null,
    val firmware: String? = null,
    val updatedAt: String? = null,
)

data class MessageRecord(
    val id: String,
    val text: String,
    val state: String,
    val replyText: String? = null,
    val createdAt: String,
)

data class EventRecord(
    val time: String,
    val title: String,
    val detail: String,
)

class GuestbookController(context: Context) {
    private val appContext = context.applicationContext
    private val mainHandler = Handler(Looper.getMainLooper())
    private val prefs: SharedPreferences =
        appContext.getSharedPreferences("guestbook", Context.MODE_PRIVATE)
    private val timeFormat = SimpleDateFormat("HH:mm:ss", Locale.getDefault())
    private val publishExecutor = Executors.newSingleThreadExecutor { runnable ->
        Thread(runnable, "guestbook-mqtt-publish").apply { isDaemon = true }
    }
    private var mqttClient: SimpleMqttClient? = null

    var config by mutableStateOf(loadConfig())
        private set

    var draftText by mutableStateOf("")
    var connectionText by mutableStateOf("未连接")
        private set
    var connectionDetail by mutableStateOf("准备连接 MQTT")
        private set
    var isConnected by mutableStateOf(false)
        private set
    var deviceStatus by mutableStateOf(DeviceStatus())
        private set

    val messages = mutableStateListOf<MessageRecord>()
    val events = mutableStateListOf<EventRecord>()

    fun connect() {
        disconnect(notify = false)
        connectionText = "连接中"
        connectionDetail = "${config.host}:${config.resolvedPort}"
        addEvent("MQTT", "连接 ${config.host}:${config.resolvedPort}")

        val mqttConfig = SimpleMqttConfig(
            host = config.host.trim(),
            port = config.resolvedPort,
            clientId = config.clientId.trim().ifBlank { GuestbookConfig().clientId },
            username = config.username.trim(),
            password = config.password,
            keepAliveSeconds = 60,
            willTopic = null,
        )

        mqttClient = SimpleMqttClient(
            config = mqttConfig,
            listener = object : SimpleMqttClient.Listener {
                override fun onConnected() {
                    post {
                        isConnected = true
                        connectionText = "已连接"
                        connectionDetail = "订阅 ${config.statusTopic}"
                        addEvent("MQTT", "订阅状态 topic 成功")
                    }
                }

                override fun onDisconnected(reason: String?) {
                    post {
                        isConnected = false
                        connectionText = "未连接"
                        connectionDetail = reason ?: "连接已关闭"
                        if (reason != null) addEvent("MQTT", "断开：$reason")
                    }
                }

                override fun onMessage(topic: String, payload: String) {
                    post {
                        handleStatusPayload(payload)
                    }
                }

                override fun onError(error: Throwable) {
                    post {
                        isConnected = false
                        connectionText = "连接异常"
                        connectionDetail = error.message ?: error.javaClass.simpleName
                        addEvent("错误", connectionDetail)
                    }
                }
            },
        )
        mqttClient?.connect(listOf(config.statusTopic))
    }

    fun disconnect() {
        disconnect(notify = true)
    }

    fun close() {
        disconnect(notify = false)
        publishExecutor.shutdownNow()
    }

    fun updateConfig(next: GuestbookConfig) {
        config = next.copy(
            host = next.host.trim(),
            port = next.port.trim(),
            username = next.username.trim(),
            deviceId = next.deviceId.trim().ifBlank { "001" },
            clientId = next.clientId.trim().ifBlank { GuestbookConfig().clientId },
        )
        saveConfig(config)
        addEvent("设置", "已保存 ${config.deviceId}")
    }

    fun sendDraft() {
        val text = draftText.trim()
        if (text.isBlank()) {
            addEvent("留言", "内容为空")
            return
        }
        sendMessage(text)
        draftText = ""
    }

    fun useQuickText(text: String) {
        draftText = text
    }

    fun sendMessage(text: String) {
        val id = "phone-${System.currentTimeMillis()}"
        val topic = config.msgTopic
        val client = mqttClient
        val payload = JSONObject()
            .put("id", id)
            .put("type", "guestbook")
            .put("text", text)
            .put("ts", System.currentTimeMillis() / 1000L)
            .toString()

        if (!isConnected || client == null) {
            addEvent("留言", "发送失败，MQTT 未连接")
            return
        }

        addEvent("留言", "正在发送 $id")
        publishExecutor.execute {
            val sent = client.publish(topic, payload, qos = 1)
            post {
                if (sent) {
                    messages.add(0, MessageRecord(id, text, "sent", createdAt = now()))
                    trimMessages()
                    addEvent("留言", "已发送 $id")
                } else {
                    addEvent("留言", "发送失败，MQTT 未连接")
                }
            }
        }
    }

    fun sendCommand(command: String) {
        val id = "cmd-${System.currentTimeMillis()}"
        val topic = config.cmdTopic
        val client = mqttClient
        val payload = JSONObject()
            .put("id", id)
            .put("cmd", command)
            .toString()

        if (!isConnected || client == null) {
            addEvent("命令", "$command 发送失败")
            return
        }

        addEvent("命令", "$command 正在发送")
        publishExecutor.execute {
            val sent = client.publish(topic, payload, qos = 1)
            post {
                addEvent("命令", if (sent) "$command 已发送" else "$command 发送失败")
            }
        }
    }

    private fun handleStatusPayload(payload: String) {
        addEvent("状态", payload)
        val json = runCatching { JSONObject(payload) }.getOrNull()
        if (json == null) {
            addEvent("状态", "JSON 解析失败")
            return
        }

        if (json.has("online") || json.has("battery") || json.has("rssi") || json.has("fw")) {
            deviceStatus = deviceStatus.copy(
                online = if (json.has("online")) json.optBoolean("online") else deviceStatus.online,
                battery = if (json.has("battery")) json.optInt("battery") else deviceStatus.battery,
                rssi = if (json.has("rssi")) json.optInt("rssi") else deviceStatus.rssi,
                firmware = if (json.has("fw")) json.optString("fw") else deviceStatus.firmware,
                updatedAt = now(),
            )
        }

        val msgId = json.optString("msg_id", "")
        val state = json.optString("state", "")
        if (msgId.isNotBlank() && state.isNotBlank()) {
            val replyText = json.optString("text", "").ifBlank { null }
            updateMessageState(msgId, state, replyText)
        }
    }

    private fun updateMessageState(id: String, state: String, replyText: String?) {
        val index = messages.indexOfFirst { it.id == id }
        if (index >= 0) {
            val old = messages[index]
            messages[index] = old.copy(state = state, replyText = replyText ?: old.replyText)
        } else {
            messages.add(
                0,
                MessageRecord(
                    id = id,
                    text = "来自设备的状态回执",
                    state = state,
                    replyText = replyText,
                    createdAt = now(),
                ),
            )
        }
        trimMessages()
    }

    private fun disconnect(notify: Boolean) {
        mqttClient?.disconnect()
        mqttClient = null
        isConnected = false
        if (notify) {
            connectionText = "未连接"
            connectionDetail = "手动断开"
            addEvent("MQTT", "手动断开")
        }
    }

    private fun post(block: () -> Unit) {
        mainHandler.post(block)
    }

    private fun addEvent(title: String, detail: String) {
        events.add(0, EventRecord(now(), title, detail))
        while (events.size > 40) events.removeLast()
    }

    private fun trimMessages() {
        while (messages.size > 30) messages.removeLast()
    }

    private fun now(): String = timeFormat.format(Date())

    private fun loadConfig(): GuestbookConfig {
        val defaults = GuestbookConfig()
        return GuestbookConfig(
            host = prefs.getString("host", defaults.host) ?: defaults.host,
            port = prefs.getString("port", defaults.port) ?: defaults.port,
            username = prefs.getString("username", defaults.username) ?: defaults.username,
            password = prefs.getString("password", defaults.password) ?: defaults.password,
            deviceId = prefs.getString("deviceId", defaults.deviceId) ?: defaults.deviceId,
            clientId = prefs.getString("clientId", defaults.clientId) ?: defaults.clientId,
        )
    }

    private fun saveConfig(config: GuestbookConfig) {
        prefs.edit()
            .putString("host", config.host)
            .putString("port", config.port)
            .putString("username", config.username)
            .putString("password", config.password)
            .putString("deviceId", config.deviceId)
            .putString("clientId", config.clientId)
            .apply()
    }
}
