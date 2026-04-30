/**
 * @ Author: luoqi
 * @ Create Time: 2024-11-02 10:16
 * @ Modified by: Your name
 * @ Modified time: 2025-08-11 22:59:06
 * @ Description:
 */

#include "qfsm.h"
#include <stdint.h>

#define QFSM_ENTRY(ptr)       ((QFsmState *)((char *)(ptr) - ((unsigned long)&((QFsmState *)0)->node)))
#define QFSM_ITER(node, list) for(node = (list)->next; node != (list); node = node->next)
#define QFSM_ITER_SAFE(node, safe, list) \
    for(node = (list)->next, safe = node->next; node != (list); node = safe, safe = node->next)

/**
 * @brief 32-bit MurmurHash3 hash algorithm
 *
 * MurmurHash3 is a non-cryptographic hash function with excellent distribution
 * and collision resistance properties. It is designed for high performance
 * and is widely used in hash table implementations.
 *
 * @param name Input string to hash
 * @return 32-bit hash value
 */
static uint32_t id_calc_(const char *name)
{
    if(!name) {
        return 0;
    }

    uint32_t hash = 0x811c9dc5; /* Initial seed */
    uint32_t prime = 0x1000193; /* 16777619 */
    const uint8_t *data = (const uint8_t *)name;

    while(*data) {
        hash ^= *data++;
        hash *= prime;
    }

    return hash;
}

static inline void list_insert_after_(QFsmList *list, QFsmList *node)
{
    if(!list || !node) {
        return;
    }
    list->next->prev = node;
    node->next = list->next;

    list->next = node;
    node->prev = list;
}

static inline void list_remove_(QFsmList *node)
{
    if(!node) {
        return;
    }
    node->next->prev = node->prev;
    node->prev->next = node->next;

    node->next = node->prev = node;
}

static inline int state_isexist_(QFsm *fsm, QFsmState *state)
{
    if(!fsm || !state) {
        return 0;
    }
    QFsmState *state_;
    QFsmList *node_;

    QFSM_ITER(node_, &fsm->list)
    {
        state_ = QFSM_ENTRY(node_);
        if(state_->id == state->id) {
            return 1;
        }
    }
    return 0;
}

int qfsm_init(QFsm *fsm)
{
    if(!fsm) {
        return -1;
    }
    fsm->current = NULL;
    fsm->hook = NULL;
    fsm->param = NULL;
    fsm->list.prev = &fsm->list;
    fsm->list.next = &fsm->list;
    for(int i = 0; i < QFSM_SKIP_STACK_MAX; i++) {
        fsm->skip_stack[i] = NULL;
    }
    return 0;
}

int qfsm_setup(QFsm *fsm, const char *start_name, void *param)
{
    if(!fsm) {
        return -1;
    }
    fsm->param = param;
    uint32_t id = id_calc_(start_name);

    QFsmState *state_;
    QFsmList *node, *safe;

    int found = 0;
    QFSM_ITER_SAFE(node, safe, &fsm->list)
    {
        state_ = QFSM_ENTRY(node);
        if(state_->id == id) {
            fsm->current = state_;
            found = 1;
            break;
        }
    }
    if(!found) {
        fsm->param = NULL;
        return -1;
    }

    QFSM_ITER_SAFE(node, safe, &fsm->list)
    {
        state_ = QFSM_ENTRY(node);
        if(state_->next) {
            continue;
        }
        QFsmList *inner_node;
        QFSM_ITER(inner_node, &fsm->list)
        {
            QFsmState *candidate = QFSM_ENTRY(inner_node);
            if(state_->next_id == candidate->id) {
                state_->next = candidate;
                break;
            }
        }
    }
    return 0;
}

int qfsm_exec(QFsm *fsm)
{
    if(!fsm || !fsm->current) {
        return -1;
    }

    if(fsm->skip_stack_top > 0) {
        QFsmState *to_ = fsm->skip_stack[fsm->skip_stack_top - 1];
        if(to_->id != fsm->current->id) {
            if(fsm->hook) {
                fsm->hook(fsm->param);
            }
            if(fsm->current->out) {
                fsm->current->out(fsm->param);
            }

            if(to_->in) {
                to_->in(fsm->param);
            }

            fsm->current = to_;
            fsm->skip_stack[fsm->skip_stack_top - 1] = NULL;
            fsm->skip_stack_top--;
        }
    }

    QFsmState *current = fsm->current;
    int ret = current->cb(fsm->param);
    if(ret == QFSM_NEXT) {
        if(fsm->hook) {
            fsm->hook(fsm->param);
        }
        if(current->out) {
            current->out(fsm->param);
            if(fsm->current != current) {
                return 0;
            }
        }
        if(!current->next) {
            return -1;
        }
        QFsmState *next = current->next;
        if(next->in) {
            next->in(fsm->param);
        }
        fsm->current = next;
    }
    return 0;
}

int qfsm_hook_set(QFsm *fsm, QFsmHook hook)
{
    if(!fsm || !hook) {
        return -1;
    } else {
        fsm->hook = hook;
    }
    return 0;
}

int qfsm_add(QFsm *fsm,
             QFsmState *state,
             const char *name,
             const char *next_name,
             QFsmCallback cb,
             QFsmHook in,
             QFsmHook out)
{
    if(!fsm || !state || !cb) {
        return -1;
    }
    state->name = name;
    state->cb = cb;
    state->in = in;
    state->out = out;
    state->id = id_calc_(name);
    state->next_id = id_calc_(next_name);
    if(state_isexist_(fsm, state)) {
        return -1;
    } else {
        list_insert_after_(&fsm->list, &state->node);
    }
    return 0;
}

int qfsm_del(QFsm *fsm, QFsmState *state)
{
    if(!fsm || !state) {
        return -1;
    } else if(state_isexist_(fsm, state)) {
        list_remove_(&state->node);
    } else {
        return -1;
    }
    return 0;
}

int qfsm_skip(QFsm *fsm, const char *state_name)
{
    if(!fsm || !state_name || !fsm->current) {
        return -1;
    }
    if(fsm->skip_stack_top >= QFSM_SKIP_STACK_MAX) {
        return -1;
    }

    uint32_t id = id_calc_(state_name);
    QFsmList *node_;
    QFSM_ITER(node_, &fsm->list)
    {
        QFsmState *to_ = QFSM_ENTRY(node_);
        if((to_->id == id) && (fsm->current->id != id)) {
            fsm->skip_stack[fsm->skip_stack_top++] = to_;
            break;
        }
    }
    return 0;
}

int qfsm_dump(QFsm *fsm, int (*dump)(const char *fmt, ...))
{
    if(!fsm || !dump) {
        return -1;
    }
    dump(" STATES:\r\n");
    size_t count = 0;
    QFsmList *node_;
    QFSM_ITER(node_, &fsm->list)
    {
        count++;
        QFsmState *state_ = QFSM_ENTRY(node_);
        const char *next_name = (state_->next) ? state_->next->name : "NULL";
        dump("  {%s} -> {%s}\r\n", state_->name, next_name);
    }
    dump(" -CUR: {%s}\r\n", fsm->current ? fsm->current->name : "NULL");
    dump(" -NUM: %u\r\n", count);
    return 0;
}
