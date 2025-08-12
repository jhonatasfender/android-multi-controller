package org.example.project

import androidx.compose.ui.window.Window
import androidx.compose.ui.window.WindowPlacement
import androidx.compose.ui.window.application
import androidx.compose.ui.window.rememberWindowState
import androidx.compose.runtime.remember
import org.example.project.di.AppDependencies
import org.example.project.di.ProvideAppDependencies
import org.example.project.repositories.adb.listDevices.JvmAdbListDevicesRepository
import org.example.project.repositories.adb.screenRecord.JvmAdbScreenRecordRepository

fun main() = application {
    val state = rememberWindowState(placement = WindowPlacement.Fullscreen)
    val deps = remember {
        AppDependencies(
            adbListDevicesRepository = JvmAdbListDevicesRepository(),
            adbScreenRecordRepository = JvmAdbScreenRecordRepository(),
        )
    }
    Window(
        onCloseRequest = ::exitApplication,
        title = "MirrorDesk",
        state = state,
        resizable = true,
    ) {
        ProvideAppDependencies(deps) {
            App()
        }
    }
}