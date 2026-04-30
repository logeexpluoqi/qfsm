/**
 * @ Author: luoqi
 * @ Create Time: 2025-05-19 14:33
 * @ Modified by: luoqi
 * @ Modified time: 2025-06-10 16:16
 * @ Description:
 */

#pragma once

#include <thread>
#include <string>
#include <unordered_map>
#include <functional>
#include <stack>

/**
 * @brief Finite State Machine (FSM) class implementation.
 * This class provides a simple and efficient way to manage state transitions in a program.
 * It supports adding, deleting, and executing states with custom handlers and hooks.
 */
class QFsm {
public:
    /**
     * @brief Enumeration of possible FSM statuses.
     * - ERR: Error occurred.
     * - KEEP: Stay in the current state.
     * - NEXT: Transition to the next state.
     * - STOP: Stop the FSM.
     */
    enum Status {
        ERR = -1, // Error
        KEEP,     // Keep current state
        NEXT,     // Go to next state
        STOP,     // Stop FSM
    };

    /**
     * @brief Type definition for state handler function.
     * @param data Pointer to user-defined data.
     * @return Status indicating the next action (ERR, KEEP, NEXT, STOP).
     */
    using Callback = std::function<Status(void *)>;

    /**
     * @brief Type definition for state hook function.
     * @param data Pointer to user-defined data.
     * @return Integer indicating success (0) or failure (-1).
     */
    using Hook = std::function<void(void *)>;

private:
    /**
     * @brief Structure representing a state in the FSM.
     */
    struct State {
        std::string name; // State name
        std::string next; // Next state name
        Callback cb;      // State callback function
        Hook in;          // State entry hook
        Hook out;         // State exit hook
        void *data;       // User-defined data
    };

    State curr_state_;                                  // Current state
    std::unordered_map<std::string, State> states_map_; // Map of all states

    std::stack<std::string> switch_stack_; // Stack of states to visit
    Hook state_switch_hook_{ nullptr };    // Hook called when state changes
    Hook state_keep_hook_{ nullptr };      // Hook called when state stays the same
    Hook state_error_hook_{ nullptr };     // Hook called when an error occurs

    /**
     * @brief Start automatic execution of the FSM.
     * The FSM will execute at the specified period (default: 1ms).
     */
    void exec_();

    std::thread thrd_;                   // Thread for automatic execution
    bool stopped_{ false }; // Flag indicating whether the FSM is running
    size_t period_;                      // Period for automatic execution in milliseconds

public:
    QFsm() = default;

    /**
     * @brief Destructor. Joins the execution thread if it is running.
     */
    ~QFsm();

    /**
     * @brief Start the FSM with the specified initial state.
     * @param first Name of the initial state.
     * @param inner_sched Whether to start the automatic execution thread (default: true).
     * @param period Period in milliseconds for automatic execution.
     * @return 0 on success, -1 on failure (e.g., state not found).
     */
    int start(const std::string &first, bool inner_sched = true, size_t period = 1);

    /**
     * @brief Add a new state to the FSM.
     * @param name Name of the state.
     * @param next Name of the next state.
     * @param cb State callback function.
     * @param data User-defined data.
     * @return 0 on success, -1 on failure (e.g., duplicate state).
     */
    int add(const std::string &name, const std::string &next, Callback cb, Hook in, Hook out, void *data);

    /**
     * @brief Delete a state from the FSM.
     * @param name Name of the state to delete.
     * @return 0 on success, -1 on failure (e.g., state not found).
     */
    int del(const std::string &name);

    /**
     * @brief Transition to the specified state.
     * @param name Name of the state to transition to.
     * @return 0 on success, -1 on failure (e.g., state not found or hook already set).
     * @note This function is used to transition to a new state without executing any hooks.
     * @note This function cannot be called in state in hook or out hook.
     */
    int skip(const std::string &name);

    /**
     * @brief Set the state change hook.
     * @param hook Hook function to be called when the state changes.
     * @return 0 on success, -1 if a hook is already set.
     */
    int state_switch_hook_set(Hook hook);

    /**
     * @brief Set the state keep hook.
     * @param hook Hook function to be called when the state stays the same.
     * @return 0 on success, -1 if a hook is already set.
     */
    int state_keep_hook_set(Hook hook);

    /**
     * @brief Set the state error hook.
     * @param hook Hook function to be called when an error occurs.
     * @return 0 on success, -1 if a hook is already set.
     */
    int state_error_hook_set(Hook hook);

    /**
     * @brief Execute the current state's handler.
     */
    void exec();

    /**
     * @brief Get the current state.
     * @return Current state.
     */
    auto curr() const { return curr_state_; }

    /**
     * @brief Get the map of all states.
     * @return Map of all states.
     */
    auto states() const { return states_map_; }
};
