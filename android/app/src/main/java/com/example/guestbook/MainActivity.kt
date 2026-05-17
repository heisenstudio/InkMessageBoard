package com.example.guestbook

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ExperimentalLayoutApi
import androidx.compose.foundation.layout.FlowRow
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.AssistChip
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Divider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Surface
import androidx.compose.material3.Tab
import androidx.compose.material3.TabRow
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.example.guestbook.ui.theme.GuestbookTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            GuestbookTheme {
                GuestbookApp()
            }
        }
    }
}

@Composable
private fun GuestbookApp() {
    val context = LocalContext.current
    val controller = remember { GuestbookController(context.applicationContext) }
    DisposableEffect(controller) {
        controller.connect()
        onDispose { controller.close() }
    }

    var selectedTab by remember { mutableIntStateOf(0) }
    val tabs = listOf("留言", "状态", "设置")

    Scaffold(
        modifier = Modifier.fillMaxSize(),
        topBar = {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .background(MaterialTheme.colorScheme.surface)
                    .padding(top = 36.dp),
            ) {
                Header(controller)
                TabRow(selectedTabIndex = selectedTab) {
                    tabs.forEachIndexed { index, title ->
                        Tab(
                            selected = selectedTab == index,
                            onClick = { selectedTab = index },
                            text = { Text(title, maxLines = 1) },
                        )
                    }
                }
            }
        },
    ) { innerPadding ->
        Box(
            modifier = Modifier
                .fillMaxSize()
                .background(MaterialTheme.colorScheme.background)
                .padding(innerPadding),
        ) {
            when (selectedTab) {
                0 -> MessagePage(controller)
                1 -> StatusPage(controller)
                else -> SettingsPage(controller)
            }
        }
    }
}

@Composable
private fun Header(controller: GuestbookController) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 18.dp, vertical = 14.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.SpaceBetween,
    ) {
        Column(modifier = Modifier.weight(1f)) {
            Text(
                text = "墨水屏留言板",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.SemiBold,
            )
            Text(
                text = controller.connectionDetail,
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                maxLines = 1,
                overflow = TextOverflow.Ellipsis,
            )
        }
        Spacer(Modifier.width(12.dp))
        StatusPill(
            text = controller.connectionText,
            active = controller.isConnected,
        )
    }
}

@Composable
@OptIn(ExperimentalLayoutApi::class)
private fun MessagePage(controller: GuestbookController) {
    val quickTexts = listOf("测试留言", "请查看设备状态", "稍后处理", "需要确认", "已完成")

    LazyColumn(
        modifier = Modifier.fillMaxSize(),
        contentPadding = androidx.compose.foundation.layout.PaddingValues(16.dp),
        verticalArrangement = Arrangement.spacedBy(14.dp),
    ) {
        item { DeviceSummary(controller.deviceStatus) }
        item {
            Column(
                modifier = Modifier.fillMaxWidth(),
                verticalArrangement = Arrangement.spacedBy(10.dp),
            ) {
                Text(
                    text = "新留言",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.SemiBold,
                )
                OutlinedTextField(
                    value = controller.draftText,
                    onValueChange = { controller.draftText = it },
                    modifier = Modifier
                        .fillMaxWidth()
                        .heightIn(min = 128.dp),
                    label = { Text("写给墨水屏的话") },
                    placeholder = { Text("例如：请查看设备状态。") },
                    minLines = 4,
                    maxLines = 8,
                )
                FlowRow(
                    horizontalArrangement = Arrangement.spacedBy(8.dp),
                    verticalArrangement = Arrangement.spacedBy(8.dp),
                ) {
                    quickTexts.forEach { text ->
                        AssistChip(
                            onClick = { controller.useQuickText(text) },
                            label = { Text(text) },
                        )
                    }
                }
                Row(horizontalArrangement = Arrangement.spacedBy(10.dp)) {
                    Button(
                        onClick = controller::sendDraft,
                        enabled = controller.isConnected,
                    ) {
                        Text("发送留言")
                    }
                    OutlinedButton(
                        onClick = { controller.draftText = "" },
                    ) {
                        Text("清空输入")
                    }
                }
            }
        }
        item {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(10.dp),
            ) {
                OutlinedButton(
                    onClick = { controller.sendCommand("refresh") },
                    enabled = controller.isConnected,
                    modifier = Modifier.weight(1f),
                ) {
                    Text("刷新屏幕")
                }
                OutlinedButton(
                    onClick = { controller.sendCommand("clear") },
                    enabled = controller.isConnected,
                    modifier = Modifier.weight(1f),
                ) {
                    Text("清除留言")
                }
            }
        }
        item {
            Text(
                text = "最近留言",
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.SemiBold,
            )
        }
        if (controller.messages.isEmpty()) {
            item { EmptyText("还没有发送记录。") }
        } else {
            items(controller.messages, key = { it.id }) { record ->
                MessageCard(record)
            }
        }
    }
}

@Composable
private fun DeviceSummary(status: DeviceStatus) {
    Surface(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(8.dp),
        color = MaterialTheme.colorScheme.surface,
        tonalElevation = 1.dp,
    ) {
        Column(
            modifier = Modifier.padding(14.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp),
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.SpaceBetween,
            ) {
                Column {
                    Text(
                        text = "设备状态",
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.SemiBold,
                    )
                    Text(
                        text = status.updatedAt?.let { "最后更新 $it" } ?: "等待设备上报",
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                    )
                }
                StatusPill(
                    text = when (status.online) {
                        true -> "在线"
                        false -> "离线"
                        null -> "未知"
                    },
                    active = status.online == true,
                )
            }
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
            ) {
                Metric("电量", status.battery?.let { "$it%" } ?: "--")
                Metric("RSSI", status.rssi?.toString() ?: "--")
                Metric("固件", status.firmware ?: "--")
            }
        }
    }
}

@Composable
private fun Metric(label: String, value: String) {
    Column(horizontalAlignment = Alignment.Start) {
        Text(
            text = label,
            style = MaterialTheme.typography.labelMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
        )
        Text(
            text = value,
            style = MaterialTheme.typography.titleMedium,
            fontWeight = FontWeight.SemiBold,
            maxLines = 1,
            overflow = TextOverflow.Ellipsis,
        )
    }
}

@Composable
private fun MessageCard(record: MessageRecord) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(8.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surface),
    ) {
        Column(
            modifier = Modifier.padding(14.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.SpaceBetween,
            ) {
                Text(
                    text = record.createdAt,
                    style = MaterialTheme.typography.labelMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                )
                StatusPill(text = stateLabel(record.state), active = record.state != "sent")
            }
            Text(
                text = record.text,
                style = MaterialTheme.typography.bodyLarge,
            )
            record.replyText?.let {
                Text(
                    text = "设备回复：$it",
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.primary,
                )
            }
            Text(
                text = record.id,
                style = MaterialTheme.typography.labelSmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                maxLines = 1,
                overflow = TextOverflow.Ellipsis,
            )
        }
    }
}

@Composable
private fun StatusPage(controller: GuestbookController) {
    LazyColumn(
        modifier = Modifier.fillMaxSize(),
        contentPadding = androidx.compose.foundation.layout.PaddingValues(16.dp),
        verticalArrangement = Arrangement.spacedBy(14.dp),
    ) {
        item {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                Text(
                    text = "Topic",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.SemiBold,
                )
                TopicLine("留言", controller.config.msgTopic)
                TopicLine("命令", controller.config.cmdTopic)
                TopicLine("状态", controller.config.statusTopic)
            }
        }
        item {
            Row(horizontalArrangement = Arrangement.spacedBy(10.dp)) {
                Button(onClick = controller::connect) {
                    Text("重新连接")
                }
                OutlinedButton(onClick = controller::disconnect) {
                    Text("断开")
                }
            }
        }
        item {
            Text(
                text = "事件日志",
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.SemiBold,
            )
        }
        if (controller.events.isEmpty()) {
            item { EmptyText("暂无 MQTT 事件。") }
        } else {
            items(controller.events) { event ->
                EventRow(event)
            }
        }
    }
}

@Composable
private fun TopicLine(label: String, topic: String) {
    Surface(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(8.dp),
        color = MaterialTheme.colorScheme.surface,
    ) {
        Column(modifier = Modifier.padding(12.dp)) {
            Text(
                text = label,
                style = MaterialTheme.typography.labelMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
            )
            Text(
                text = topic,
                style = MaterialTheme.typography.bodyMedium,
            )
        }
    }
}

@Composable
private fun EventRow(event: EventRecord) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 8.dp),
        verticalAlignment = Alignment.Top,
    ) {
        Text(
            text = event.time,
            style = MaterialTheme.typography.labelMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            modifier = Modifier.width(62.dp),
        )
        Column(modifier = Modifier.weight(1f)) {
            Text(
                text = event.title,
                style = MaterialTheme.typography.bodyMedium,
                fontWeight = FontWeight.SemiBold,
            )
            Text(
                text = event.detail,
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
            )
        }
    }
    Divider()
}

@Composable
private fun SettingsPage(controller: GuestbookController) {
    var editing by remember(controller.config) { mutableStateOf(controller.config) }
    Column(
        modifier = Modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp),
    ) {
        Text(
            text = "连接设置",
            style = MaterialTheme.typography.titleMedium,
            fontWeight = FontWeight.SemiBold,
        )
        OutlinedTextField(
            value = editing.deviceId,
            onValueChange = { editing = editing.copy(deviceId = it) },
            modifier = Modifier.fillMaxWidth(),
            label = { Text("设备 ID") },
            singleLine = true,
        )
        OutlinedTextField(
            value = editing.host,
            onValueChange = { editing = editing.copy(host = it) },
            modifier = Modifier.fillMaxWidth(),
            label = { Text("MQTT Host") },
            singleLine = true,
        )
        OutlinedTextField(
            value = editing.port,
            onValueChange = { editing = editing.copy(port = it) },
            modifier = Modifier.fillMaxWidth(),
            label = { Text("端口") },
            keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number),
            singleLine = true,
        )
        OutlinedTextField(
            value = editing.username,
            onValueChange = { editing = editing.copy(username = it) },
            modifier = Modifier.fillMaxWidth(),
            label = { Text("用户名") },
            singleLine = true,
        )
        OutlinedTextField(
            value = editing.password,
            onValueChange = { editing = editing.copy(password = it) },
            modifier = Modifier.fillMaxWidth(),
            label = { Text("密码") },
            visualTransformation = PasswordVisualTransformation(),
            singleLine = true,
        )
        OutlinedTextField(
            value = editing.clientId,
            onValueChange = { editing = editing.copy(clientId = it) },
            modifier = Modifier.fillMaxWidth(),
            label = { Text("手机端 Client ID") },
            singleLine = true,
        )
        Row(horizontalArrangement = Arrangement.spacedBy(10.dp)) {
            Button(
                onClick = {
                    controller.updateConfig(editing)
                    controller.connect()
                },
            ) {
                Text("保存并重连")
            }
            TextButton(
                onClick = { editing = GuestbookConfig(clientId = editing.clientId) },
            ) {
                Text("恢复默认")
            }
        }
    }
}

@Composable
private fun StatusPill(text: String, active: Boolean) {
    val color = if (active) {
        MaterialTheme.colorScheme.primary
    } else {
        MaterialTheme.colorScheme.outline
    }
    Surface(
        shape = RoundedCornerShape(50),
        color = color.copy(alpha = 0.12f),
        contentColor = color,
    ) {
        Row(
            modifier = Modifier.padding(horizontal = 10.dp, vertical = 6.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(6.dp),
        ) {
            Box(
                modifier = Modifier
                    .size(7.dp)
                    .clip(CircleShape)
                    .background(color),
            )
            Text(text = text, style = MaterialTheme.typography.labelMedium)
        }
    }
}

@Composable
private fun EmptyText(text: String) {
    Text(
        text = text,
        modifier = Modifier.padding(vertical = 18.dp),
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant,
    )
}

private fun stateLabel(state: String): String {
    return when (state) {
        "sent" -> "已发送"
        "accepted" -> "已接收"
        "displayed" -> "已显示"
        "seen" -> "已收到"
        "confirm" -> "待确认"
        "later" -> "稍后处理"
        "done" -> "已处理"
        else -> state
    }
}
