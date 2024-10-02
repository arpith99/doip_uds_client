#include "doip_message.h"

namespace DoIP {

std::string payloadTypeToString(PayloadType type) {
    switch (type) {
        case PayloadType::GenericDoIPHeader_NACK: return "Generic DoIP Header NACK";
        case PayloadType::VehicleIdentificationRequest: return "Vehicle Identification Request";
        case PayloadType::VehicleIdentificationRequestWithEID: return "Vehicle Identification Request with EID";
        case PayloadType::VehicleIdentificationRequestWithVIN: return "Vehicle Identification Request with VIN";
        case PayloadType::VehicleAnnouncementMessage: return "Vehicle Announcement Message";
        case PayloadType::RoutingActivationRequest: return "Routing Activation Request";
        case PayloadType::RoutingActivationResponse: return "Routing Activation Response";
        case PayloadType::AliveCheckRequest: return "Alive Check Request";
        case PayloadType::AliveCheckResponse: return "Alive Check Response";
        case PayloadType::DoIPEntityStatusRequest: return "DoIP Entity Status Request";
        case PayloadType::DoIPEntityStatusResponse: return "DoIP Entity Status Response";
        case PayloadType::DiagnosticMessage: return "Diagnostic Message";
        case PayloadType::DiagnosticMessagePositiveAcknowledgement: return "Diagnostic Message Positive Acknowledgement";
        case PayloadType::DiagnosticMessageNegativeAcknowledgement: return "Diagnostic Message Negative Acknowledgement";
        default: return "Unknown Payload Type";
    }
}

} // namespace DoIP
