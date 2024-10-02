#ifndef UDS_CLIENT_H
#define UDS_CLIENT_H

#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <functional>

namespace UDS {

class UDSError : public std::runtime_error {
public:
    explicit UDSError(const std::string& what_arg) : std::runtime_error(what_arg) {}
};

enum class ServiceID : uint8_t {
    DiagnosticSessionControl = 0x10,
    ECUReset = 0x11,
    SecurityAccess = 0x27,
    CommunicationControl = 0x28,
    TesterPresent = 0x3E,
    AccessTimingParameter = 0x83,
    SecuredDataTransmission = 0x84,
    ControlDTCSetting = 0x85,
    ResponseOnEvent = 0x86,
    LinkControl = 0x87,
    ReadDataByIdentifier = 0x22,
    ReadMemoryByAddress = 0x23,
    ReadScalingDataByIdentifier = 0x24,
    ReadDataByPeriodicIdentifier = 0x2A,
    DynamicallyDefineDataIdentifier = 0x2C,
    WriteDataByIdentifier = 0x2E,
    WriteMemoryByAddress = 0x3D,
    ClearDiagnosticInformation = 0x14,
    ReadDTCInformation = 0x19,
    InputOutputControlByIdentifier = 0x2F,
    RoutineControl = 0x31,
    RequestDownload = 0x34,
    RequestUpload = 0x35,
    TransferData = 0x36,
    RequestTransferExit = 0x37
};

struct UDSMessage {
    ServiceID service;
    std::vector<uint8_t> data;

    UDSMessage(ServiceID sid, std::vector<uint8_t> payload) : service(sid), data(std::move(payload)) {}
};

class UDSClient {
public:
    UDSClient() = default;
    ~UDSClient() = default;

    // Prevent copying
    UDSClient(const UDSClient&) = delete;
    UDSClient& operator=(const UDSClient&) = delete;

    // Allow moving
    UDSClient(UDSClient&&) = default;
    UDSClient& operator=(UDSClient&&) = default;

    // Create a UDS request
    static UDSMessage createRequest(ServiceID service, const std::vector<uint8_t>& data);

    // Interpret a UDS response
    std::string interpretResponse(const UDSMessage& response);

    // High-level UDS operations
    UDSMessage diagnosticSessionControl(uint8_t sessionType);
    UDSMessage ecuReset(uint8_t resetType);
    UDSMessage readDataByIdentifier(uint16_t dataIdentifier);
    UDSMessage writeDataByIdentifier(uint16_t dataIdentifier, const std::vector<uint8_t>& data);
    UDSMessage routineControl(uint8_t routineControlType, uint16_t routineIdentifier, const std::vector<uint8_t>& routineControlOptions);

    // Add a custom service handler
    void addServiceHandler(ServiceID service, std::function<std::string(const std::vector<uint8_t>&)> handler);

private:
    std::unordered_map<ServiceID, std::function<std::string(const std::vector<uint8_t>&)>> serviceHandlers;

    // Helper methods
    static std::string getServiceName(ServiceID service);
    static std::string formatHex(const std::vector<uint8_t>& data);
};

} // namespace UDS

#endif // UDS_CLIENT_H
