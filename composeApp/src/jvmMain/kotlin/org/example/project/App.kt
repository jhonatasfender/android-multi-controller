package org.example.project

import androidx.compose.runtime.Composable
import org.example.project.ui.screens.DevicesScreen
import org.example.project.ui.theme.MirrorDeskTheme
import org.jetbrains.compose.ui.tooling.preview.Preview

@Composable
@Preview
fun App() {
    MirrorDeskTheme {
        DevicesScreen()
    }
}
