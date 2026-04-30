# qfsm
A very simple and small finite state machine c/cpp librery, easy to use

## Interface Documentation

### C Version Usage Template

The following is a usage template for the C language interface based on `c/qfsm.h` and `c/qfsm.c`. This example demonstrates how to define states, initialize the FSM, add states, set hooks, and execute the state machine.

```c
#include "qfsm.h"
#include <stdio.h>

// Define state callback functions
QFsmResp state1_callback(void *param) {
    printf("State 1 executing\n");
    // Assume some condition is met to return QFSM_NEXT, otherwise QFSM_KEEP
    return QFSM_NEXT;  // or QFSM_KEEP
}

QFsmResp state2_callback(void *param) {
    printf("State 2 executing\n");
    return QFSM_KEEP;  // Keep current state
}

void state1_in_hook(void *param) {
    printf("Entering State 1\n");
}

void state1_out_hook(void *param) {
    printf("Exiting State 1\n");
}

void state2_in_hook(void *param) {
    printf("Entering State 2\n");
}

void global_hook(void *param) {
    printf("Global state change hook\n");
}

int main() {
    // Define FSM object
    QFsm fsm;
    
    // Define state objects
    QFsmState state1, state2;
    
    // Initialize FSM
    if (qfsm_init(&fsm) != 0) {
        printf("FSM init failed\n");
        return -1;
    }
    
    // Add states
    if (qfsm_add(&fsm, &state1, "state1", "state2", state1_callback, state1_in_hook, state1_out_hook) != 0) {
        printf("Add state1 failed\n");
        return -1;
    }
    
    if (qfsm_add(&fsm, &state2, "state2", "state1", state2_callback, state2_in_hook, NULL) != 0) {
        printf("Add state2 failed\n");
        return -1;
    }
    
    // Set global hook (optional)
    qfsm_hook_set(&fsm, global_hook);
    
    // Set initial state and parameter
    void *param = NULL;  // Can pass custom parameters
    if (qfsm_setup(&fsm, "state1", param) != 0) {
        printf("FSM setup failed\n");
        return -1;
    }
    
    // Execute FSM (call in a loop)
    for (int i = 0; i < 10; i++) {  // Example: execute 10 times
        if (qfsm_exec(&fsm) != 0) {
            printf("FSM exec failed\n");
            break;
        }
    }
    
    // Optional: Skip to a specific state
    qfsm_skip(&fsm, "state2");
    
    // Optional: Delete a state
    qfsm_del(&fsm, &state1);
    
    // Optional: Dump state information
    qfsm_dump(&fsm, printf);
    
    return 0;
}
```

### C++ Version Usage Template

The following is a usage template for the C++ language interface based on `cpp/qfsm.h` and `cpp/qfsm.cpp`. This example demonstrates how to create an FSM object, add states, set hooks, and execute the state machine.

```cpp
#include "qfsm.h"
#include <iostream>
#include <thread>
#include <chrono>

// Define state callback functions
QFsm::Status state1_callback(void *param) {
    std::cout << "State 1 executing" << std::endl;
    // Assume some condition is met to return QFsm::NEXT, otherwise QFsm::KEEP
    return QFsm::NEXT;  // or QFsm::KEEP
}

QFsm::Status state2_callback(void *param) {
    std::cout << "State 2 executing" << std::endl;
    return QFsm::KEEP;  // Keep current state
}

void state1_in_hook(void *param) {
    std::cout << "Entering State 1" << std::endl;
}

void state1_out_hook(void *param) {
    std::cout << "Exiting State 1" << std::endl;
}

void state2_in_hook(void *param) {
    std::cout << "Entering State 2" << std::endl;
}

void global_switch_hook(void *param) {
    std::cout << "Global state switch hook" << std::endl;
}

void global_keep_hook(void *param) {
    std::cout << "Global state keep hook" << std::endl;
}

void global_error_hook(void *param) {
    std::cout << "Global state error hook" << std::endl;
}

int main() {
    // Create FSM object
    QFsm fsm;
    
    // Add states
    try {
        fsm.add("state1", "state2", state1_callback, state1_in_hook, state1_out_hook, nullptr);
        fsm.add("state2", "state1", state2_callback, state2_in_hook, nullptr, nullptr);
    } catch (const std::exception &e) {
        std::cerr << "Add state failed: " << e.what() << std::endl;
        return -1;
    }
    
    // Set hooks (optional)
    fsm.state_switch_hook_set(global_switch_hook);
    fsm.state_keep_hook_set(global_keep_hook);
    fsm.state_error_hook_set(global_error_hook);
    
    // Start FSM (using internal scheduling, automatic execution)
    try {
        fsm.start("state1", true, 100);  // Initial state "state1", internal scheduling, period 100ms
    } catch (const std::exception &e) {
        std::cerr << "Start FSM failed: " << e.what() << std::endl;
        return -1;
    }
    
    // If not using internal scheduling, execute manually
    // for (int i = 0; i < 10; i++) {
    //     fsm.exec();
    //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // }
    
    // Wait for some time to let FSM run
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // Optional: Skip to a specific state
    try {
        fsm.skip("state2");
    } catch (const std::exception &e) {
        std::cerr << "Skip to state failed: " << e.what() << std::endl;
    }
    
    // Optional: Delete a state
    try {
        fsm.del("state1");
    } catch (const std::exception &e) {
        std::cerr << "Delete state failed: " << e.what() << std::endl;
    }
    
    // Optional: Get current state
    auto current_state = fsm.curr();
    std::cout << "Current state: " << current_state.name << std::endl;
    
    // Optional: Get all states
    auto states = fsm.states();
    for (const auto &pair : states) {
        std::cout << "State: " << pair.first << " -> " << pair.second.next << std::endl;
    }
    
    return 0;
}
```
