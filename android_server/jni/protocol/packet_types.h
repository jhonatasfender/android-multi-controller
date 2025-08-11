#ifndef ANDROID_SERVER_PROTOCOL_PACKET_TYPES_H
#define ANDROID_SERVER_PROTOCOL_PACKET_TYPES_H

#include "../common/types.h"
#include <cstring>

namespace android_server::protocol
{
    constexpr uint32_t PACKET_MAGIC = 0x53435250;
    constexpr uint16_t PROTOCOL_VERSION = 1;

    enum class PacketType : uint8_t
    {
        METADATA = 0x01,
        VIDEO_CONFIG = 0x02,
        VIDEO_DATA = 0x03,
        AUDIO_CONFIG = 0x04,
        AUDIO_DATA = 0x05,
        CONTROL_EVENT = 0x06,
        HEARTBEAT = 0x07,
        ERROR_MESSAGE = 0x08,
        CONNECTION_ACK = 0x09
    };

    enum class ErrorCode : uint32_t
    {
        NO_ERROR = 0,
        INVALID_PACKET = 1,
        PROTOCOL_MISMATCH = 2,
        AUTHENTICATION_FAILED = 3,
        ENCODER_ERROR = 4,
        CAPTURE_ERROR = 5,
        NETWORK_ERROR = 6,
        INSUFFICIENT_RESOURCES = 7,
        UNKNOWN_ERROR = 999
    };

    enum class PacketFlags : uint8_t
    {
        NONE = 0x00,
        KEYFRAME = 0x01,
        CONFIG_PACKET = 0x02,
        END_OF_STREAM = 0x04,
        ENCRYPTED = 0x08
    };

    inline PacketFlags operator|(PacketFlags a, PacketFlags b)
    {
        return static_cast<PacketFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    inline PacketFlags operator&(PacketFlags a, PacketFlags b)
    {
        return static_cast<PacketFlags>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }

    inline bool hasFlag(const PacketFlags flags, const PacketFlags flag)
    {
        return (flags & flag) == flag;
    }

    struct PacketHeader
    {
        uint32_t magic;
        uint16_t version;
        PacketType type;
        PacketFlags flags;
        uint32_t length;
        uint64_t timestamp;
        uint32_t sequence;
        uint32_t crc32;

        PacketHeader()
            : magic(PACKET_MAGIC)
              , version(PROTOCOL_VERSION)
              , type(PacketType::METADATA)
              , flags(PacketFlags::NONE)
              , length(sizeof(PacketHeader))
              , timestamp(0)
              , sequence(0)
              , crc32(0)
        {
        }
    } __attribute__((packed));

    struct MetadataPacket
    {
        PacketHeader header;
        char deviceModel[64];
        char deviceManufacturer[64];
        char androidVersion[16];
        uint32_t apiLevel;
        uint32_t screenWidth;
        uint32_t screenHeight;
        float screenDensity;
        VideoCodec videoCodec;
        AudioCodec audioCodec;
        uint32_t videoBitrate;
        uint32_t videoFps;
        uint32_t audioBitrate;
        uint32_t audioSampleRate;
        uint32_t audioChannels;

        MetadataPacket()
        {
            header.type = PacketType::METADATA;
            header.length = sizeof(MetadataPacket);
            memset(deviceModel, 0, sizeof(deviceModel));
            memset(deviceManufacturer, 0, sizeof(deviceManufacturer));
            memset(androidVersion, 0, sizeof(androidVersion));
            apiLevel = 0;
            screenWidth = 0;
            screenHeight = 0;
            screenDensity = 1.0f;
            videoCodec = VideoCodec::H264;
            audioCodec = AudioCodec::AAC;
            videoBitrate = 4000000;
            videoFps = 30;
            audioBitrate = 128000;
            audioSampleRate = 44100;
            audioChannels = 2;
        }
    } __attribute__((packed));

    struct VideoConfigPacket
    {
        PacketHeader header;
        uint32_t configDataSize;

        VideoConfigPacket()
        {
            header.type = PacketType::VIDEO_CONFIG;
            header.flags = PacketFlags::CONFIG_PACKET;
            header.length = sizeof(VideoConfigPacket);
            configDataSize = 0;
        }
    } __attribute__((packed));

    struct VideoDataPacket
    {
        PacketHeader header;
        uint64_t pts;
        uint64_t dts;
        uint32_t frameNumber;
        uint32_t dataSize;

        VideoDataPacket()
        {
            header.type = PacketType::VIDEO_DATA;
            header.length = sizeof(VideoDataPacket);
            pts = 0;
            dts = 0;
            frameNumber = 0;
            dataSize = 0;
        }
    } __attribute__((packed));

    struct AudioConfigPacket
    {
        PacketHeader header;
        uint32_t sampleRate;
        uint16_t channels;
        uint16_t bitsPerSample;
        uint32_t configDataSize;

        AudioConfigPacket()
        {
            header.type = PacketType::AUDIO_CONFIG;
            header.flags = PacketFlags::CONFIG_PACKET;
            header.length = sizeof(AudioConfigPacket);
            sampleRate = 44100;
            channels = 2;
            bitsPerSample = 16;
            configDataSize = 0;
        }
    } __attribute__((packed));

    struct AudioDataPacket
    {
        PacketHeader header;
        uint64_t pts;
        uint32_t frameNumber;
        uint32_t dataSize;

        AudioDataPacket()
        {
            header.type = PacketType::AUDIO_DATA;
            header.length = sizeof(AudioDataPacket);
            pts = 0;
            frameNumber = 0;
            dataSize = 0;
        }
    } __attribute__((packed));

    struct ControlEventPacket
    {
        PacketHeader header;
        InputEventType eventType;
        uint32_t eventDataSize;

        ControlEventPacket()
        {
            header.type = PacketType::CONTROL_EVENT;
            header.length = sizeof(ControlEventPacket);
            eventType = InputEventType::TOUCH_DOWN;
            eventDataSize = 0;
        }
    } __attribute__((packed));

    struct HeartbeatPacket
    {
        PacketHeader header;
        uint64_t serverTime;
        uint32_t connectionId;

        HeartbeatPacket()
        {
            header.type = PacketType::HEARTBEAT;
            header.length = sizeof(HeartbeatPacket);
            serverTime = 0;
            connectionId = 0;
        }
    } __attribute__((packed));

    struct ErrorPacket
    {
        PacketHeader header;
        uint32_t errorCode;
        uint32_t messageLength;

        ErrorPacket()
        {
            header.type = PacketType::ERROR_MESSAGE;
            header.length = sizeof(ErrorPacket);
            errorCode = 0;
            messageLength = 0;
        }
    } __attribute__((packed));

    struct ConnectionAckPacket
    {
        PacketHeader header;
        uint32_t connectionId;
        uint32_t maxPacketSize;
        uint32_t bufferSize;

        ConnectionAckPacket()
        {
            header.type = PacketType::CONNECTION_ACK;
            header.length = sizeof(ConnectionAckPacket);
            connectionId = 0;
            maxPacketSize = 65536;
            bufferSize = 1048576;
        }
    } __attribute__((packed));
}

#endif // ANDROID_SERVER_PROTOCOL_PACKET_TYPES_H
