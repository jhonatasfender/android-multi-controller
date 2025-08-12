package org.example.project

import androidx.compose.runtime.Composable
import org.jetbrains.compose.ui.tooling.preview.Preview
import org.example.project.ui.theme.MirrorDeskTheme
import org.example.project.ui.screens.DevicesScreen

@Composable
@Preview
fun App() {
    MirrorDeskTheme {
        DevicesScreen()
    }
}