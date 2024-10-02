#ifndef DOIP_CLIENT_H
#define DOIP_CLIENT_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <chrono>
#include "doip_message.h"
#include "uds_client.h"

namespace DoIP {

class ConnectionError : public std::runtime_error {
public:
    explicit ConnectionError(const std::string& what_arg) : std::runtime_error(what_arg) {}
};

class DoIPClient {
public:
    DoIPClient(const std::string& serverIP, uint16_t port);
    ~DoIPClient();

    // Prevent copying
    DoIPClient(const DoIPClient&) = delete;
    DoIPClient& operator=(const DoIPClient&) = delete;

    // Allow moving
    DoIPClient(DoIPClient&&) noexcept;
    DoIPClient& operator=(DoIPClient&&) noexcept;

    // Connection management
    void connect();
    void disconnect();
    bool isConnected() const;

    // DoIP message sending
    void sendMessage(const Message& message);
    Message receiveMessage();

    // High-level DoIP operations
    Message sendVehicleIdentificationRequest();
    Message sendRoutingActivationRequest(uint16_t sourceAddress);
    Message sendEntityStatusRequest();
    Message sendDiagnosticMessage(uint16_t sourceAddress, uint16_t targetAddress, const std::vector<uint8_t>& data);

    // UDS integration
    Message sendUDSRequest(uint16_t sourceAddress, uint16_t targetAddress, UDS::ServiceID service, const std::vector<uint8_t>& data);

    // Configuration
    void setResponseTimeout(std::chrono::milliseconds timeout);
    void setRetryCount(int count);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;

    // Helper methods
    void ensureConnected() const;
    Message sendAndReceive(const Message& message);
};

} // namespace DoIP

#endif // DOIP_CLIENT_H
