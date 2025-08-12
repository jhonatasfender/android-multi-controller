package org.example.project.repositories.adb.listDevices

data class AdbDevice(
    val id: String,
    val state: String,
    val model: String? = null,
    val device: String? = null,
    val product: String? = null,
) {
    val displayName: String = listOfNotNull(model, device, product, id).first()
}