#pragma once

struct scripting_system {
    scripting_system() = delete;
    scripting_system(const scripting_system& other) = delete;
    scripting_system& operator=(const scripting_system& other) = delete;
    scripting_system(scripting_system&& other) = delete;
    scripting_system& operator=(scripting_system&& other) = delete;

    template <typename state_t>
    static void use_controller_state();
    
    static void resolve_controller_states();

};