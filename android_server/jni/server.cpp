#include "server.h"
#include "screen_capture/media_projection_capture.h"
#include "video_encoder/mediacodec_h264_encoder.h"
#include "network/tcp_server.h"

#include <chrono>

namespace android_server {

Server::Server(const ServerConfig& config)
    : m_config(config)
    , m_status(ServerStatus::STOPPED)
    , m_running(false)
    , m_nextConnectionId(1) {
}

Server::~Server() {
    stop();
    cleanup();
}

Result Server::initialize() {
    if (m_status != ServerStatus::STOPPED) {
        return Result::ERROR_ALREADY_RUNNING;
    }
    
    m_status = ServerStatus::STARTING;
    
    Result result = initializeDeviceInfo();
    if (result != Result::SUCCESS) {
        m_status = ServerStatus::ERROR;
        return result;
    }
    
    result = initializeCapture();
    if (result != Result::SUCCESS) {
        m_status = ServerStatus::ERROR;
        return result;
    }
    
    result = initializeEncoder();
    if (result != Result::SUCCESS) {
        m_status = ServerStatus::ERROR;
        return result;
    }
    
    result = initializeNetwork();
    if (result != Result::SUCCESS) {
        m_status = ServerStatus::ERROR;
        return result;
    }
    
    return Result::SUCCESS;
}

Result Server::start() {
    if (m_status != ServerStatus::STARTING) {
        return Result::ERROR_NOT_INITIALIZED;
    }
    
    if (m_running) {
        return Result::ERROR_ALREADY_RUNNING;
    }
    
    Result result = m_tcpServer->start();
    if (result != Result::SUCCESS) {
        m_status = ServerStatus::ERROR;
        return result;
    }
    
    result = m_videoEncoder->start();
    if (result != Result::SUCCESS) {
        m_status = ServerStatus::ERROR;
        return result;
    }
    
    result = m_screenCapture->start();
    if (result != Result::SUCCESS) {
        m_status = ServerStatus::ERROR;
        return result;
    }
    
    m_running = true;
    m_status = ServerStatus::RUNNING;
    
    m_captureThread = std::make_unique<std::thread>(&Server::captureThreadFunc, this);
    m_networkThread = std::make_unique<std::thread>(&Server::networkThreadFunc, this);
    m_heartbeatThread = std::make_unique<std::thread>(&Server::heartbeatThreadFunc, this);
    
    return Result::SUCCESS;
}

Result Server::stop() {
    if (!m_running) {
        return Result::SUCCESS;
    }
    
    m_status = ServerStatus::STOPPING;
    m_running = false;
    
    if (m_captureThread && m_captureThread->joinable()) {
        m_captureThread->join();
        m_captureThread.reset();
    }
    
    if (m_networkThread && m_networkThread->joinable()) {
        m_networkThread->join();
        m_networkThread.reset();
    }
    
    if (m_heartbeatThread && m_heartbeatThread->joinable()) {
        m_heartbeatThread->join();
        m_heartbeatThread.reset();
    }
    
    if (m_screenCapture) {
        m_screenCapture->stop();
    }
    
    if (m_videoEncoder) {
        m_videoEncoder->stop();
    }
    
    if (m_tcpServer) {
        m_tcpServer->stop();
    }
    
    m_status = ServerStatus::STOPPED;
    
    return Result::SUCCESS;
}

bool Server::isRunning() const {
    return m_running.load();
}

ServerStatus Server::getStatus() const {
    return m_status.load();
}

std::vector<ClientInfo> Server::getConnectedClients() const {
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    
    std::vector<ClientInfo> clients;
    for (const auto& client : m_clients) {
        if (client->active) {
            clients.emplace_back(client->connectionId, client->address, client->port, 
                               client->connectTime, client->lastHeartbeat, client->active.load());
        }
    }
    
    return clients;
}

DeviceInfo Server::getDeviceInfo() const {
    return m_deviceInfo;
}

void Server::setVideoConfig(const VideoConfig& config) {
    m_config.videoConfig = config;
    if (m_videoEncoder) {
        m_videoEncoder->setBitrate(config.bitrate);
        m_videoEncoder->setFrameRate(config.fps);
    }
}

void Server::setAudioConfig(const AudioConfig& config) {
    m_config.audioConfig = config;
}

Result Server::initializeDeviceInfo() {
    m_deviceInfo.model = "Android Device";
    m_deviceInfo.manufacturer = "Unknown";
    m_deviceInfo.brand = "Android";
    m_deviceInfo.device = "generic";
    m_deviceInfo.product = "aosp";
    m_deviceInfo.androidVersion = "Unknown";
    m_deviceInfo.apiLevel = 21;
    m_deviceInfo.screenResolution = m_config.videoConfig.resolution;
    m_deviceInfo.screenDensity = 2.0f;
    
    return Result::SUCCESS;
}

Result Server::initializeCapture() {
    m_screenCapture = std::make_unique<screen_capture::MediaProjectionCapture>();
    
    m_screenCapture->setFrameCallback([this](const byte_vector& frameData, uint64_t timestamp, const Resolution& resolution) {
        if (m_videoEncoder && m_videoEncoder->isEncoding()) {
            m_videoEncoder->encodeFrame(frameData, resolution, AIMAGE_FORMAT_RGBA_8888, timestamp);
        }
    });
    
    m_screenCapture->setErrorCallback([](Result error, const std::string& message) {
        LOG_E("Screen capture error: %s - %s", resultToString(error), message.c_str());
    });
    
    Result result = m_screenCapture->initialize(m_config.videoConfig.resolution);
    if (result != Result::SUCCESS) {
        return result;
    }
    
    return Result::SUCCESS;
}

Result Server::initializeEncoder() {
    m_videoEncoder = std::make_unique<video_encoder::MediaCodecH264Encoder>();
    
    m_videoEncoder->setEncodedDataCallback([this](const byte_vector& data, uint64_t pts, uint64_t /* dts */, bool keyframe, bool isConfig) {
        if (isConfig) {
            broadcastVideoConfigToAllClients(data);
        } else {
            broadcastVideoFrame(data, pts, keyframe);
        }
    });
    
    m_videoEncoder->setErrorCallback([](Result error, const std::string& message) {
        LOG_E("Video encoder error: %s - %s", resultToString(error), message.c_str());
    });
    
    Result result = m_videoEncoder->initialize(m_config.videoConfig);
    if (result != Result::SUCCESS) {
        return result;
    }
    
    return Result::SUCCESS;
}

Result Server::initializeAudio() {
    if (!m_config.enableAudio) {
        return Result::SUCCESS;
    }
    
    LOG_I("Audio capture initialization skipped (not implemented)");
    return Result::SUCCESS;
}

Result Server::initializeNetwork() {
    m_tcpServer = std::make_unique<network::TcpServer>();
    
    m_tcpServer->setConnectionCallback([this](uint32_t connectionId, const std::string& address, uint16_t port) {
        onClientConnected(connectionId, address, port);
    });
    
    m_tcpServer->setDisconnectionCallback([this](uint32_t connectionId) {
        onClientDisconnected(connectionId);
    });
    
    m_tcpServer->setDataCallback([this](uint32_t connectionId, const byte_vector& data) {
        onClientData(connectionId, data);
    });
    
    Result result = m_tcpServer->initialize(m_config.port, m_config.maxConnections);
    if (result != Result::SUCCESS) {
        return result;
    }
    
    return Result::SUCCESS;
}

void Server::captureThreadFunc() {
    
    while (m_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
}

void Server::networkThreadFunc() {
    
    while (m_running) {
        removeInactiveClients();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
}

void Server::heartbeatThreadFunc() {
    
    while (m_running) {
        broadcastHeartbeat();
        std::this_thread::sleep_for(std::chrono::milliseconds(m_config.heartbeatInterval));
    }
    
}

void Server::onClientConnected(uint32_t connectionId, const std::string& address, uint16_t port) {
    auto clientInfo = std::make_shared<ClientInfo>(connectionId, address, port);
    clientInfo->connectTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        m_clients.push_back(clientInfo);
    }
    
    sendMetadataToClient(connectionId);
    
    if (m_videoEncoder) {
        sendVideoConfigToClient(connectionId);
    }
    
}

void Server::onClientDisconnected(uint32_t connectionId) {
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        auto it = std::find_if(m_clients.begin(), m_clients.end(),
                              [connectionId](const auto& client) { 
                                  return client->connectionId == connectionId; 
                              });
        
        if (it != m_clients.end()) {
            (*it)->active = false;
            m_clients.erase(it);
        }
    }
    
}

void Server::onClientData(uint32_t connectionId, const byte_vector& data) {
    LOG_D("Received %zu bytes from client %d", data.size(), connectionId);
}

void Server::sendMetadataToClient(uint32_t connectionId) {
    protocol::MetadataPacket packet;
    
    strncpy(packet.deviceModel, m_deviceInfo.model.c_str(), sizeof(packet.deviceModel) - 1);
    strncpy(packet.deviceManufacturer, m_deviceInfo.manufacturer.c_str(), sizeof(packet.deviceManufacturer) - 1);
    strncpy(packet.androidVersion, m_deviceInfo.androidVersion.c_str(), sizeof(packet.androidVersion) - 1);
    
    packet.apiLevel = m_deviceInfo.apiLevel;
    packet.screenWidth = m_deviceInfo.screenResolution.width;
    packet.screenHeight = m_deviceInfo.screenResolution.height;
    packet.screenDensity = m_deviceInfo.screenDensity;
    packet.videoCodec = VideoCodec::H264;
    packet.audioCodec = AudioCodec::AAC;
    packet.videoBitrate = m_config.videoConfig.bitrate;
    packet.videoFps = m_config.videoConfig.fps;
    packet.audioBitrate = m_config.audioConfig.bitrate;
    packet.audioSampleRate = m_config.audioConfig.sampleRate;
    packet.audioChannels = m_config.audioConfig.channelCount;
    
    packet.header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    byte_vector data(reinterpret_cast<const uint8_t*>(&packet), 
                    reinterpret_cast<const uint8_t*>(&packet) + sizeof(packet));
    
    m_tcpServer->sendData(connectionId, data);
    
}

void Server::sendVideoConfigToClient(uint32_t connectionId) {
    if (!m_videoEncoder) return;
    
    auto configData = m_videoEncoder->getConfigurationData();
    if (configData.empty()) return;
    
    protocol::VideoConfigPacket packet;
    packet.configDataSize = static_cast<uint32_t>(configData.size());
    packet.header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    byte_vector data(reinterpret_cast<const uint8_t*>(&packet), 
                    reinterpret_cast<const uint8_t*>(&packet) + sizeof(packet));
    data.insert(data.end(), configData.begin(), configData.end());
    
    m_tcpServer->sendData(connectionId, data);
    
}

void Server::broadcastVideoFrame(const byte_vector& frameData, uint64_t pts, bool keyframe) {
    protocol::VideoDataPacket packet;
    packet.pts = pts;
    packet.dts = pts;
    packet.frameNumber = 0;
    packet.dataSize = static_cast<uint32_t>(frameData.size());
    
    if (keyframe) {
        packet.header.flags = protocol::PacketFlags::KEYFRAME;
    }
    
    packet.header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    byte_vector data(reinterpret_cast<const uint8_t*>(&packet), 
                    reinterpret_cast<const uint8_t*>(&packet) + sizeof(packet));
    data.insert(data.end(), frameData.begin(), frameData.end());
    
    m_tcpServer->sendDataToAll(data);
}

void Server::broadcastVideoConfigToAllClients(const byte_vector& configData) {
    protocol::VideoConfigPacket packet;
    packet.configDataSize = static_cast<uint32_t>(configData.size());
    packet.header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    byte_vector data(reinterpret_cast<const uint8_t*>(&packet), 
                    reinterpret_cast<const uint8_t*>(&packet) + sizeof(packet));
    data.insert(data.end(), configData.begin(), configData.end());
    
    m_tcpServer->sendDataToAll(data);
}

void Server::broadcastHeartbeat() {
    protocol::HeartbeatPacket packet;
    packet.serverTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    packet.connectionId = 0;
    
    byte_vector data(reinterpret_cast<const uint8_t*>(&packet), 
                    reinterpret_cast<const uint8_t*>(&packet) + sizeof(packet));
    
    m_tcpServer->sendDataToAll(data);
}

void Server::cleanup() {
    m_screenCapture.reset();
    m_videoEncoder.reset();
    m_audioCapture.reset();
    m_tcpServer.reset();
    
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        m_clients.clear();
    }
}

void Server::removeInactiveClients() {
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    
    m_clients.erase(
        std::remove_if(m_clients.begin(), m_clients.end(),
                      [](const auto& client) { return !client->active.load(); }),
        m_clients.end()
    );
}

} // namespace android_server 