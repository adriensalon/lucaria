#include <core/fetch.hpp>
#include <ecs/system/async.hpp>
#include <core/world.hpp>
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

    template <typename ref_t>
    static void update_ref(std::optional<std::shared_future<std::shared_ptr<ref_t>>>& future, std::shared_ptr<ref_t>& value)
    {
        bool _must_erase = false;
        if (future.has_value()) {
            std::shared_future<std::shared_ptr<ref_t>>& _future_value = future.value();
            if (get_is_future_ready<std::shared_ptr<ref_t>>(_future_value)) {
                value = _future_value.get();
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
    each_level([](entt::registry& _registry) {
        // _registry.view<model_component<model_shader::pbr>>().each([](model_component<model_shader::pbr>& _model) {
        //     detail::update_ref<mesh_ref>(_model._fetched_mesh, _model._mesh);
        //     detail::update_ref<material_ref>(_model._fetched_material, _model._material);
        // });
        // _registry.view<model_component<model_shader::blockout>>().each([](model_component<model_shader::blockout>& _model) {
        //     detail::update_ref<mesh_ref>(_model._fetched_mesh, _model._mesh);
        //     detail::update_ref<material_ref>(_model._fetched_material, _model._material);
        // });
        _registry.view<model_component<model_shader::unlit>>().each([](model_component<model_shader::unlit>& _model) {
            detail::update_ref<mesh_ref>(_model._fetched_mesh, _model._mesh);
            detail::update_ref<material_ref>(_model._fetched_material, _model._material);
        });
        // _registry.view<animator_component>().each([](animator_component& _animator) {
        //     detail::update_ref<skeleton_ref>(_animator._future_skeleton, _animator._skeleton);
        //     detail::update_refmap<std::string, animation_ref>(_animator._future_animations, _animator._animations);
        // });
    });
}