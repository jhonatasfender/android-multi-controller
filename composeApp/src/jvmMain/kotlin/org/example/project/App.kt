package org.example.project

import androidx.compose.foundation.background
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import org.jetbrains.compose.ui.tooling.preview.Preview
import org.example.project.usecase.GetConnectedDevicesUseCase
import org.example.project.di.LocalAppDependencies
import org.example.project.core.stream.DeviceVideoStreamClient
import org.example.project.repositories.adb.listDevices.AdbDevice

@Composable
@Preview
fun App() {
    MaterialTheme(colorScheme = darkColorScheme()) {
        val deps = LocalAppDependencies.current
        val useCase = remember(deps) { GetConnectedDevicesUseCase(deps.adbListDevicesRepository) }
        var devices by remember { mutableStateOf<List<AdbDevice>>(emptyList()) }
        var loading by remember { mutableStateOf(true) }

        val clients = remember { androidx.compose.runtime.mutableStateMapOf<String, DeviceVideoStreamClient>() }

        LaunchedEffect(Unit) {
            try {
                devices = useCase()
            } finally {
                loading = false
            }
        }

        LaunchedEffect(devices) {
            val desired = devices.filter { it.state == "device" }.map { it.id }.toSet()
            val existing = clients.keys.toSet()
            for (id in desired - existing) {
                val c = DeviceVideoStreamClient(screenRecordRepo = deps.adbScreenRecordRepository, serial = id)
                clients[id] = c
                c.start()
            }
            for (id in existing - desired) {
                clients.remove(id)?.stop()
            }
        }

        DisposableEffect(Unit) {
            onDispose {
                clients.values.forEach { it.stop() }
                clients.clear()
            }
        }

        Box(
            modifier = Modifier
                .fillMaxSize()
                .background(MaterialTheme.colorScheme.background)
        ) {
            if (devices.isNotEmpty()) {
                LazyVerticalGrid(
                    columns = GridCells.Adaptive(minSize = 260.dp),
                    contentPadding = PaddingValues(16.dp),
                    modifier = Modifier.fillMaxSize()
                ) {
                    items(devices) { device ->
                        val client = clients[device.id]
                        val streamState = if (client != null) client.state.collectAsState().value else DeviceVideoStreamClient.State()
                         DeviceTile(
                            name = device.displayName,
                            state = device.state,
                            running = streamState.running,
                             bytes = streamState.bytesReceived,
                             error = streamState.error,
                              preview = streamState.lastFrame,
                            minTileHeight = 180.dp,
                        )
                    }
                }
            } else {
                val message = if (loading) {
                    "Procurando dispositivos ADB..."
                } else {
                    "Nenhum dispositivo detectado. Conecte via USB e habilite a Depuração USB (ADB)."
                }
                Box(modifier = Modifier.fillMaxSize()) {
                    Text(
                        text = message,
                        style = MaterialTheme.typography.bodyLarge,
                        modifier = Modifier.align(Alignment.Center)
                    )
                }
            }
        }
    }
}

data class Device(val id: String, val name: String)

private fun demoDevices(count: Int): List<Device> = List(count) { idx ->
    Device(id = "device-$idx", name = "Device ${idx + 1}")
}

@Composable
private fun DeviceTile(
    name: String,
    state: String,
    running: Boolean,
    bytes: Long,
    error: String?,
    preview: androidx.compose.ui.graphics.ImageBitmap?,
    minTileHeight: Dp,
) {
    Card(
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surface,
            contentColor = MaterialTheme.colorScheme.onSurface
        ),
        elevation = CardDefaults.cardElevation(defaultElevation = 2.dp),
        modifier = Modifier
            .padding(8.dp)
            .aspectRatio(9f / 16f)
            .background(MaterialTheme.colorScheme.surface)
    ) {
        Box(modifier = Modifier.fillMaxSize()) {
            Text(
                text = name,
                style = MaterialTheme.typography.titleMedium,
                modifier = Modifier.align(Alignment.TopStart).padding(12.dp)
            )
            Text(
                text = state,
                style = MaterialTheme.typography.labelMedium,
                modifier = Modifier.align(Alignment.TopEnd).padding(12.dp)
            )
            if (preview != null) {
                Image(
                    bitmap = preview,
                    contentDescription = null,
                    modifier = Modifier.align(Alignment.Center).fillMaxSize()
                )
            }
            val status = when {
                error != null -> "Erro: ${'$'}error"
                running -> "Streaming • ${'$'}{formatBytes(bytes)}"
                else -> "Aguardando"
            }
            Text(
                text = status,
                style = MaterialTheme.typography.bodySmall,
                modifier = Modifier.align(Alignment.BottomStart).padding(12.dp)
            )
        }
    }
}

private fun formatBytes(bytes: Long): String {
    if (bytes <= 0) return "0 B"
    val kb = bytes / 1024.0
    val mb = kb / 1024.0
    return if (mb >= 1.0) String.format("%.1f MB", mb) else String.format("%.0f KB", kb)
}