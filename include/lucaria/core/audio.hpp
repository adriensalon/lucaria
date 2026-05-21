#pragma once

#include <lucaria/bin/audio_data.hpp>
#include <lucaria/core/workaround.hpp>
#include <lucaria/core/resource.hpp>
#include <lucaria/core/refcount.hpp>

namespace lucaria {
namespace detail {

    struct audio_implementation {
        LUCARIA_DELETE_DEFAULT(audio_implementation)
        audio_implementation(const audio_implementation& other) = delete;
        audio_implementation& operator=(const audio_implementation& other) = delete;
        audio_implementation(audio_implementation&& other) = default;
        audio_implementation& operator=(audio_implementation&& other) = default;

        audio_implementation(const std::vector<char>& bytes);
        audio_implementation(audio_data&& data);

        audio_data data;
    };

}

/// @brief
struct audio_object {
    audio_object() = default;
    audio_object(const audio_object& other) = default;
    audio_object& operator=(const audio_object& other) = default;
    audio_object(audio_object&& other) = default;
    audio_object& operator=(audio_object&& other) = default;
	~audio_object();

    static audio_object fetch(const std::filesystem::path& path);

    [[nodiscard]] bool has_value() const;

    [[nodiscard]] explicit operator bool() const;

	[[nodiscard]] audio_data* data();

	[[nodiscard]] const audio_data* data() const;

private:
	detail::refcount_flag _refcount = {};
    detail::resource_manager<detail::audio_implementation>* _manager = nullptr;
    detail::resource_container<detail::audio_implementation>* _resource = nullptr;
    explicit audio_object(detail::resource_container<detail::audio_implementation>* resource);
};

}
