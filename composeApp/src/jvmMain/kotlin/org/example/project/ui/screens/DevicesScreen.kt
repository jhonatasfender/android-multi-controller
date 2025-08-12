package org.example.project.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.lazy.grid.rememberLazyGridState
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import org.example.project.core.stream.DeviceVideoStreamClient
import org.example.project.di.LocalAppDependencies
import org.example.project.repositories.adb.listDevices.AdbDevice
import org.example.project.ui.components.DeviceTile
import org.example.project.usecase.GetConnectedDevicesUseCase

@Composable
fun DevicesScreen() {
    val deps = LocalAppDependencies.current
    val useCase = remember(deps) { GetConnectedDevicesUseCase(deps.adbListDevicesRepository) }
    var devices by remember { mutableStateOf<List<AdbDevice>>(emptyList()) }
    var loading by remember { mutableStateOf(true) }

    val clients = remember { mutableStateMapOf<String, DeviceVideoStreamClient>() }

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
        modifier =
            Modifier
                .fillMaxSize()
                .background(MaterialTheme.colorScheme.background),
    ) {
        if (devices.isNotEmpty()) {
            BoxWithConstraints(modifier = Modifier.fillMaxSize()) {
                val targetTileWidth = 360.dp
                val minColumns = 2
                val maxColumns = 6
                val columns =
                    remember(maxWidth) {
                        val raw = (maxWidth / targetTileWidth).toInt().coerceAtLeast(minColumns)
                        raw.coerceAtMost(maxColumns)
                    }

                val gridState = rememberLazyGridState()
                LazyVerticalGrid(
                    columns = GridCells.Fixed(columns),
                    contentPadding = PaddingValues(24.dp),
                    horizontalArrangement = Arrangement.spacedBy(24.dp),
                    verticalArrangement = Arrangement.spacedBy(24.dp),
                    state = gridState,
                    modifier = Modifier.fillMaxSize(),
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
            }
        } else {
            val message =
                if (loading) {
                    "Procurando dispositivos ADB..."
                } else {
                    "Nenhum dispositivo detectado. Conecte via USB e habilite a Depuração USB (ADB)."
                }
            Box(modifier = Modifier.fillMaxSize()) {
                Text(
                    text = message,
                    style = MaterialTheme.typography.bodyLarge,
                    modifier = Modifier.align(Alignment.Center),
                )
            }
        }
    }
}
