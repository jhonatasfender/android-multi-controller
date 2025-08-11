#ifndef ANDROID_SERVER_PROTOCOL_HANDLER_H
#define ANDROID_SERVER_PROTOCOL_HANDLER_H

#include "../common/types.h"
#include "packet_types.h"
#include <functional>

namespace android_server::protocol
{
    class ProtocolHandler
    {
    public:
        using DataCallback = std::function<void(uint32_t, const byte_vector&)>;
        using ControlEventCallback = std::function<void(uint32_t, const ControlEventPacket&)>;

        ProtocolHandler();
        ~ProtocolHandler();

        static Result initialize();
        void cleanup();

        void setDataCallback(DataCallback callback);
        void setControlEventCallback(ControlEventCallback callback);

        Result processIncomingData(uint32_t connectionId, const byte_vector& data);

        byte_vector createMetadataPacket(const DeviceInfo& deviceInfo);
        byte_vector createVideoConfigPacket(const VideoConfig& config);
        byte_vector createAudioConfigPacket(const AudioConfig& config);
        byte_vector createVideoDataPacket(const byte_vector& frameData, uint64_t pts, bool keyframe);
        byte_vector createAudioDataPacket(const byte_vector& frameData, uint64_t pts);
        byte_vector createHeartbeatPacket();
        byte_vector createErrorPacket(ErrorCode code, const std::string& message);
        byte_vector createConnectionAckPacket();

        static uint32_t calculateCRC32(const byte_vector& data);
        static bool validatePacket(const byte_vector& data);

    private:
        DataCallback m_dataCallback;
        ControlEventCallback m_controlEventCallback;

        std::vector<byte_vector> m_pendingPackets;
        uint32_t m_sequenceNumber;

        Result parsePacket(uint32_t connectionId, const byte_vector& data);
        Result handleControlEvent(uint32_t connectionId, const byte_vector& packetData);

        byte_vector createPacket(PacketType type, const byte_vector& payload);
        PacketHeader createHeader(PacketType type, uint32_t length);
    };
}

#endif // ANDROID_SERVER_PROTOCOL_HANDLER_H
