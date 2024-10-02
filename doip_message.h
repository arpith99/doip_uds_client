#ifndef DOIP_MESSAGE_H
#define DOIP_MESSAGE_H

#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <stdexcept>

namespace DoIP {

// Constants
constexpr uint8_t PROTOCOL_VERSION = 0x02;
constexpr uint8_t INVERSE_PROTOCOL_VERSION = 0xFD;
constexpr size_t HEADER_SIZE = 8;

// Payload types
enum class PayloadType : uint16_t {
    GenericDoIPHeader_NACK                  = 0x0000,
    VehicleIdentificationRequest            = 0x0001,
    VehicleIdentificationRequestWithEID     = 0x0002,
    VehicleIdentificationRequestWithVIN     = 0x0003,
    VehicleAnnouncementMessage              = 0x0004,
    RoutingActivationRequest                = 0x0005,
    RoutingActivationResponse               = 0x0006,
    AliveCheckRequest                       = 0x0007,
    AliveCheckResponse                      = 0x0008,
    DoIPEntityStatusRequest                 = 0x4001,
    DoIPEntityStatusResponse                = 0x4002,
    DiagnosticMessage                       = 0x8001,
    DiagnosticMessagePositiveAcknowledgement= 0x8002,
    DiagnosticMessageNegativeAcknowledgement= 0x8003
};

// DoIP message structure
struct Message {
    uint8_t protocolVersion;
    uint8_t inverseProtocolVersion;
    PayloadType payloadType;
    uint32_t payloadLength;
    std::vector<uint8_t> payload;

    Message() : protocolVersion(PROTOCOL_VERSION), inverseProtocolVersion(INVERSE_PROTOCOL_VERSION),
                payloadType(PayloadType::GenericDoIPHeader_NACK), payloadLength(0) {}

    // Constructor for creating a message with a specific payload type
    Message(PayloadType type) : Message() {
        payloadType = type;
    }

    // Serialize the message to a byte vector
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> result(HEADER_SIZE + payload.size());
        result[0] = protocolVersion;
        result[1] = inverseProtocolVersion;
        result[2] = static_cast<uint8_t>((static_cast<uint16_t>(payloadType) >> 8) & 0xFF);
        result[3] = static_cast<uint8_t>(static_cast<uint16_t>(payloadType) & 0xFF);
        for (int i = 0; i < 4; ++i) {
            result[4 + i] = static_cast<uint8_t>((payloadLength >> (24 - i * 8)) & 0xFF);
        }
        std::copy(payload.begin(), payload.end(), result.begin() + HEADER_SIZE);
        return result;
    }

    // Deserialize a byte vector into a DoIP message
    static Message deserialize(const std::vector<uint8_t>& data) {
        if (data.size() < HEADER_SIZE) {
            throw std::runtime_error("Insufficient data for DoIP header");
        }

        Message msg;
        msg.protocolVersion = data[0];
        msg.inverseProtocolVersion = data[1];
        msg.payloadType = static_cast<PayloadType>((static_cast<uint16_t>(data[2]) << 8) | data[3]);
        msg.payloadLength = (static_cast<uint32_t>(data[4]) << 24) |
                            (static_cast<uint32_t>(data[5]) << 16) |
                            (static_cast<uint32_t>(data[6]) << 8) |
                            static_cast<uint32_t>(data[7]);

        if (data.size() < HEADER_SIZE + msg.payloadLength) {
            throw std::runtime_error("Insufficient data for DoIP payload");
        }

        msg.payload.assign(data.begin() + HEADER_SIZE, data.begin() + HEADER_SIZE + msg.payloadLength);
        return msg;
    }
};

// Utility functions

// Convert PayloadType to string for logging/debugging
std::string payloadTypeToString(PayloadType type);

} // namespace DoIP

#endif // DOIP_MESSAGE_H
