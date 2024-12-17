#include <micro_ros_arduino.h>

#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microros/rmw_microros.h>

#include <rclc_parameter/rclc_parameter.h>

#define LED_PIN 2 // Define the pin for the onboard LED

// Macro for error handling
#define RCCHECK(fn) \
  { \
    rcl_ret_t temp_rc = fn; \
    if ((temp_rc != RCL_RET_OK)) { error_loop(); } \
  }

// Macro to execute a task every N milliseconds
#define EXECUTE_EVERY_N_MS(MS, X) \
  do { \
    static volatile int64_t init = -1; \
    if (init == -1) { init = uxr_millis(); } \
    if (uxr_millis() - init > MS) { \
      X; \
      init = uxr_millis(); \
    } \
  } while (0)

// Macro for soft error checking
#define RCSOFTCHECK(fn) \
  { \
    rcl_ret_t temp_rc = fn; \
    if ((temp_rc != RCL_RET_OK)) {} \
  }

// Function to handle errors (currently commented out)
void error_loop() {
  // Uncomment this section to toggle the LED in case of an error
  // while(1){
  //   digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  //   delay(100);
  // }
}

// Declare necessary micro-ROS entities
rclc_support_t support;
rcl_node_t node;
rcl_timer_t timer;
rclc_executor_t executor;
rcl_allocator_t allocator;
rclc_parameter_server_t param_server;

// Define states for agent connection management
enum states {
  WAITING_AGENT,       // Waiting for the micro-ROS agent to be available
  AGENT_AVAILABLE,     // Agent is available, ready to connect
  AGENT_CONNECTED,     // Successfully connected to the agent
  AGENT_DISCONNECTED   // Agent connection lost
} state;

// Initialize parameter server options
const rclc_parameter_options_t options = {
    .notify_changed_over_dds = true,          // Notify changes over DDS
    .max_params = 3,                         // Maximum number of parameters
    .allow_undeclared_parameters = true,     // Allow undeclared parameters
    .low_mem_mode = false                    // Disable low memory mode
};

// Function to create and initialize all necessary entities
bool create_entities() {
  allocator = rcl_get_default_allocator();

  // Initialize support structure
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // Initialize node
  RCCHECK(rclc_node_init_default(&node, "param_server_test_node", "", &support));

  // Initialize executor with appropriate handles
  RCCHECK(rclc_executor_init(&executor, &support.context, RCLC_EXECUTOR_PARAMETER_SERVER_HANDLES - 1, &allocator));

  // Initialize parameter server with options
  rclc_parameter_server_init_with_option(&param_server, &node, &options);

  // Add parameter server to executor without callback (optional)
  RCCHECK(rclc_executor_add_parameter_server(&executor, &param_server, NULL));

  // Add parameters to the server
  rclc_add_parameter(&param_server, "bool_param", RCLC_PARAMETER_BOOL);     // Boolean parameter
  rclc_add_parameter(&param_server, "int_param", RCLC_PARAMETER_INT);      // Integer parameter
  rclc_add_parameter(&param_server, "double_param", RCLC_PARAMETER_DOUBLE);   // Double parameter

  // Set default values for the parameters
  rclc_parameter_set_bool(&param_server, "bool_param", false);
  rclc_parameter_set_int(&param_server, "int_param", 10);
  rclc_parameter_set_double(&param_server, "double_param", 0.01);

  return true;
}

// Function to destroy all created entities
void destroy_entities() {
  rmw_context_t* rmw_context = rcl_context_get_rmw_context(&support.context);
  (void)rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0);

  rclc_executor_fini(&executor);
  rcl_node_fini(&node);
  rclc_support_fini(&support);
  rclc_parameter_server_fini(&param_server, &node);
}

// Arduino setup function
void setup() {
  set_microros_transports(); // Initialize micro-ROS transports

  pinMode(LED_PIN, OUTPUT);  // Set the LED pin as output
  digitalWrite(LED_PIN, HIGH); // Turn the LED on

  delay(2000); // Wait for 2 seconds
  state = WAITING_AGENT; // Start in the WAITING_AGENT state
}

// Arduino loop function
void loop() {
  switch (state) {
    case WAITING_AGENT:
      // Check if the agent is available every 500 ms
      EXECUTE_EVERY_N_MS(500, state = (RMW_RET_OK == rmw_uros_ping_agent(100, 1)) ? AGENT_AVAILABLE : WAITING_AGENT;);
      break;
    case AGENT_AVAILABLE:
      // Create entities if the agent is available
      state = (true == create_entities()) ? AGENT_CONNECTED : WAITING_AGENT;
      if (state == WAITING_AGENT) {
        destroy_entities(); // Clean up if creation fails
      };
      break;
    case AGENT_CONNECTED:
      // Check agent connection every 200 ms
      EXECUTE_EVERY_N_MS(200, state = (RMW_RET_OK == rmw_uros_ping_agent(100, 1)) ? AGENT_CONNECTED : AGENT_DISCONNECTED;);
      if (state == AGENT_CONNECTED) {
        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)); // Handle executor tasks
      }
      break;
    case AGENT_DISCONNECTED:
      // Clean up and go back to waiting for the agent
      destroy_entities();
      state = WAITING_AGENT;
      break;
    default:
      break;
  }

  // Update LED state based on connection status
  if (state == AGENT_CONNECTED) {
    digitalWrite(LED_PIN, 1); // Turn the LED on when connected
  } else {
    digitalWrite(LED_PIN, 0); // Turn the LED off when disconnected
  }
}
