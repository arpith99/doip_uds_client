#include "doip_client.h"
#include "uds_client.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

void printSeparator() {
    std::cout << std::string(50, '-') << std::endl;
}

int main() {
    try {
        // Initialize DoIP client
        DoIP::DoIPClient doipClient("192.168.1.10", 13400);
        doipClient.setResponseTimeout(std::chrono::seconds(5));
        doipClient.setRetryCount(3);

        // Initialize UDS client
        UDS::UDSClient udsClient;

        // Connect to the DoIP server
        std::cout << "Connecting to DoIP server..." << std::endl;
        doipClient.connect();
        std::cout << "Connected successfully." << std::endl;
        printSeparator();

        // Send Vehicle Identification Request
        std::cout << "Sending Vehicle Identification Request..." << std::endl;
        auto vehIdResponse = doipClient.sendVehicleIdentificationRequest();
        std::cout << "Received: " << DoIP::payloadTypeToString(vehIdResponse.payloadType) << std::endl;
        printSeparator();

        // Activate diagnostic session
        std::cout << "Activating diagnostic session..." << std::endl;
        auto routingResponse = doipClient.sendRoutingActivationRequest(0x0E80);
        std::cout << "Routing activation response: " << DoIP::payloadTypeToString(routingResponse.payloadType) << std::endl;
        printSeparator();

        // Add custom handlers for UDS responses
        udsClient.addServiceHandler(UDS::ServiceID::ReadDataByIdentifier,
            [](const std::vector<uint8_t>& data) {
                std::ostringstream oss;
                if (data.size() >= 3) {
                    uint16_t did = (static_cast<uint16_t>(data[1]) << 8) | data[2];
                    oss << "DID: 0x" << std::hex << std::setw(4) << std::setfill('0') << did << "\n";
                    oss << "Value: ";
                    for (size_t i = 3; i < data.size(); ++i) {
                        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
                    }
                }
                return oss.str();
            });

        // Read Vehicle Identification Number (VIN)
        std::cout << "Reading Vehicle Identification Number..." << std::endl;
        auto vinRequest = udsClient.readDataByIdentifier(0xF190);
        auto vinResponse = doipClient.sendUDSRequest(0x0E80, 0x0EE0, vinRequest.service, vinRequest.data);
        UDS::UDSMessage udsVinResponse(static_cast<UDS::ServiceID>(vinResponse.payload[4]),
                                       std::vector<uint8_t>(vinResponse.payload.begin() + 5, vinResponse.payload.end()));
        std::cout << "VIN Response:\n" << udsClient.interpretResponse(udsVinResponse) << std::endl;
        printSeparator();

        // Read DTCs
        std::cout << "Reading Diagnostic Trouble Codes..." << std::endl;
        auto dtcRequest = UDS::UDSClient::createRequest(UDS::ServiceID::ReadDTCInformation, {0x02}); // Report DTCs
        auto dtcResponse = doipClient.sendUDSRequest(0x0E80, 0x0EE0, dtcRequest.service, dtcRequest.data);
        UDS::UDSMessage udsDtcResponse(static_cast<UDS::ServiceID>(dtcResponse.payload[4]),
                                       std::vector<uint8_t>(dtcResponse.payload.begin() + 5, dtcResponse.payload.end()));
        std::cout << "DTC Response:\n" << udsClient.interpretResponse(udsDtcResponse) << std::endl;
        printSeparator();

        // Perform a routine control (example: check programming preconditions)
        std::cout << "Performing Routine Control: Check Programming Preconditions..." << std::endl;
        auto routineRequest = udsClient.routineControl(0x01, 0xFF00, {}); // Start routine
        auto routineResponse = doipClient.sendUDSRequest(0x0E80, 0x0EE0, routineRequest.service, routineRequest.data);
        UDS::UDSMessage udsRoutineResponse(static_cast<UDS::ServiceID>(routineResponse.payload[4]),
                                           std::vector<uint8_t>(routineResponse.payload.begin() + 5, routineResponse.payload.end()));
        std::cout << "Routine Control Response:\n" << udsClient.interpretResponse(udsRoutineResponse) << std::endl;
        printSeparator();

        // Simulate periodic tester present messages
        std::cout << "Simulating periodic Tester Present messages for 10 seconds..." << std::endl;
        for (int i = 0; i < 5; ++i) {
            auto testerPresentRequest = UDS::UDSClient::createRequest(UDS::ServiceID::TesterPresent, {0x00});
            auto testerPresentResponse = doipClient.sendUDSRequest(0x0E80, 0x0EE0, testerPresentRequest.service, testerPresentRequest.data);
            UDS::UDSMessage udsTesterPresentResponse(static_cast<UDS::ServiceID>(testerPresentResponse.payload[4]),
                                                     std::vector<uint8_t>(testerPresentResponse.payload.begin() + 5, testerPresentResponse.payload.end()));
            std::cout << "Tester Present sent. Response: " << udsClient.interpretResponse(udsTesterPresentResponse) << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        printSeparator();

        // Disconnect from the DoIP server
        std::cout << "Disconnecting from DoIP server..." << std::endl;
        doipClient.disconnect();
        std::cout << "Disconnected successfully." << std::endl;

    } catch (const DoIP::ConnectionError& e) {
        std::cerr << "DoIP Connection error: " << e.what() << std::endl;
    } catch (const UDS::UDSError& e) {
        std::cerr << "UDS error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
