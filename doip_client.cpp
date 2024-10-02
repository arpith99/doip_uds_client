#include "doip_client.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <chrono>
#include <thread>

namespace DoIP {

class DoIPClient::Impl {
public:
    Impl(const std::string& serverIP, uint16_t port)
        : serverIP(serverIP), port(port), sock(-1), connected(false),
          responseTimeout(std::chrono::seconds(5)), retryCount(3) {}

    ~Impl() {
        disconnect();
    }

    void connect() {
        if (connected) {
            return;
        }

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            throw ConnectionError("Failed to create socket");
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
            throw ConnectionError("Invalid address/ Address not supported");
        }

        if (::connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
            throw ConnectionError("Connection Failed");
        }

        connected = true;
    }

    void disconnect() {
        if (connected) {
            close(sock);
            connected = false;
        }
    }

    bool isConnected() const {
        return connected;
    }

    void sendMessage(const Message& message) {
        auto serialized = message.serialize();
        if (send(sock, serialized.data(), serialized.size(), 0) < 0) {
            throw ConnectionError("Failed to send message");
        }
    }

    Message receiveMessage() {
        std::vector<uint8_t> buffer(HEADER_SIZE);
        ssize_t bytesReceived = recv(sock, buffer.data(), HEADER_SIZE, 0);
        if (bytesReceived != HEADER_SIZE) {
            throw ConnectionError("Failed to receive message header");
        }

        uint32_t payloadLength = (static_cast<uint32_t>(buffer[4]) << 24) |
                                 (static_cast<uint32_t>(buffer[5]) << 16) |
                                 (static_cast<uint32_t>(buffer[6]) << 8) |
                                 static_cast<uint32_t>(buffer[7]);

        buffer.resize(HEADER_SIZE + payloadLength);
        bytesReceived = recv(sock, buffer.data() + HEADER_SIZE, payloadLength, 0);
        if (bytesReceived != static_cast<ssize_t>(payloadLength)) {
            throw ConnectionError("Failed to receive message payload");
        }

        return Message::deserialize(buffer);
    }

    Message sendAndReceive(const Message& message) {
        for (int attempt = 0; attempt < retryCount; ++attempt) {
            try {
                sendMessage(message);

                pollfd pfd = {sock, POLLIN, 0};
                int pollResult = poll(&pfd, 1, static_cast<int>(responseTimeout.count()));

                if (pollResult > 0) {
                    if (pfd.revents & POLLIN) {
                        return receiveMessage();
                    }
                } else if (pollResult == 0) {
                    throw ConnectionError("Response timeout");
                } else {
                    throw ConnectionError("Poll error");
                }
            } catch (const ConnectionError& e) {
                if (attempt == retryCount - 1) {
                    throw;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        throw ConnectionError("Max retry attempts reached");
    }

    std::string serverIP;
    uint16_t port;
    int sock;
    bool connected;
    std::chrono::milliseconds responseTimeout;
    int retryCount;
};

DoIPClient::DoIPClient(const std::string& serverIP, uint16_t port)
    : pImpl(std::make_unique<Impl>(serverIP, port)) {}

DoIPClient::~DoIPClient() = default;

DoIPClient::DoIPClient(DoIPClient&&) noexcept = default;
DoIPClient& DoIPClient::operator=(DoIPClient&&) noexcept = default;

void DoIPClient::connect() { pImpl->connect(); }
void DoIPClient::disconnect() { pImpl->disconnect(); }
bool DoIPClient::isConnected() const { return pImpl->isConnected(); }

void DoIPClient::sendMessage(const Message& message) {
    ensureConnected();
    pImpl->sendMessage(message);
}

Message DoIPClient::receiveMessage() {
    ensureConnected();
    return pImpl->receiveMessage();
}

Message DoIPClient::sendVehicleIdentificationRequest() {
    Message request(PayloadType::VehicleIdentificationRequest);
    return sendAndReceive(request);
}

Message DoIPClient::sendRoutingActivationRequest(uint16_t sourceAddress) {
    Message request(PayloadType::RoutingActivationRequest);
    request.payload = {
        static_cast<uint8_t>((sourceAddress >> 8) & 0xFF),
        static_cast<uint8_t>(sourceAddress & 0xFF),
        0x00,  // Activation type (Default)
        0x00, 0x00, 0x00, 0x00  // Reserved
    };
    request.payloadLength = request.payload.size();
    return sendAndReceive(request);
}

Message DoIPClient::sendEntityStatusRequest() {
    Message request(PayloadType::DoIPEntityStatusRequest);
    return sendAndReceive(request);
}

Message DoIPClient::sendDiagnosticMessage(uint16_t sourceAddress, uint16_t targetAddress, const std::vector<uint8_t>& data) {
    Message request(PayloadType::DiagnosticMessage);
    request.payload.reserve(4 + data.size());
    request.payload.push_back(static_cast<uint8_t>((sourceAddress >> 8) & 0xFF));
    request.payload.push_back(static_cast<uint8_t>(sourceAddress & 0xFF));
    request.payload.push_back(static_cast<uint8_t>((targetAddress >> 8) & 0xFF));
    request.payload.push_back(static_cast<uint8_t>(targetAddress & 0xFF));
    request.payload.insert(request.payload.end(), data.begin(), data.end());
    request.payloadLength = request.payload.size();
    return sendAndReceive(request);
}

Message DoIPClient::sendUDSRequest(uint16_t sourceAddress, uint16_t targetAddress, UDS::ServiceID service, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> udsData;
    udsData.push_back(static_cast<uint8_t>(service));
    udsData.insert(udsData.end(), data.begin(), data.end());
    return sendDiagnosticMessage(sourceAddress, targetAddress, udsData);
}

void DoIPClient::setResponseTimeout(std::chrono::milliseconds timeout) {
    pImpl->responseTimeout = timeout;
}

void DoIPClient::setRetryCount(int count) {
    pImpl->retryCount = count;
}

void DoIPClient::ensureConnected() const {
    if (!isConnected()) {
        throw ConnectionError("Not connected to DoIP server");
    }
}

Message DoIPClient::sendAndReceive(const Message& message) {
    ensureConnected();
    return pImpl->sendAndReceive(message);
}

} // namespace DoIP
