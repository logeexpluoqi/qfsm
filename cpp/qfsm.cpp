/**
 * @ Author: luoqi
 * @ Create Time: 2025-05-19 14:33
 * @ Modified by: luoqi
 * @ Modified time: 2025-05-19 15:50
 * @ Description:
 */

#include <stdexcept>
#include "qfsm.h"

int QFsm::start(const std::string &first, bool inner_sched, size_t period)
{
    auto it = states_map_.find(first);
    if(it != states_map_.end()) {
        curr_state_ = states_map_.at(first);
    } else {
        throw std::invalid_argument("State not found");
    }
    stopped_ = false;
    period_ = period;
    if(inner_sched) {
        thrd_ = std::thread(&QFsm::exec_, this);
    }
    return 0;
}

QFsm::~QFsm()
{
    stopped_ = true;
    if(thrd_.joinable()) {
        thrd_.join();
    }
}

int QFsm::add(const std::string &name, const std::string &next, Callback cb, Hook in, Hook out, void *data)
{
    if(states_map_.find(name) != states_map_.end()) {
        throw std::invalid_argument("State already exists");
    }
    states_map_.insert(std::make_pair(name, State{ name, next, cb, in, out, data }));
    return 0;
}

int QFsm::del(const std::string &name)
{
    auto it = states_map_.find(name);
    if(it != states_map_.end()) {
        states_map_.erase(it);
    } else {
        throw std::invalid_argument("State not found");
    }
    return 0;
}

int QFsm::skip(const std::string &name)
{
    if(states_map_.find(name) != states_map_.end()) {
        switch_stack_.push(name);
    } else {
        throw std::invalid_argument("State not found");
    }
    return 0;
}

void QFsm::exec()
{
    if(!switch_stack_.empty()) {
        auto sw = states_map_.at(switch_stack_.top());
        if(sw.name != curr_state_.name) {
            if(state_switch_hook_ != nullptr) {
                state_switch_hook_(curr_state_.data);
            }
            if(curr_state_.out != nullptr) {
                curr_state_.out(curr_state_.data);
            }
            if(sw.in != nullptr) {
                sw.in(curr_state_.data);
            }
            curr_state_ = sw;
        }
        switch_stack_.pop();
    }

    if(curr_state_.cb == nullptr) {
        throw std::runtime_error("State callback is null");
    }
    Status next = curr_state_.cb(curr_state_.data);
    if(next == KEEP) {
        if(state_keep_hook_ != nullptr) {
            state_keep_hook_(curr_state_.data);
        }
    } else if(next == NEXT) {
        if(state_switch_hook_ != nullptr) {
            state_switch_hook_(curr_state_.data);
        }
        if(curr_state_.out != nullptr) {
            curr_state_.out(curr_state_.data);
        }
        auto it = states_map_.find(curr_state_.next);
        if(it == states_map_.end()) {
            if(state_error_hook_ != nullptr) {
                state_error_hook_(curr_state_.data);
            }
            return;
        }
        curr_state_ = it->second;
        if(curr_state_.in != nullptr) {
            curr_state_.in(curr_state_.data);
        }
    } else if(next == STOP) {
        stopped_ = true;
    } else {
        throw std::runtime_error("Invalid status");
    }
}

void QFsm::exec_()
{
    while(!stopped_) {
        exec();
        std::this_thread::sleep_for(std::chrono::milliseconds(period_));
    }
}

int QFsm::state_switch_hook_set(Hook hook)
{
    state_switch_hook_ = hook;
    return 0;
}

int QFsm::state_error_hook_set(Hook hook)
{
    state_error_hook_ = hook;
    return 0;
}

int QFsm::state_keep_hook_set(Hook hook)
{
    state_keep_hook_ = hook;
    return 0;
}
