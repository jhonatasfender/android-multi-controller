package org.example.project.ui.components

import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.ImageBitmap
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp

@Composable
fun DeviceTile(
    name: String,
    state: String,
    running: Boolean,
    bytes: Long,
    error: String?,
    preview: ImageBitmap?,
    minTileHeight: Dp,
) {
    Card(
        colors =
            CardDefaults.cardColors(
                containerColor = MaterialTheme.colorScheme.surface,
                contentColor = MaterialTheme.colorScheme.onSurface,
            ),
        elevation = CardDefaults.cardElevation(defaultElevation = 2.dp),
        modifier =
            Modifier
                .padding(8.dp)
                .aspectRatio(9f / 16f)
                .background(MaterialTheme.colorScheme.surface)
                .defaultMinSize(minHeight = minTileHeight),
    ) {
        Column(modifier = Modifier.fillMaxSize().padding(12.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically, modifier = Modifier.fillMaxWidth()) {
                Text(
                    text = name,
                    style = MaterialTheme.typography.titleMedium,
                    modifier = Modifier.weight(1f),
                )
                SuggestionChip(onClick = {}, label = { Text(state) })
            }

            Spacer(Modifier.height(8.dp))

            Box(
                modifier =
                    Modifier
                        .fillMaxSize(),
                contentAlignment = Alignment.Center,
            ) {
                if (preview != null) {
                    Image(
                        bitmap = preview,
                        contentDescription = null,
                        modifier = Modifier.fillMaxHeight().aspectRatio(9f / 16f),
                        contentScale = ContentScale.FillHeight,
                    )
                }
            }

            val status =
                when {
                    error != null -> "Erro: $error"
                    running -> "Streaming • ${formatBytes(bytes)}"
                    else -> "Aguardando"
                }
            Text(
                text = status,
                style = MaterialTheme.typography.bodySmall,
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
