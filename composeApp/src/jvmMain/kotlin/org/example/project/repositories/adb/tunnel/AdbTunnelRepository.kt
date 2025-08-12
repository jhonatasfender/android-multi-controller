package org.example.project.repositories.adb.tunnel

interface AdbTunnelRepository {
    fun reverse(serial: String, deviceSocketName: String, localPort: Int): Boolean
    fun forward(serial: String, localPort: Int, deviceSocketName: String): Boolean
    fun removeReverse(serial: String, deviceSocketName: String): Boolean
    fun removeForward(serial: String, localPort: Int): Boolean
}


