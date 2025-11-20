# lucaria

Minimal game engine 

## Requirements

## Getting started

Just use this library with the `add_subdirectory()` CMake command. Create a target for assets compilation using this CMake function

```cmake
add_lucaria_assets(mygame_assets
    "${mygame_assets_input_dir}" 
    "${mygame_assets_output_dir}")
```

Create a target for each platform using these CMake functions. Only the `SOURCES` argument is required, others are optional. Some platforms expose more optional arguments

```cmake
add_lucaria_game_linux(mygame 
    SOURCES "${mygame_sources}"
    INCLUDES "${CMAKE_CURRENT_LIST_DIR}/source"
    INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/install")
    
add_lucaria_game_macos(mygame 
    SOURCES "${mygame_sources}"
    INCLUDES "${CMAKE_CURRENT_LIST_DIR}/source"
    INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/install")

# win32 builds can set HIDE_CONSOLE to hide the console and link to WinMain
add_lucaria_game_win32(mygame 
    SOURCES "${mygame_sources}"
    INCLUDES "${CMAKE_CURRENT_LIST_DIR}/source"
    INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/install"
    HIDE_CONSOLE ON) 

# web builds can set HTML_SHELL to use another source than the default one
add_lucaria_game_web(mygame 
    SOURCES "${mygame_sources}"
    INCLUDES "${CMAKE_CURRENT_LIST_DIR}/source"
    INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/install"
    HTML_SHELL "${CMAKE_CURRENT_LIST_DIR}/source/main.html") 
```

Build configuration features

```cpp
using namespace lucaria;

shape shape_character = create_capsule_shape(0.3f, 1.5f);

fetched<texture> mytexture = fetch_texture("assets/mytexture.bin");
fetched<mesh> mycharacter = fetch_mesh("assets/mycharacter.bin");
```

```cpp
static std::vector<entt::registry> scenes;

int main()
{
    // 
    lucaria::setup(scenes);

    // create a scene
    scenes.reserve(1);
    entt::registry& my_scene = scenes.emplace_back();

    // add some components
    
    // set options
    lucaria::set_fxaa(true);
    lucaria::set_fxaa_parameters();

    // run game loop
    lucaria::run([&my_scene]() {
        // this code will be executed every frame
        // this is where you place logic
        
    });

    return 0;
}
```

## Features overview

### Resources

### Components

### Misc features
- PBR renderer
- collisions
- skeletal animations
- spatial audio playback
- assets compilation