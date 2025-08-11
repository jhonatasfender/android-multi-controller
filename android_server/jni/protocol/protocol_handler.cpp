#include "protocol_handler.h"
#include "../common/logger.h"

namespace android_server::protocol
{
    ProtocolHandler::ProtocolHandler() : m_sequenceNumber(0)
    {
    }

    ProtocolHandler::~ProtocolHandler()
    {
        cleanup();
    }

    Result ProtocolHandler::initialize()
    {
        return Result::SUCCESS;
    }

    void ProtocolHandler::cleanup()
    {
        m_pendingPackets.clear();
        m_dataCallback = nullptr;
        m_controlEventCallback = nullptr;
    }

    void ProtocolHandler::setDataCallback(DataCallback callback)
    {
        m_dataCallback = callback;
    }

    void ProtocolHandler::setControlEventCallback(ControlEventCallback callback)
    {
        m_controlEventCallback = callback;
    }

    Result ProtocolHandler::processIncomingData(uint32_t connectionId, const byte_vector& data)
    {
        if (m_dataCallback)
        {
            m_dataCallback(connectionId, data);
        }

        return Result::SUCCESS;
    }

    byte_vector ProtocolHandler::createMetadataPacket(const DeviceInfo& deviceInfo)
    {
        MetadataPacket packet;

        strncpy(packet.deviceModel, deviceInfo.model.c_str(), sizeof(packet.deviceModel) - 1);
        strncpy(packet.deviceManufacturer, deviceInfo.manufacturer.c_str(), sizeof(packet.deviceManufacturer) - 1);
        strncpy(packet.androidVersion, deviceInfo.androidVersion.c_str(), sizeof(packet.androidVersion) - 1);

        packet.apiLevel = deviceInfo.apiLevel;
        packet.screenWidth = deviceInfo.screenResolution.width;
        packet.screenHeight = deviceInfo.screenResolution.height;
        packet.screenDensity = deviceInfo.screenDensity;

        packet.header.sequence = m_sequenceNumber++;
        packet.header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

        return byte_vector(reinterpret_cast<const uint8_t*>(&packet),
                           reinterpret_cast<const uint8_t*>(&packet) + sizeof(packet));
    }

    byte_vector ProtocolHandler::createVideoConfigPacket(const VideoConfig& config)
    {
        VideoConfigPacket packet;
        packet.configDataSize = 0;
        packet.header.sequence = m_sequenceNumber++;
        packet.header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

        return byte_vector(reinterpret_cast<const uint8_t*>(&packet),
                           reinterpret_cast<const uint8_t*>(&packet) + sizeof(packet));
    }

    byte_vector ProtocolHandler::createAudioConfigPacket(const AudioConfig& config)
    {
        AudioConfigPacket packet;
        packet.sampleRate = config.sampleRate;
        packet.channels = static_cast<uint16_t>(config.channelCount);
        packet.bitsPerSample = 16;
        packet.configDataSize = 0;
        packet.header.sequence = m_sequenceNumber++;
        packet.header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

        return byte_vector(reinterpret_cast<const uint8_t*>(&packet),
                           reinterpret_cast<const uint8_t*>(&packet) + sizeof(packet));
    }

    byte_vector ProtocolHandler::createVideoDataPacket(const byte_vector& frameData, uint64_t pts, bool keyframe)
    {
        VideoDataPacket packet;
        packet.pts = pts;
        packet.dts = pts;
        packet.frameNumber = 0;
        packet.dataSize = static_cast<uint32_t>(frameData.size());

        if (keyframe)
        {
            packet.header.flags = PacketFlags::KEYFRAME;
        }

        packet.header.sequence = m_sequenceNumber++;
        packet.header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

        byte_vector data(reinterpret_cast<const uint8_t*>(&packet),
                         reinterpret_cast<const uint8_t*>(&packet) + sizeof(packet));
        data.insert(data.end(), frameData.begin(), frameData.end());

        return data;
    }

    byte_vector ProtocolHandler::createAudioDataPacket(const byte_vector& frameData, uint64_t pts)
    {
        AudioDataPacket packet;
        packet.pts = pts;
        packet.frameNumber = 0;
        packet.dataSize = static_cast<uint32_t>(frameData.size());
        packet.header.sequence = m_sequenceNumber++;
        packet.header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

        byte_vector data(reinterpret_cast<const uint8_t*>(&packet),
                         reinterpret_cast<const uint8_t*>(&packet) + sizeof(packet));
        data.insert(data.end(), frameData.begin(), frameData.end());

        return data;
    }

    byte_vector ProtocolHandler::createHeartbeatPacket()
    {
        HeartbeatPacket packet;
        packet.serverTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        packet.connectionId = 0;
        packet.header.sequence = m_sequenceNumber++;
        packet.header.timestamp = packet.serverTime;

        return byte_vector(reinterpret_cast<const uint8_t*>(&packet),
                           reinterpret_cast<const uint8_t*>(&packet) + sizeof(packet));
    }

    byte_vector ProtocolHandler::createErrorPacket(ErrorCode code, const std::string& message)
    {
        ErrorPacket packet;
        packet.errorCode = static_cast<uint32_t>(code);
        packet.messageLength = static_cast<uint32_t>(message.size());
        packet.header.sequence = m_sequenceNumber++;
        packet.header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

        byte_vector data(reinterpret_cast<const uint8_t*>(&packet),
                         reinterpret_cast<const uint8_t*>(&packet) + sizeof(packet));
        data.insert(data.end(), message.begin(), message.end());

        return data;
    }

    byte_vector ProtocolHandler::createConnectionAckPacket()
    {
        ConnectionAckPacket packet;
        packet.connectionId = 0;
        packet.maxPacketSize = 65536;
        packet.bufferSize = 1048576;
        packet.header.sequence = m_sequenceNumber++;
        packet.header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

        return byte_vector(reinterpret_cast<const uint8_t*>(&packet),
                           reinterpret_cast<const uint8_t*>(&packet) + sizeof(packet));
    }

    uint32_t ProtocolHandler::calculateCRC32(const byte_vector& data)
    {
        // Simple CRC32 implementation placeholder
        uint32_t crc = 0xFFFFFFFF;
        for (uint8_t byte : data)
        {
            crc ^= byte;
            for (int i = 0; i < 8; i++)
            {
                if (crc & 1)
                {
                    crc = (crc >> 1) ^ 0xEDB88320;
                }
                else
                {
                    crc >>= 1;
                }
            }
        }
        return ~crc;
    }

    bool ProtocolHandler::validatePacket(const byte_vector& data)
    {
        if (data.size() < sizeof(PacketHeader))
        {
            return false;
        }

        const PacketHeader* header = reinterpret_cast<const PacketHeader*>(data.data());
        return header->magic == PACKET_MAGIC && header->version == PROTOCOL_VERSION;
    }

    Result ProtocolHandler::parsePacket(uint32_t connectionId, const byte_vector& data)
    {
        if (!validatePacket(data))
        {
            return Result::ERROR_INVALID_PARAMS;
        }

        return Result::SUCCESS;
    }

    Result ProtocolHandler::handleControlEvent(uint32_t connectionId, const byte_vector& packetData)
    {
        if (packetData.size() < sizeof(ControlEventPacket))
        {
            return Result::ERROR_INVALID_PARAMS;
        }

        const ControlEventPacket* packet = reinterpret_cast<const ControlEventPacket*>(packetData.data());

        if (m_controlEventCallback)
        {
            m_controlEventCallback(connectionId, *packet);
        }

        return Result::SUCCESS;
    }

    byte_vector ProtocolHandler::createPacket(PacketType type, const byte_vector& payload)
    {
        PacketHeader header = createHeader(type, static_cast<uint32_t>(sizeof(PacketHeader) + payload.size()));

        byte_vector packet(reinterpret_cast<const uint8_t*>(&header),
                           reinterpret_cast<const uint8_t*>(&header) + sizeof(header));
        packet.insert(packet.end(), payload.begin(), payload.end());

        return packet;
    }

    PacketHeader ProtocolHandler::createHeader(PacketType type, uint32_t length)
    {
        PacketHeader header;
        header.magic = PACKET_MAGIC;
        header.version = PROTOCOL_VERSION;
        header.type = type;
        header.flags = PacketFlags::NONE;
        header.length = length;
        header.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        header.sequence = m_sequenceNumber++;
        header.crc32 = 0;

        return header;
    }
}
