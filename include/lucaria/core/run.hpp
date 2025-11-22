#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <imgui.h>

/// @brief Main entry point for a Lucaria application. Must be implemented by
/// the application. Arguments will be forwarded on all platforms that support it.
/// This allows the engine to setup before invoking user code. Despite giving the illusion
/// that this library can be used without further initialization, static initialization
/// of user code happens before the platform and engine could initialize
/// @param argc the standard argc argument
/// @param argv the standard argv argument
/// @return the standard return code
int lucaria_main(int argc = 0, char** argv = nullptr);

namespace lucaria {

/// @brief Runs a Lucaria application from user owned registries
/// @param scenes user owned registries holding the components
/// @param on_update callback to fire on evey frame
void run(std::vector<entt::registry>& scenes, const std::function<void()>& on_update);

/// @brief Gets if the implementation supports ETC2 texture format
/// @return if the feature is supported
[[nodiscard]] bool get_is_etc2_supported();

/// @brief Gets if the implementation supports float audio buffers
/// @return if the feature is supported
[[nodiscard]] bool get_is_alf32_supported();

/// @brief Gets if the implementation supports S3TC texture format
/// @return if the feature is supported
[[nodiscard]] bool get_is_s3tc_supported();

/// @brief Gets the current size of the final framebuffer
/// @return size of the screen
[[nodiscard]] glm::uvec2 get_screen_size();

/// @brief Gets the current time delta from previous frame
/// @return current time delta from previous frame
[[nodiscard]] glm::float64 get_time_delta();

/// @brief Gets if the implementation has locked the mouse
/// @return if the mouse is locked
[[nodiscard]] bool get_is_game_locked();

/// @brief
/// @param callback
void each_scene(const std::function<void(entt::registry&)>& callback);

}
