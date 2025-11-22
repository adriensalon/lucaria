# lucaria

Minimal game engine 

GLES3

Assets compilation

Cross platform for the same source

## Requirements



__CMake__ is required as the build system

__MSVC__ or __Clang__ with __C++17__ were tested


## Components and features
- Transform
- Model
- Rigidbody
- Animator
- Interface
- Speaker

## Supported file formats

#### Geometry (.gltf, .glb)

Geometry files will be optimized and compiled to `<file>.bin` binary assets. If the source file contains a skeleton, a corresponding skeleton binary is generated at `<file>_skeleton.bin`. For each animation found in the file, the compiler generates a `<file>_animation_<name>.bin` and a `<file>_animation_<name>_motion_track.bin` files.

#### Images (.png, .jpg, .bmp)

Image files will be compiled to `<file>.bin` binary assets. Only PNGs will generate `<file_s3tc>.bin` and `<file_etc2>.bin` compressed assets that can be loaded directly on the GPU without having to decompress.

#### Shaders (.glsl, .txt, .vert, .frag)
Shader files will be be compiled to `<file>.bin` binary asset.

#### Audio (.wav, .aiff)
Audio files will be compiled to `<file>.bin` OGG Vorbis compressed assets to be decompressed when loaded

#### Fonts (.ttf, .otf)
Font files will be compiled to WOFF2 compressed assets and decompressed when loaded

### Event tracks (.evtt)
Event tracks exported from Blender with the addon/lucaria_event_track_export.py script will be compiled to binary




## Getting started

Just use this library with the `add_subdirectory()` CMake command. Create a target for assets compilation using this CMake function


### Add a Win32 target

Create a target for each platform using these CMake functions. Only the `SOURCES` argument is required, others are optional. Some platforms expose more optional arguments

```cmake
add_lucaria_game_win32(MyGame 
    SOURCES "${MyGameSources}"
    INCLUDES "${CMAKE_CURRENT_LIST_DIR}/source"
    INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/install"

    # Hides the Win32 console (defaults to OFF)
    HIDE_CONSOLE ON)
```

### Add an Android target
    
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

### Add an Emscripten web target

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

### Implement the game loop

```cpp
static std::vector<entt::registry> my_scenes;

int lucaria_main(int argc, char** argv)
{
    // set options
    lucaria::set_fxaa(true);
    lucaria::set_fxaa_parameters();

    // run game loop
    lucaria::run(my_scenes, []() {
        // this code will be executed every frame
        // this is where you place logic
        
    });

    return 0;
}
```

### Create assets on the CPU

```cpp
lucaria::geometry_data my_geometry_data;

my_geometry_data.positions = {
    glm::vec3(-1.f, -1.f, 0.f),
    glm::vec3(1.f, -1.f, 0.f),
    glm::vec3(1.f, 1.f, 0.f),
    glm::vec3(-1.f, 1.f, 0.f),
};

my_geometry_data.indices = {
    glm::uvec3(0, 1, 2),
    glm::uvec3(0, 2, 3),
};

// Object geometry represents geometry on the CPU
lucaria::geometry my_geometry(std::move(my_geometry_data));

// Object mesh represents uploaded geometry on the GPU
lucaria::mesh my_mesh(my_geometry);

// Uploaded data still can be rewritten at runtime
my_mesh.update_attribute(my_geometry, lucaria::mesh_attribute::position, my_geometry_data.count);
```

### Add an assets compiler target

```cmake
add_lucaria_assets(MyGameCompiler
    IMPORT_DIR "${MyGameAssetsImportDir}" 
    ASSETS_DIR "${CMAKE_CURRENT_LIST_DIR}/install/assets")
```

```cpp
lucaria::set_fetch_path("assets");
```

Sync

```cpp
// Load assets syncronously
lucaria::texture my_texture("room00/my_image.bin");
lucaria::mesh my_mesh("room00/my_geometry.bin");

// Reference in a component
const entt::entity my_entity = my_scene->create();
my_scene->emplace<lucaria::unlit_model_component>(my_entity)
    .use_color(my_texture)
    .use_mesh(my_mesh);
```

or
Async


```cpp
// Load assets asyncronously as lucaria::fetched<T>
auto my_mesh = lucaria::fetch_mesh("room00/my_geometry.bin");
auto my_texture = lucaria::fetch_texture("room00/my_image.bin");

// Reference in a component as if already available
const entt::entity my_entity = my_scene->create();
my_scene->emplace<lucaria::unlit_model_component>(my_entity)
    .use_color(my_texture)
    .use_mesh(my_mesh);
```

### Example

Build configuration features




### Misc features
- PBR renderer
- collisions
- skeletal animations
- spatial audio playback
- assets compilation