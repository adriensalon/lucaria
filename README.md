# lucaria_engine

Minimal CMake/C++17 game engine and toolchain made to run my gamedev/music portfolio at [777reroll.fr](https://777reroll.fr). It compiles to Win32, Android and Emscripten without specific configuration.

![Texte alternatif](doc/img/777reroll.png "Titre de l'image")

It also builds a resource compiler that generates compressed files that can be fetched by the runtime: animations, skeletons and motion tracks are exported as ozz-animation binaries, audio data as ogg vorbis, images as S3TC for desktops, ECT2 for mobiles and binary fallback, fonts data as woff2, text and geometry data as binary. A Blender script is provided to export named events from animation tracks so that they can be converted by the compiler.

It features basic rendering (with post processing FXAA), 3D and screen space GUIs, spatialized sound playback, animations blending with events, and root motion based locomotion that can be combined to realtime physics (collisions and forces) as ECS components to benefit from SoA layout.

This code was made to overcome game engines limitations concerning their WebGL target through Emscripten, such as fetching assets dynamically: Godot and Unity use the --preload-file option that fetches all the assets required for the game before entering main(), preventing us to draw anything while waiting. Here we dynamically fetch assets when needed so that we can only download the file formats supported by the implementation (ETC2/S3TC textures for example). For developing commercial games please use a real game engine such as Godot.

## Getting started

Download this and extract to a subfolder of your project, and use the CMake command `add_subdirectory("my/subfolder/path")` to have lucaria configured as a dependency of your project. Then create a target for the assets compiler.

```cmake
add_lucaria_assets(MyGameCompilerExecutable
    IMPORT_DIR "${MyGameAssetsImportDir}" 
    ASSETS_DIR "${MyGameAssetsImportDir}/install/assets")
```

Functions are provided for each supported platform. Only the `SOURCES` argument is required, others are optional. Some platforms expose more optional arguments.

```cmake
add_lucaria_game_win32(MyGame 
    SOURCES "${MyGameSources}"
    INCLUDES "${CMAKE_CURRENT_LIST_DIR}/source"
    INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/install"

    # Hides the Win32 console (defaults to OFF)
    HIDE_CONSOLE ON)
```
    
```cmake
add_lucaria_game_android(MyGame 
    SOURCES "${MyGameSources}"
    INCLUDES "${CMAKE_CURRENT_LIST_DIR}/source"
    INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/install"

    # On Android assets need to be packaged
    ASSETS_DIR "${CMAKE_CURRENT_LIST_DIR}/install/assets"

    # Target ABI (defaults to "arm64-v8a")
    ANDROID_ABI "arm64-v8a"

    # Target platform (defaults to "android-24")
    ANDROID_PLATFORM "android-24")
```

```cmake
add_lucaria_game_web(MyGame 
    SOURCES "${MyGameSources}"
    INCLUDES "${CMAKE_CURRENT_LIST_DIR}/source"
    INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/install"

    # On Emscripten assets can be packaged with --preload-files
    # (defaults to HTTP fetching paths on same origin with caching)
    ASSETS_DIR "${CMAKE_CURRENT_LIST_DIR}/install/assets"

    # Custom HTML shell source (defaults to the one provided in
    # lucaria/source/game/web/shell.html)
    HTML_SHELL "${CMAKE_CURRENT_LIST_DIR}/source/main.html") 
```

## Fetch assets

Assets are not loaded at startup, the runtime exposes a typed fetching API that resolves resources on demand. Each fetch call returns a lightweight handle that becomes valid once the underlying data has been downloaded, decompressed and uploaded to the appropriate subsystem (GPU, audio device, etc.).

This mechanism is used uniformly across all platforms and allows the application to remain responsive while assets are being retrieved.

```cpp
void fetch_assets(entt::registry& my_scene) 
{
    using namespace lucaria;

    // fetch an ozz-animation binary animation
    fetched<animation> my_animation = fetch_animation("assets/my_animation.bin");
    
    // fetch an ozz-animation binary skeleton
    fetched<skeleton> my_skeleton = fetch_skeleton("assets/my_skeleton.bin");
    
    // fetch an ozz-animation binary motiion track
    fetched<motion_track> my_motion_track = fetch_skeleton("assets/my_motion_track.bin");

    // fetch a woff2 font and update font texture
    fetched<font> my_font = fetch_font("assets/my_font.bin", 38.f);

    // fetch binary geometry data as OpenGL mesh buffers
    fetched<mesh> my_mesh = fetch_mesh("assets/my_mesh.bin");

    // fetch binary geometry data as bullet collision data
    fetched<shape> my_shape = fetch_shape("assets/my_mesh.bin"); 
    
    // fetch binary fallback texture as OpenGL texture
    fetched<texture> my_texture = fetch_texture("assets/my_texture.bin");

    // fetch ogg vorbis audio data, decompress and upload to OpenAL buffer
    fetched<sound_track> my_sound_track = fetch_sound_track("assets/my_sound_track.bin");

    // etc...
}
```

## Add entities and components

Gameplay objects are defined by composing components on entities stored in an EnTT registry. Each component maps directly to a runtime system (rendering, animation, physics, audio) and exposes a fluent configuration interface.

This approach keeps entity construction explicit and data-oriented, while allowing systems to operate independently on tightly packed component arrays. Transform hierarchies, animation state, collision behavior and audio playback are all expressed through components.

```cpp
void add_entities_and_components(entt::registry& my_scene) 
{
    using namespace lucaria;
    const entt::entity my_entity = my_scene.create();

    // add a transform component
    my_scene.emplace<component_transform>(my_entity)
        .use_parent()
        .set_position_warp(glm::vec3(1.6f, 0.f, 0.f));

    // add an unlit model component
    my_scene.emplace<component_model_unlit>(my_entity)
        .use_color(my_texture)
        .use_mesh(my_mesh);

    // add an animator component
    my_scene.emplace<animator_component>(my_entity)
        .use_skeleton(my_skeleton)
        .use_animation("walk", my_animation)
        .use_motion_track("walk", my_motion_track);

    // add a speaker component 
    my_scene.emplace<component_speaker_spatial>(my_entity)
        .use_sound(my_sound_track);

    // add a dynamic rigidbody component
    my_scene.emplace<component_rigidbody_dynamic>(my_entity)
        .use_shape(room00_state->shape_character)
        .set_group_layer(collision_layer::layer_1)
        .set_mask_layer(collision_layer::layer_0)
        .set_mass(70.f)
        .set_friction(1.f)
        .set_lock_angular({ true, false, true })
        .set_linear_pd(26000.f, 2600.f, 2000.f)
        .set_angular_pd(1200.f, 150.f, 900.f);

    // etc...
}
```

## Implement the game loop

Once initialized, the engine takes ownership of frame timing, input polling and system execution. User code is executed through per-frame callbacks, where gameplay logic, input handling and high-level state updates can be performed.

```cpp
using namespace lucaria;

static std::vector<entt::registry> my_scenes;

int lucaria_main(int argc, char** argv)
{
    fetch_assets();
    add_entities_and_components(my_scenes.emplace_back());

    set_fxaa_enable(true);
    set_camera_near(0.01f);
    set_skybox_rotation(-90.f);
    
    set_update_callback(my_scenes, []() {
        // this code will be executed every frame
        // this is where you place logic

        if (get_is_keyboard_supported() && get_keys()[button_keys::keyboard_z]) {
            // ...
        }
        
    });

    return 0;
}
```
