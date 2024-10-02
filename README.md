# DoIP and UDS Client Project

This project implements a Diagnostics over IP (DoIP) client with Unified Diagnostic Services (UDS) support. It provides a flexible and extensible framework for automotive diagnostic communications.

## Features

- DoIP client implementation
- UDS message creation and interpretation
- Support for various UDS services (e.g., ReadDataByIdentifier, RoutineControl)
- Customizable UDS response handlers
- Example diagnostic session implementation

## Prerequisites

- C++17 compatible compiler (e.g., GCC 7+ or Clang 5+)
- Make build system
- POSIX-compliant operating system (Linux, macOS, etc.)

## Project Structure

```
.
├── include/
│   ├── doip_client.h
│   ├── doip_message.h
│   └── uds_client.h
├── src/
│   ├── doip_client.cpp
│   ├── doip_message.cpp
│   ├── uds_client.cpp
│   └── main.cpp
├── Makefile
└── README.md
```

## Building the Project

1. Clone the repository:
   ```
   git clone https://github.com/yourusername/doip-uds-client.git
   cd doip-uds-client
   ```

2. Build the project using Make:
   ```
   make
   ```

   This will compile the source files and create an executable named `diagnostic_tool` in the `bin/` directory.

3. (Optional) To build with debug symbols:
   ```
   make debug
   ```

## Running the Diagnostic Tool

After building, you can run the diagnostic tool:

```
./bin/diagnostic_tool
```

Note: Make sure to update the IP address and port in `main.cpp` to match your DoIP server configuration before building and running.

## Usage Example

Here's a basic example of how to use the DoIP and UDS clients in your code:

```cpp
#include "doip_client.h"
#include "uds_client.h"

int main() {
    // Initialize clients
    DoIP::DoIPClient doipClient("192.168.1.10", 13400);
    UDS::UDSClient udsClient;

    // Connect to DoIP server
    doipClient.connect();

    // Perform a UDS request (e.g., Read VIN)
    auto vinRequest = udsClient.readDataByIdentifier(0xF190);
    auto vinResponse = doipClient.sendUDSRequest(0x0E80, 0x0EE0, vinRequest.service, vinRequest.data);

    // Interpret the response
    UDS::UDSMessage udsResponse(static_cast<UDS::ServiceID>(vinResponse.payload[4]),
                                std::vector<uint8_t>(vinResponse.payload.begin() + 5, vinResponse.payload.end()));
    std::cout << udsClient.interpretResponse(udsResponse) << std::endl;

    // Disconnect
    doipClient.disconnect();

    return 0;
}
```

## Customizing UDS Response Handlers

You can add custom handlers for specific UDS services:

```cpp
udsClient.addServiceHandler(UDS::ServiceID::ReadDataByIdentifier,
    [](const std::vector<uint8_t>& data) {
        // Custom interpretation logic here
        return "Custom interpretation of ReadDataByIdentifier response";
    });
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Thanks to all contributors who have helped with this project.
- Special thanks to the automotive diagnostic community for their valuable resources and standards.
