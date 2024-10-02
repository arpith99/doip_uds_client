#include "uds_client.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace UDS {

UDSMessage UDSClient::createRequest(ServiceID service, const std::vector<uint8_t>& data) {
    return UDSMessage(service, data);
}

std::string UDSClient::interpretResponse(const UDSMessage& response) {
    std::ostringstream oss;
    oss << "Service: " << getServiceName(response.service) << " (0x" 
        << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(response.service) << ")\n";

    if (!response.data.empty()) {
        oss << "Data: " << formatHex(response.data) << "\n";
    }

    // Check if we have a custom handler for this service
    auto it = serviceHandlers.find(response.service);
    if (it != serviceHandlers.end()) {
        oss << it->second(response.data);
    } else {
        // Default interpretation
        if (response.data[0] == 0x00) {
            oss << "Status: Positive Response\n";
        } else {
            oss << "Status: Negative Response\n";
            if (response.data.size() > 1) {
                oss << "NRC: 0x" << std::hex << std::setw(2) << std::setfill('0') 
                    << static_cast<int>(response.data[1]) << "\n";
            }
        }
    }

    return oss.str();
}

UDSMessage UDSClient::diagnosticSessionControl(uint8_t sessionType) {
    return createRequest(ServiceID::DiagnosticSessionControl, {sessionType});
}

UDSMessage UDSClient::ecuReset(uint8_t resetType) {
    return createRequest(ServiceID::ECUReset, {resetType});
}

UDSMessage UDSClient::readDataByIdentifier(uint16_t dataIdentifier) {
    return createRequest(ServiceID::ReadDataByIdentifier, {
        static_cast<uint8_t>((dataIdentifier >> 8) & 0xFF),
        static_cast<uint8_t>(dataIdentifier & 0xFF)
    });
}

UDSMessage UDSClient::writeDataByIdentifier(uint16_t dataIdentifier, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> payload = {
        static_cast<uint8_t>((dataIdentifier >> 8) & 0xFF),
        static_cast<uint8_t>(dataIdentifier & 0xFF)
    };
    payload.insert(payload.end(), data.begin(), data.end());
    return createRequest(ServiceID::WriteDataByIdentifier, payload);
}

UDSMessage UDSClient::routineControl(uint8_t routineControlType, uint16_t routineIdentifier, const std::vector<uint8_t>& routineControlOptions) {
    std::vector<uint8_t> payload = {
        routineControlType,
        static_cast<uint8_t>((routineIdentifier >> 8) & 0xFF),
        static_cast<uint8_t>(routineIdentifier & 0xFF)
    };
    payload.insert(payload.end(), routineControlOptions.begin(), routineControlOptions.end());
    return createRequest(ServiceID::RoutineControl, payload);
}

void UDSClient::addServiceHandler(ServiceID service, std::function<std::string(const std::vector<uint8_t>&)> handler) {
    serviceHandlers[service] = std::move(handler);
}

std::string UDSClient::getServiceName(ServiceID service) {
    switch (service) {
        case ServiceID::DiagnosticSessionControl: return "DiagnosticSessionControl";
        case ServiceID::ECUReset: return "ECUReset";
        case ServiceID::SecurityAccess: return "SecurityAccess";
        case ServiceID::CommunicationControl: return "CommunicationControl";
        case ServiceID::TesterPresent: return "TesterPresent";
        case ServiceID::AccessTimingParameter: return "AccessTimingParameter";
        case ServiceID::SecuredDataTransmission: return "SecuredDataTransmission";
        case ServiceID::ControlDTCSetting: return "ControlDTCSetting";
        case ServiceID::ResponseOnEvent: return "ResponseOnEvent";
        case ServiceID::LinkControl: return "LinkControl";
        case ServiceID::ReadDataByIdentifier: return "ReadDataByIdentifier";
        case ServiceID::ReadMemoryByAddress: return "ReadMemoryByAddress";
        case ServiceID::ReadScalingDataByIdentifier: return "ReadScalingDataByIdentifier";
        case ServiceID::ReadDataByPeriodicIdentifier: return "ReadDataByPeriodicIdentifier";
        case ServiceID::DynamicallyDefineDataIdentifier: return "DynamicallyDefineDataIdentifier";
        case ServiceID::WriteDataByIdentifier: return "WriteDataByIdentifier";
        case ServiceID::WriteMemoryByAddress: return "WriteMemoryByAddress";
        case ServiceID::ClearDiagnosticInformation: return "ClearDiagnosticInformation";
        case ServiceID::ReadDTCInformation: return "ReadDTCInformation";
        case ServiceID::InputOutputControlByIdentifier: return "InputOutputControlByIdentifier";
        case ServiceID::RoutineControl: return "RoutineControl";
        case ServiceID::RequestDownload: return "RequestDownload";
        case ServiceID::RequestUpload: return "RequestUpload";
        case ServiceID::TransferData: return "TransferData";
        case ServiceID::RequestTransferExit: return "RequestTransferExit";
        default: return "Unknown";
    }
}

std::string UDSClient::formatHex(const std::vector<uint8_t>& data) {
    std::ostringstream oss;
    for (auto byte : data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    return oss.str();
}

} // namespace UDS
