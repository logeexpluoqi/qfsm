/**
 * @ Author: luoqi
 * @ Create Time: 2024-11-02 10:16
 * @ Modified by: Your name
 * @ Modified time: 2025-08-15 21:20:55
 * @ Description:
 */

#ifndef _QFSM_H
#define _QFSM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef QFSM_SKIP_STACK_MAX
#define QFSM_SKIP_STACK_MAX 3
#endif

/**
 * @brief Doubly linked list node structure for FSM states
 */
typedef struct QFsmList QFsmList;
struct QFsmList {
    struct QFsmList *prev; ///< Pointer to the previous node in the list
    struct QFsmList *next; ///< Pointer to the next node in the list
};

// qfsm state callback response
/**
 * @brief Response codes for FSM state callbacks
 */
typedef enum {
    QFSM_NEXT, ///< Indicates that the FSM should transition to the next state
    QFSM_KEEP, ///< Indicates that the FSM should remain in the current state
} QFsmResp;

/**
 * @brief Function pointer type for FSM hook functions
 * @param param Pointer to callback parameter data
 */
typedef void (*QFsmHook)(void *param);

/**
 * @brief Function pointer type for FSM state callback functions
 * @param param Pointer to callback parameter data
 * @return QFsmResp Response indicating whether to transition to next state or stay
 */
typedef QFsmResp (*QFsmCallback)(void *param);

/**
 * @brief Structure representing a state in the FSM
 */
typedef struct QFsmState QFsmState;
struct QFsmState {
    const char *name; ///< Name of the state
    uint32_t id;      ///< Unique identifier for the state
    QFsmHook in;      ///< Callback function called when entering the state
    QFsmCallback cb;  ///< Main callback function for the state logic
    QFsmHook out;     ///< Callback function called when exiting the state
    uint32_t next_id; ///< ID of the next state to transition to
    QFsmState *next;  ///< Pointer to the next state object
    QFsmList node;    ///< Node for linking states in a doubly linked list
};

/**
 * @brief Structure representing the Finite State Machine
 */
typedef struct {
    void *param;                         ///< Parameter passed to callback functions
    QFsmState *current;                  ///< Pointer to the current active state
    QFsmHook hook;                       ///< Global hook function called on state changes
    QFsmState *skip_stack[QFSM_SKIP_STACK_MAX]; ///< List of states to skip
    size_t skip_stack_top;               ///< Top of the skip stack
    QFsmList list;                       ///< List of all states in the FSM
} QFsm;

/**
 * @brief Macro to create and initialize an FSM object
 * @param fsm Name of the FSM variable to create
 */
#define QFSM_CREATE(fsm) \
    QFsm fsm = { .param = NULL, .current = NULL, .hook = NULL, ._list_ = { .prev = &fsm._list_, .next = &fsm._list_ } }

/**
 * @brief Initialize the FSM object
 * @param fsm Pointer to the FSM object
 * @return 0 on success, negative value on error
 */
int qfsm_init(QFsm *fsm);

/**
 * @brief Set up the FSM with initial state and parameter
 * @param fsm Pointer to the FSM object
 * @param start_name Name of the initial state
 * @param param Pointer to parameter data for callbacks
 * @return 0 on success, negative value on error
 */
int qfsm_setup(QFsm *fsm, const char *start_name, void *param);

/**
 * @brief Set a global hook function that is called on state changes
 * @param fsm Pointer to the FSM object
 * @param hook Hook function to be called on state transitions
 * @return 0 on success, negative value on error
 */
int qfsm_hook_set(QFsm *fsm, QFsmHook hook);

/**
 * @brief Execute the current state's callback function
 * @param fsm Pointer to the FSM object
 * @return 0 on success, negative value on error
 */
int qfsm_exec(QFsm *fsm);

/**
 * @brief Add a new state to the FSM
 * @param fsm Pointer to the FSM object
 * @param state Pointer to the state object to add
 * @param name Name of the new state
 * @param next_name Name of the next state to transition to
 * @param cb Callback function for the state logic
 * @param in Callback function called when entering the state
 * @param out Callback function called when exiting the state
 * @return 0 on success, negative value on error
 */
int qfsm_add(QFsm *fsm,
             QFsmState *state,
             const char *name,
             const char *next_name,
             QFsmCallback cb,
             QFsmHook in,
             QFsmHook out);

/**
 * @brief Remove a state from the FSM
 * @param fsm Pointer to the FSM object
 * @param state Pointer to the state object to remove
 * @return 0 on success, negative value on error
 */
int qfsm_del(QFsm *fsm, QFsmState *state);

/**
 * @brief Skip to a specific state by name
 * @param fsm Pointer to the FSM object
 * @param state_name Name of the state to transition to
 * @return 0 on success, negative value on error
 * @note This function can be called from any state callback to transition to a different state.
 * @note This function cannot be called in state in hook or out hook
 */
int qfsm_skip(QFsm *fsm, const char *state_name);

/**
 * @brief Dump the FSM state information
 * @param fsm Pointer to the FSM object
 * @param dump Function pointer to the dump function
 * @return 0 on success, negative value on error
 */
int qfsm_dump(QFsm *fsm, int (*dump)(const char *fmt, ...));

#endif
