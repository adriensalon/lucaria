# lucaria

Minimal game engine 

## Features
- PBR renderer
- collisions
- skeletal animations
- spatial audio playback
- assets compilation

## Usage

Just use this library with the `add_subdirectory()` CMake command. Create a target for assets compilation using this CMake function

```cmake
add_lucaria_assets(mygame_assets
    "${mygame_assets_input_dir}" 
    "${mygame_assets_output_dir}")
```

Create a target for each platform using these CMake functions. Only the `SOURCES` argument is required, others are optional

```cmake
add_lucaria_game_web(mygame 
    SOURCES "${mygame_sources}"
    INCLUDES "${CMAKE_CURRENT_LIST_DIR}/source"
    DEFINES
    BUILD_ARGS
    INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/install")
    
add_lucaria_game_win32(mygame 
    SOURCES "${mygame_sources}"
    INCLUDES "${CMAKE_CURRENT_LIST_DIR}/source"
    INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/install")
```
