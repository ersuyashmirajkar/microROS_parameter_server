# micro-ROS Parameter Server Example for ESP32 with Arduino IDE

This repository contains an example of a micro-ROS parameter server implemented on an ESP32 using the Arduino IDE. The code demonstrates how to create and manage parameters, connect to a micro-ROS agent, and handle dynamic parameter updates.

## Features

- Initializes a micro-ROS parameter server with configurable parameters.
- Connects to a micro-ROS agent with reconnection support.
- Supports three parameters: `bool_param`, `int_param`, and `double_param`.
- Dynamic parameter updates through a micro-ROS agent.
- LED indicator for connection status.

## Prerequisites

1. Install the Arduino IDE.
2. Install the **micro-ROS Arduino** library from the Arduino Library Manager.
3. Configure your ESP32 board in the Arduino IDE.
4. Set up a micro-ROS agent on your host machine.

## Wiring

The onboard LED of the ESP32 (GPIO 2) is used as a status indicator:

- **ON**: Connected to the micro-ROS agent.
- **OFF**: Disconnected from the micro-ROS agent.

## Setup Instructions

1. Clone this repository or copy the code into the Arduino IDE.
2. Upload the sketch to your ESP32 board.
3. Run the micro-ROS agent on your host machine:
   ```bash
   ros2 run micro_ros_agent micro_ros_agent serial --dev [device_port]
   ```
   Replace `[device_port]` with the serial port connected to your ESP32.

## Parameter Interaction

The parameter server provides three parameters:

- `bool_param`: Boolean type (default: `false`)
- `int_param`: Integer type (default: `10`)
- `double_param`: Double type (default: `0.01`)

### Setting and Getting Parameters

You can interact with the parameters using ROS 2 commands:

1. **List Parameters:**

   ```bash
   ros2 param list /param_server_test_node
   ```

2. **Get Parameter Value:**

   ```bash
   ros2 param get /param_server_test_node bool_param
   ros2 param get /param_server_test_node int_param
   ros2 param get /param_server_test_node double_param
   ```

3. **Set Parameter Value:**

   ```bash
   ros2 param set /param_server_test_node bool_param true
   ros2 param set /param_server_test_node int_param 20
   ros2 param set /param_server_test_node double_param 3.14
   ```

4. **Describe Parameter:**

   ```bash
   ros2 param describe /param_server_test_node bool_param
   ```

### Example Commands

To set the integer parameter:

```bash
ros2 param set /param_server_test_node int_param 42
```

To get the updated value:

```bash
ros2 param get /param_server_test_node int_param
```

## Code Overview

The main functionality is implemented in the `loop()` function, which handles the state of the connection with the micro-ROS agent. The parameter server is initialized and managed through the `rclc_parameter_server` APIs.

### States

1. `WAITING_AGENT`: Waiting for the agent to be available.
2. `AGENT_AVAILABLE`: Agent is available and entities are being created.
3. `AGENT_CONNECTED`: Connected to the agent; executor handles tasks.
4. `AGENT_DISCONNECTED`: Agent connection lost; entities are destroyed.

## LED Behavior

The onboard LED reflects the connection state:

- **ON**: Successfully connected to the micro-ROS agent.
- **OFF**: Disconnected or waiting for the agent.

## Troubleshooting

1. **Connection Issues:** Ensure the micro-ROS agent is running and the transport settings match.
2. **Serial Port Not Found:** Verify the ESP32 is correctly connected and identified by your host machine.
3. **Parameter Errors:** Use `ros2 param list` to ensure the parameter names match your commands.

## License

This project is licensed under the MIT License. Feel free to use and modify it.

