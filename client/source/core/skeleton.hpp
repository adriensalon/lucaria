#pragma once

#include <future>

struct skeleton_ref {
private:
    skeleton_ref() = default;
public:
    skeleton_ref(const skeleton_ref& other) = delete;
    skeleton_ref& operator=(const skeleton_ref& other) = delete;
    skeleton_ref(skeleton_ref&& other);
    skeleton_ref& operator=(skeleton_ref&& other);
    ~skeleton_ref();


private:
    

    friend skeleton_ref load_skeleton(const std::filesystem::path& file);
    friend std::future<skeleton_ref> fetch_skeleton(const std::filesystem::path& file);
};

/// @brief 
/// @param file 
/// @return 
skeleton_ref load_skeleton(const std::filesystem::path& file);

/// @brief 
/// @param file 
/// @return 
std::future<skeleton_ref> fetch_skeleton(const std::filesystem::path& file);