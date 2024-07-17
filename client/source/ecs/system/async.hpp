#pragma once


// template <typename value_t>
// struct async_container {

//     void emplace(const std::shared_future<std::shared_ptr<value_t>>& future);
//     bool is_waiting() const;
//     bool is_instanced() const;
//     value_t& get();
//     value_t* get_ptr();

// };

struct async_system {
    async_system() = delete;
    async_system(const async_system& other) = delete;
    async_system& operator=(const async_system& other) = delete;
    async_system(async_system&& other) = delete;
    async_system& operator=(async_system&& other) = delete;

    static void update();



    // static void 

};