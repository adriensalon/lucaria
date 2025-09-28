// #include <lucaria/ecs/component/interface.hpp>

// namespace lucaria {
// namespace ecs {

// interface_component<interface_mode::screen>& interface_component<interface_mode::screen>::use_font(font& from)
// {
//     _fonts.emplace_back().emplace(from);
//     return *this;
// }

// interface_component<interface_mode::screen>& interface_component<interface_mode::screen>::use_font(fetched<font>& from)
// {
//     _fonts.emplace_back().emplace(from);
//     return *this;
// }

// interface_component<interface_mode::screen>& interface_component<interface_mode::screen>::set_callback(const std::function<void()>& callback)
// {
//     return set_callback([callback] (const std::vector<ImFont*>&) {
//         callback();
//     });
// }

// interface_component<interface_mode::screen>& interface_component<interface_mode::screen>::set_callback(const std::function<void(const std::vector<ImFont*>&)>& callback)
// {
//     _imgui_callback = callback;
//     return *this;
// }


// }
// }