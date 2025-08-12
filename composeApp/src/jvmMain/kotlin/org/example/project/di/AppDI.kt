package org.example.project.di

import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.staticCompositionLocalOf
import org.example.project.repositories.adb.listDevices.AdbListDevicesRepository
import org.example.project.repositories.adb.screenRecord.AdbScreenRecordRepository

data class AppDependencies(
    val adbListDevicesRepository: AdbListDevicesRepository,
    val adbScreenRecordRepository: AdbScreenRecordRepository,
)

val LocalAppDependencies = staticCompositionLocalOf<AppDependencies> {
    error("AppDependencies not provided")
}

@Composable
fun ProvideAppDependencies(
    deps: AppDependencies,
    content: @Composable () -> Unit,
) {
    CompositionLocalProvider(LocalAppDependencies provides deps) {
        content()
    }
}
