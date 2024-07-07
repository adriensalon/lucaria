#include <glue/fetch.hpp>
#include <ecs/system/async.hpp>
#include <ecs/system/world.hpp>
#include <ecs/component/animator.hpp>
#include <ecs/component/model.hpp>

namespace detail {

    template <typename ref_t>
    static void update_ref(std::optional<std::future<ref_t>>& future, std::optional<ref_t>& value)
    {
        bool _must_erase = false;
        if (future.has_value()) {
            std::future<ref_t>& _future_value = future.value();
            if (get_is_future_ready<ref_t>(_future_value)) {
                value = std::move(_future_value.get());
                _must_erase = true;
            }
        }
        if (_must_erase) {
            future = std::nullopt;
        }
    }

    template <typename key_t, typename ref_t>
    static void update_refmap(
        std::unordered_map<key_t, std::optional<std::future<ref_t>>>& futures, 
        std::unordered_map<key_t, std::optional<ref_t>>& values, 
        const std::function<void(const key_t)>& callback = nullptr)
    {
        std::vector<key_t> _to_erase = {};
        for (std::pair<const key_t, std::optional<std::future<ref_t>>>& _pair : futures) {
            if (_pair.second.has_value()) {
                std::future<ref_t>& _future = _pair.second.value();
                if (get_is_future_ready<ref_t>(_future)) {
                    values.insert_or_assign(_pair.first, std::move(_future.get()));
                    if (callback) {
                        // callback(_pair.first);
                    }
                    _to_erase.push_back(_pair.first);
                }
            }
        }
        for (const key_t& _key : _to_erase) {
            futures.erase(_key);
        }
    }
}

void async_system::update()
{
    world_system::each_level([](entt::registry& _registry) {
        _registry.view<model_component>().each([](model_component& _model) {
            detail::update_ref<mesh_ref>(_model._future_mesh, _model._mesh);
            detail::update_refmap<model_texture, texture_ref>(_model._future_textures, _model._textures);
        });
        _registry.view<animator_component>().each([](animator_component& _animator) {
            detail::update_ref<skeleton_ref>(_animator._future_skeleton, _animator._skeleton);
            detail::update_refmap<std::string, animation_ref>(_animator._future_animations, _animator._animations);
        });
    });
}