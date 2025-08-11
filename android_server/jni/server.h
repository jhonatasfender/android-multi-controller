#ifndef ANDROID_SERVER_SERVER_H
#define ANDROID_SERVER_SERVER_H

#include "common/types.h"
#include "common/logger.h"
#include "screen_capture/capture_interface.h"
#include "video_encoder/encoder_interface.h"
#include "audio_capture/audio_capture_interface.h"
#include "network/tcp_server.h"
#include "protocol/protocol_handler.h"

#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>

namespace android_server {

struct ServerConfig {
    uint16_t port;
    VideoConfig videoConfig;
    AudioConfig audioConfig;
    bool enableAudio;
    bool enableInput;
    uint32_t maxConnections;
    uint32_t heartbeatInterval;
    
    ServerConfig() 
        : port(8080)
        , enableAudio(true)
        , enableInput(true)
        , maxConnections(4)
        , heartbeatInterval(5000)
    {}
};

struct ClientInfo {
    uint32_t connectionId;
    std::string address;
    uint16_t port;
    uint64_t connectTime;
    uint64_t lastHeartbeat;
    std::atomic<bool> active;
    
    ClientInfo(uint32_t id, const std::string& addr, uint16_t p)
        : connectionId(id)
        , address(addr)
        , port(p)
        , connectTime(0)
        , lastHeartbeat(0)
        , active(true)
    {}
    
    ClientInfo(uint32_t id, const std::string& addr, uint16_t p, uint64_t cTime, uint64_t hTime, bool act)
        : connectionId(id)
        , address(addr)
        , port(p)
        , connectTime(cTime)
        , lastHeartbeat(hTime)
        , active(act)
    {}
    
    ClientInfo(const ClientInfo& other) = delete;
    ClientInfo& operator=(const ClientInfo& other) = delete;
    
    ClientInfo(ClientInfo&& other) noexcept
        : connectionId(other.connectionId)
        , address(std::move(other.address))
        , port(other.port)
        , connectTime(other.connectTime)
        , lastHeartbeat(other.lastHeartbeat)
        , active(other.active.load())
    {}
    
    ClientInfo& operator=(ClientInfo&& other) noexcept {
        if (this != &other) {
            connectionId = other.connectionId;
            address = std::move(other.address);
            port = other.port;
            connectTime = other.connectTime;
            lastHeartbeat = other.lastHeartbeat;
            active = other.active.load();
        }
        return *this;
    }
};

class Server {
public:
    explicit Server(const ServerConfig& config);
    ~Server();
    
    Result initialize();
    Result start();
    Result stop();
    bool isRunning() const;
    
    ServerStatus getStatus() const;
    std::vector<ClientInfo> getConnectedClients() const;
    DeviceInfo getDeviceInfo() const;
    const ServerConfig& getConfig() const { return m_config; }
    void setVideoConfig(const VideoConfig& config);
    void setAudioConfig(const AudioConfig& config);
    
private:
    ServerConfig m_config;
    
    std::atomic<ServerStatus> m_status;
    std::atomic<bool> m_running;
    
    std::unique_ptr<network::TcpServer> m_tcpServer;
    std::unique_ptr<screen_capture::CaptureInterface> m_screenCapture;
    std::unique_ptr<video_encoder::EncoderInterface> m_videoEncoder;
    std::unique_ptr<audio_capture::AudioCaptureInterface> m_audioCapture;
    std::unique_ptr<protocol::ProtocolHandler> m_protocolHandler;
    
    std::unique_ptr<std::thread> m_captureThread;
    std::unique_ptr<std::thread> m_networkThread;
    std::unique_ptr<std::thread> m_heartbeatThread;
    
    mutable std::mutex m_clientsMutex;
    std::vector<std::shared_ptr<ClientInfo>> m_clients;
    std::atomic<uint32_t> m_nextConnectionId;
    
    DeviceInfo m_deviceInfo;
    Result initializeDeviceInfo();
    Result initializeCapture();
    Result initializeEncoder();
    Result initializeAudio();
    Result initializeNetwork();
    
    void captureThreadFunc();
    void networkThreadFunc();
    void heartbeatThreadFunc();
    
    void onClientConnected(uint32_t connectionId, const std::string& address, uint16_t port);
    void onClientDisconnected(uint32_t connectionId);
    void onClientData(uint32_t connectionId, const byte_vector& data);
    
    void handleControlEvent(uint32_t connectionId, const protocol::ControlEventPacket& packet);
    void sendMetadataToClient(uint32_t connectionId);
    void sendVideoConfigToClient(uint32_t connectionId);
    void sendAudioConfigToClient(uint32_t connectionId);
    
    void broadcastVideoFrame(const byte_vector& frameData, uint64_t pts, bool keyframe);
    void broadcastAudioFrame(const byte_vector& frameData, uint64_t pts);
    void broadcastHeartbeat();
    void broadcastVideoConfigToAllClients(const byte_vector& configData);
    
    void cleanup();
    void removeInactiveClients();
};

} // namespace android_server

#endif // ANDROID_SERVER_SERVER_H 