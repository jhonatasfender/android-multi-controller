#ifndef ANDROID_SERVER_NETWORK_TCP_SERVER_H
#define ANDROID_SERVER_NETWORK_TCP_SERVER_H

#include "../common/types.h"
#include <functional>
#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>

namespace android_server::network
{
    using ConnectionCallback = std::function<void(
        uint32_t connectionId,
        const std::string& address,
        uint16_t port
    )>;

    using DisconnectionCallback = std::function<void(uint32_t connectionId)>;

    using DataCallback = std::function<void(uint32_t connectionId, const byte_vector& data)>;

    struct ConnectionInfo
    {
        uint32_t connectionId;
        std::string address;
        uint16_t port;
        int socket;
        uint64_t connectTime;
        uint64_t bytesReceived;
        uint64_t bytesSent;
        std::atomic<bool> active;

        ConnectionInfo(const uint32_t id, const std::string& addr, const uint16_t p, const int sock)
            : connectionId(id), address(addr), port(p), socket(sock), connectTime(0), bytesReceived(0), bytesSent(0),
              active(true)
        {
        }

        ConnectionInfo(const ConnectionInfo& other) = delete;
        ConnectionInfo& operator=(const ConnectionInfo& other) = delete;

        ConnectionInfo(ConnectionInfo&& other) noexcept
            : connectionId(other.connectionId)
              , address(std::move(other.address))
              , port(other.port)
              , socket(other.socket)
              , connectTime(other.connectTime)
              , bytesReceived(other.bytesReceived)
              , bytesSent(other.bytesSent)
              , active(other.active.load())
        {
        }

        ConnectionInfo& operator=(ConnectionInfo&& other) noexcept
        {
            if (this != &other)
            {
                connectionId = other.connectionId;
                address = std::move(other.address);
                port = other.port;
                socket = other.socket;
                connectTime = other.connectTime;
                bytesReceived = other.bytesReceived;
                bytesSent = other.bytesSent;
                active = other.active.load();
            }
            return *this;
        }
    };

    class TcpServer
    {
    public:
        TcpServer();
        ~TcpServer();

        Result initialize(uint16_t port, uint32_t maxConnections = 10);
        Result start();
        Result stop();
        bool isRunning() const;

        void setConnectionCallback(ConnectionCallback callback);
        void setDisconnectionCallback(DisconnectionCallback callback);
        void setDataCallback(DataCallback callback);

        Result sendData(uint32_t connectionId, const byte_vector& data);
        Result sendDataToAll(const byte_vector& data);
        Result sendDataToAll(const byte_vector& data, uint32_t excludeConnectionId);

        Result disconnectClient(uint32_t connectionId);
        std::vector<ConnectionInfo> getConnections() const;
        size_t getConnectionCount() const;
        bool isClientConnected(uint32_t connectionId) const;

        void setReceiveBufferSize(size_t size);
        void setSendBufferSize(size_t size);
        void setReceiveTimeout(uint32_t timeoutMs);
        void setSendTimeout(uint32_t timeoutMs);
        void setKeepAlive(bool enable, uint32_t intervalSec = 60);

        uint16_t getPort() const;
        uint32_t getMaxConnections() const;
        std::string getBindAddress() const;

        uint64_t getTotalConnections() const;
        uint64_t getTotalBytesReceived() const;
        uint64_t getTotalBytesSent() const;

    private:
        uint16_t m_port;
        uint32_t m_maxConnections;
        std::string m_bindAddress;

        std::atomic<bool> m_running;
        std::atomic<bool> m_initialized;

        int m_serverSocket;

        std::unique_ptr<std::thread> m_acceptThread;
        std::unique_ptr<std::thread> m_receiveThread;

        mutable std::mutex m_connectionsMutex;
        std::vector<std::shared_ptr<ConnectionInfo>> m_connections;
        std::atomic<uint32_t> m_nextConnectionId;

        ConnectionCallback m_connectionCallback;
        DisconnectionCallback m_disconnectionCallback;
        DataCallback m_dataCallback;

        size_t m_receiveBufferSize;
        size_t m_sendBufferSize;
        uint32_t m_receiveTimeout;
        uint32_t m_sendTimeout;
        bool m_keepAliveEnabled;
        uint32_t m_keepAliveInterval;

        std::atomic<uint64_t> m_totalConnections;
        std::atomic<uint64_t> m_totalBytesReceived;
        std::atomic<uint64_t> m_totalBytesSent;
        
        void acceptThreadFunc();
        void receiveThreadFunc();

        Result setupServerSocket();
        void closeServerSocket();

        void handleNewConnection(int clientSocket,
                                 const std::string& address,
                                 uint16_t port);

        void handleClientData(std::shared_ptr<ConnectionInfo> connection);
        void removeConnection(uint32_t connectionId);
        void removeConnection(std::shared_ptr<ConnectionInfo> connection);

        std::shared_ptr<ConnectionInfo> getConnection(uint32_t connectionId) const;

        static std::pair<std::string, uint16_t> getSocketAddress(int socket);
        static Result setSocketNonBlocking(int socket);
        static Result setSocketKeepAlive(int socket, bool enable, uint32_t intervalSec);
        static Result setSocketBufferSize(int socket, size_t receiveSize, size_t sendSize);
        static Result setSocketTimeout(int socket, uint32_t receiveTimeoutMs, uint32_t sendTimeoutMs);
    };
}

#endif // ANDROID_SERVER_NETWORK_TCP_SERVER_H
