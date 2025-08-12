package com.mirrordesk.serverstub

import java.net.ServerSocket

fun main(args: Array<String>) {
    var port = 27183
    var i = 0
    while (i < args.size) {
        when (args[i]) {
            "--port" -> {
                val v = args.getOrNull(i + 1)?.toIntOrNull()
                if (v != null) port = v
                i += 1
            }
        }
        i += 1
    }

    val server = ServerSocket(port)
    println("serverStub listening on port $port")
    while (true) {
        val socket = server.accept()
        Thread {
            socket.getOutputStream().use { out ->
                val hello = "HELLO_MIRRORDESK\n".toByteArray()
                out.write(hello)
                out.flush()
                Thread.sleep(50)
            }
            socket.close()
        }.start()
    }
}


