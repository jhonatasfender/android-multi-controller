#include "tcp_server.h"
#include "../common/logger.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <algorithm>

namespace android_server::network
{
    TcpServer::TcpServer()
        : m_port(0)
          , m_maxConnections(10)
          , m_bindAddress("0.0.0.0")
          , m_running(false)
          , m_initialized(false)
          , m_serverSocket(-1)
          , m_nextConnectionId(1)
          , m_receiveBufferSize(65536)
          , m_sendBufferSize(65536)
          , m_receiveTimeout(5000)
          , m_sendTimeout(5000)
          , m_keepAliveEnabled(true)
          , m_keepAliveInterval(60)
          , m_totalConnections(0)
          , m_totalBytesReceived(0)
          , m_totalBytesSent(0)
    {
    }

    TcpServer::~TcpServer()
    {
        stop();
        closeServerSocket();
    }

    Result TcpServer::initialize(uint16_t port, uint32_t maxConnections)
    {
        if (m_initialized)
        {
            return Result::ERROR_ALREADY_RUNNING;
        }

        m_port = port;
        m_maxConnections = maxConnections;

        Result result = setupServerSocket();
        if (result != Result::SUCCESS)
        {
            return result;
        }

        m_initialized = true;

        return Result::SUCCESS;
    }

    Result TcpServer::start()
    {
        if (!m_initialized)
        {
            return Result::ERROR_NOT_INITIALIZED;
        }

        if (m_running)
        {
            return Result::ERROR_ALREADY_RUNNING;
        }

        if (listen(m_serverSocket, static_cast<int>(m_maxConnections)) < 0)
        {
            return Result::ERROR_NETWORK_FAILED;
        }

        m_running = true;

        m_acceptThread = std::make_unique<std::thread>(&TcpServer::acceptThreadFunc, this);
        m_receiveThread = std::make_unique<std::thread>(&TcpServer::receiveThreadFunc, this);

        return Result::SUCCESS;
    }

    Result TcpServer::stop()
    {
        if (!m_running)
        {
            return Result::SUCCESS;
        }

        m_running = false;

        if (m_acceptThread && m_acceptThread->joinable())
        {
            m_acceptThread->join();
            m_acceptThread.reset();
        }

        if (m_receiveThread && m_receiveThread->joinable())
        {
            m_receiveThread->join();
            m_receiveThread.reset();
        }

        {
            std::lock_guard<std::mutex> lock(m_connectionsMutex);
            for (auto& connection : m_connections)
            {
                connection->active = false;
                if (connection->socket >= 0)
                {
                    close(connection->socket);
                    connection->socket = -1;
                }
            }
            m_connections.clear();
        }

        return Result::SUCCESS;
    }

    bool TcpServer::isRunning() const
    {
        return m_running.load();
    }

    void TcpServer::setConnectionCallback(ConnectionCallback callback)
    {
        m_connectionCallback = callback;
    }

    void TcpServer::setDisconnectionCallback(DisconnectionCallback callback)
    {
        m_disconnectionCallback = callback;
    }

    void TcpServer::setDataCallback(DataCallback callback)
    {
        m_dataCallback = callback;
    }

    Result TcpServer::sendData(uint32_t connectionId, const byte_vector& data)
    {
        auto connection = getConnection(connectionId);
        if (!connection || !connection->active)
        {
            return Result::ERROR_NOT_RUNNING;
        }

        ssize_t sent = send(connection->socket, data.data(), data.size(), MSG_NOSIGNAL);
        if (sent < 0)
        {
            removeConnection(connection);
            return Result::ERROR_NETWORK_FAILED;
        }

        if (static_cast<size_t>(sent) != data.size())
        {
            // LOG_W("Partial send to connection %d: %zd/%zu bytes", connectionId, sent, data.size());
        }

        connection->bytesSent += static_cast<uint64_t>(sent);
        m_totalBytesSent += static_cast<uint64_t>(sent);

        return Result::SUCCESS;
    }

    Result TcpServer::sendDataToAll(const byte_vector& data)
    {
        return sendDataToAll(data, 0);
    }

    Result TcpServer::sendDataToAll(const byte_vector& data, uint32_t excludeConnectionId)
    {
        std::lock_guard<std::mutex> lock(m_connectionsMutex);

        for (auto& connection : m_connections)
        {
            if (connection->active && connection->connectionId != excludeConnectionId)
            {
                sendData(connection->connectionId, data);
            }
        }

        return Result::SUCCESS;
    }

    Result TcpServer::disconnectClient(uint32_t connectionId)
    {
        auto connection = getConnection(connectionId);
        if (!connection)
        {
            return Result::ERROR_NOT_RUNNING;
        }

        removeConnection(connection);
        return Result::SUCCESS;
    }

    std::vector<ConnectionInfo> TcpServer::getConnections() const
    {
        std::lock_guard<std::mutex> lock(m_connectionsMutex);

        std::vector<ConnectionInfo> connections;
        for (const auto& conn : m_connections)
        {
            if (conn->active)
            {
                connections.emplace_back(
                    conn->connectionId,
                    conn->address,
                    conn->port,
                    conn->socket
                );
            }
        }

        return connections;
    }

    size_t TcpServer::getConnectionCount() const
    {
        std::lock_guard<std::mutex> lock(m_connectionsMutex);
        return std::count_if(m_connections.begin(), m_connections.end(),
                             [](const auto& conn) { return conn->active.load(); });
    }

    bool TcpServer::isClientConnected(uint32_t connectionId) const
    {
        auto connection = getConnection(connectionId);
        return connection && connection->active;
    }

    uint16_t TcpServer::getPort() const
    {
        return m_port;
    }

    uint32_t TcpServer::getMaxConnections() const
    {
        return m_maxConnections;
    }

    std::string TcpServer::getBindAddress() const
    {
        return m_bindAddress;
    }

    uint64_t TcpServer::getTotalConnections() const
    {
        return m_totalConnections.load();
    }

    uint64_t TcpServer::getTotalBytesReceived() const
    {
        return m_totalBytesReceived.load();
    }

    uint64_t TcpServer::getTotalBytesSent() const
    {
        return m_totalBytesSent.load();
    }

    Result TcpServer::setupServerSocket()
    {
        m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_serverSocket < 0)
        {
            return Result::ERROR_NETWORK_FAILED;
        }

        int opt = 1;
        if (setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        {
            // LOG_W("Failed to set SO_REUSEADDR: %s", strerror(errno));
        }

        setSocketBufferSize(m_serverSocket, m_receiveBufferSize, m_sendBufferSize);
        setSocketTimeout(m_serverSocket, m_receiveTimeout, m_sendTimeout);

        if (m_keepAliveEnabled)
        {
            setSocketKeepAlive(m_serverSocket, true, m_keepAliveInterval);
        }

        struct sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(m_port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(m_serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0)
        {
            close(m_serverSocket);
            m_serverSocket = -1;
            return Result::ERROR_NETWORK_FAILED;
        }

        return Result::SUCCESS;
    }

    void TcpServer::closeServerSocket()
    {
        if (m_serverSocket >= 0)
        {
            close(m_serverSocket);
            m_serverSocket = -1;
        }
        m_initialized = false;
    }

    void TcpServer::acceptThreadFunc()
    {
        while (m_running)
        {
            struct sockaddr_in clientAddr{};
            socklen_t clientAddrLen = sizeof(clientAddr);

            int clientSocket = accept(m_serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr),
                                      &clientAddrLen);

            if (clientSocket < 0)
            {
                if (m_running && errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    // LOG_E("Accept failed: %s", strerror(errno));
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            std::string clientAddress = inet_ntoa(clientAddr.sin_addr);
            uint16_t clientPort = ntohs(clientAddr.sin_port);

            handleNewConnection(clientSocket, clientAddress, clientPort);
        }
    }

    void TcpServer::receiveThreadFunc()
    {
        while (m_running)
        {
            std::vector<std::shared_ptr<ConnectionInfo>> activeConnections;

            {
                std::lock_guard<std::mutex> lock(m_connectionsMutex);
                for (auto& conn : m_connections)
                {
                    if (conn->active)
                    {
                        activeConnections.push_back(conn);
                    }
                }
            }

            for (auto& connection : activeConnections)
            {
                if (!connection->active) continue;

                handleClientData(connection);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void TcpServer::handleNewConnection(int clientSocket, const std::string& address, uint16_t port)
    {
        {
            std::lock_guard<std::mutex> lock(m_connectionsMutex);
            if (m_connections.size() >= m_maxConnections)
            {
                close(clientSocket);
                return;
            }
        }

        setSocketNonBlocking(clientSocket);
        setSocketBufferSize(clientSocket, m_receiveBufferSize, m_sendBufferSize);
        setSocketTimeout(clientSocket, m_receiveTimeout, m_sendTimeout);

        if (m_keepAliveEnabled)
        {
            setSocketKeepAlive(clientSocket, true, m_keepAliveInterval);
        }

        uint32_t connectionId = m_nextConnectionId++;
        auto connection = std::make_shared<ConnectionInfo>(connectionId, address, port, clientSocket);
        connection->connectTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

        {
            std::lock_guard<std::mutex> lock(m_connectionsMutex);
            m_connections.push_back(connection);
        }

        m_totalConnections++;

        if (m_connectionCallback)
        {
            m_connectionCallback(connectionId, address, port);
        }
    }

    void TcpServer::handleClientData(std::shared_ptr<ConnectionInfo> connection)
    {
        if (!connection || !connection->active) return;

        byte_vector buffer(4096);
        ssize_t received = recv(connection->socket, buffer.data(), buffer.size(), MSG_DONTWAIT);

        if (received > 0)
        {
            buffer.resize(static_cast<size_t>(received));
            connection->bytesReceived += static_cast<uint64_t>(received);
            m_totalBytesReceived += static_cast<uint64_t>(received);

            if (m_dataCallback)
            {
                m_dataCallback(connection->connectionId, buffer);
            }
        }
        else if (received == 0)
        {
            removeConnection(connection);
        }
        else if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            removeConnection(connection);
        }
    }

    void TcpServer::removeConnection(uint32_t connectionId)
    {
        auto connection = getConnection(connectionId);
        if (connection)
        {
            removeConnection(connection);
        }
    }

    void TcpServer::removeConnection(std::shared_ptr<ConnectionInfo> connection)
    {
        if (!connection) return;

        connection->active = false;

        if (connection->socket >= 0)
        {
            close(connection->socket);
            connection->socket = -1;
        }

        if (m_disconnectionCallback)
        {
            m_disconnectionCallback(connection->connectionId);
        }

        {
            std::lock_guard<std::mutex> lock(m_connectionsMutex);
            m_connections.erase(
                std::remove_if(m_connections.begin(), m_connections.end(),
                               [connection](const auto& conn) { return conn == connection; }),
                m_connections.end()
            );
        }
    }

    std::shared_ptr<ConnectionInfo> TcpServer::getConnection(uint32_t connectionId) const
    {
        std::lock_guard<std::mutex> lock(m_connectionsMutex);

        auto it = std::find_if(m_connections.begin(), m_connections.end(),
                               [connectionId](const auto& conn)
                               {
                                   return conn->connectionId == connectionId && conn->active;
                               });

        return (it != m_connections.end()) ? *it : nullptr;
    }

    Result TcpServer::setSocketNonBlocking(int socket)
    {
        int flags = fcntl(socket, F_GETFL, 0);
        if (flags < 0) return Result::ERROR_NETWORK_FAILED;

        if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) < 0)
        {
            return Result::ERROR_NETWORK_FAILED;
        }

        return Result::SUCCESS;
    }

    Result TcpServer::setSocketKeepAlive(int socket, bool enable, uint32_t intervalSec)
    {
        int opt = enable ? 1 : 0;
        if (setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0)
        {
            return Result::ERROR_NETWORK_FAILED;
        }

        // Note: TCP_KEEPINTVL and TCP_KEEPIDLE are not available on Android NDK
        // The SO_KEEPALIVE option provides basic keep-alive functionality
        (void)intervalSec; // Suppress unused parameter warning

        return Result::SUCCESS;
    }

    Result TcpServer::setSocketBufferSize(int socket, size_t receiveSize, size_t sendSize)
    {
        int recvSize = static_cast<int>(receiveSize);
        int sendSizeInt = static_cast<int>(sendSize);

        setsockopt(socket, SOL_SOCKET, SO_RCVBUF, &recvSize, sizeof(recvSize));
        setsockopt(socket, SOL_SOCKET, SO_SNDBUF, &sendSizeInt, sizeof(sendSizeInt));

        return Result::SUCCESS;
    }

    Result TcpServer::setSocketTimeout(int socket, uint32_t receiveTimeoutMs, uint32_t sendTimeoutMs)
    {
        struct timeval recvTimeout{};
        recvTimeout.tv_sec = receiveTimeoutMs / 1000;
        recvTimeout.tv_usec = (receiveTimeoutMs % 1000) * 1000;

        struct timeval sendTimeout{};
        sendTimeout.tv_sec = sendTimeoutMs / 1000;
        sendTimeout.tv_usec = (sendTimeoutMs % 1000) * 1000;

        setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &recvTimeout, sizeof(recvTimeout));
        setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &sendTimeout, sizeof(sendTimeout));

        return Result::SUCCESS;
    }
}
