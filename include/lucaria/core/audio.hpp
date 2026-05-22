#pragma once

#include <cereal/types/variant.hpp>

#include <lucaria/bin/audio_data.hpp>
#include <lucaria/core/refcount.hpp>
#include <lucaria/core/resource.hpp>
#include <lucaria/core/workaround.hpp>

namespace lucaria {
namespace detail {

    struct sound_track_implementation;

    enum struct audio_origin {
        path,
        data
    };

    struct audio_implementation {
        LUCARIA_DELETE_DEFAULT(audio_implementation)
        audio_implementation(const audio_implementation& other) = delete;
        audio_implementation& operator=(const audio_implementation& other) = delete;
        audio_implementation(audio_implementation&& other) = default;
        audio_implementation& operator=(audio_implementation&& other) = default;

        audio_implementation(const std::vector<char>& bytes);
        audio_implementation(audio_data&& data);
        audio_implementation(const sound_track_implementation& sound_track); // NOT IMPLEMENTED

        audio_origin origin;
        audio_data data;
    };

    struct audio_path_recipe {
        std::filesystem::path path;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

    struct audio_data_recipe {
        audio_data data;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("data", data));
        }
    };

    using audio_recipe = std::variant<audio_path_recipe, audio_data_recipe>;

	[[nodiscard]] audio_recipe make_recipe(const implementation_container<audio_implementation>& container);
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
    detail::implementation_manager<detail::audio_implementation>* _manager = nullptr;
    detail::implementation_container<detail::audio_implementation>* _resource = nullptr;
    
	template <typename ArchiveType>
    void save(ArchiveType& archive) const;
    template <typename ArchiveType>
    void load(ArchiveType& archive);

	friend class cereal::access;
};

}
