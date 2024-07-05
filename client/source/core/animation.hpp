#pragma once

#include <filesystem>
#include <future>

struct animation_ref {
private:
    animation_ref() = default;
public:
    animation_ref(const animation_ref& other) = delete;
    animation_ref& operator=(const animation_ref& other) = delete;
    animation_ref(animation_ref&& other);
    animation_ref& operator=(animation_ref&& other);
    ~animation_ref();


private:
    
    bool _must_destroy;
    friend animation_ref load_animation(const std::filesystem::path& file);
    friend std::future<animation_ref> fetch_animation(const std::filesystem::path& file);
};

/// @brief 
/// @param file 
/// @return 
animation_ref load_animation(const std::filesystem::path& file);

/// @brief 
/// @param file 
/// @return 
std::future<animation_ref> fetch_animation(const std::filesystem::path& file);