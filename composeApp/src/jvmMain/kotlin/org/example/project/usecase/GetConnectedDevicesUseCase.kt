package org.example.project.usecase

import org.example.project.repositories.adb.listDevices.AdbDevice
import org.example.project.repositories.adb.listDevices.AdbListDevicesRepository

class GetConnectedDevicesUseCase(
    private val adb: AdbListDevicesRepository,
) {
    suspend operator fun invoke(): List<AdbDevice> = adb.listDevices()
}
