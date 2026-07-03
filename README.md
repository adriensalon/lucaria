# lucaria_engine

## Getting started

Download this and extract to a subfolder of your project, and use the CMake command `add_subdirectory("my/subfolder/path")` to have lucaria configured as a dependency of your project.

```cmake
add_game(
	NAME MyGame
	GAME_DIR "${CMAKE_CURRENT_LIST_DIR}"
	INSTALL_DIR install
	ASSETS_DIR assets
	SOURCES
		source/*.cpp
		source/*.gslc
	INCLUDE_DIRS
		source
	COMPILE_DEFINITIONS
	COMPILE_OPTIONS
	WIN32_BACKEND OPENGL
	PSP_ENCRYPT ON)
```

## Add a scene

```cpp
struct MyScene {
	handle_shape shape_character;
    handle_texture texture_character;
    handle_mesh mesh_character;
    handle_skeleton skeleton_character;
    handle_animation animation_character_walk;
    handle_motion_track motion_track_character_walk;
	handle_entity character_entity;
	
    void start(context_game& game)
	{
		shape_character = game.objects.create_shape_capsule(0.3f, 1.5f);
		texture_character = game.objects.fetch_texture("room00/image_character.bin");
		mesh_character = game.objects.fetch_mesh("room00/geometry_character.bin");
		skeleton_character = game.objects.fetch_skeleton("room00/geometry_character_skeleton.bin");
		animation_character_walk = game.objects.fetch_animation("room00/geometry_character_animation_walk.bin");
		motion_track_character_walk = game.objects.fetch_motion_track("room00/geometry_character_animation_walk_motion_track.bin");

		// character
		character_entity = game.scenes.create_entity();
		game.scenes.create_transform(character_entity);
		game.scenes.create_unlit_model(character_entity)
			.use_mesh(mesh_character)
			.use_color(texture_character);			
		game.scenes.create_dynamic_rigidbody(game.dynamics, character_entity)
			.use_shape(shape_character)
			.set_group_layer(collision_layer::layer_1)
			.set_mask_layer(collision_layer::layer_0)
			.set_mass(70.f)
			.set_friction(1.f)
			.set_lock_angular({ true, false, true })
			.set_linear_pd(26000.f, 2600.f, 2000.f)
			.set_angular_pd(1200.f, 150.f, 900.f);
		game.scenes.create_animator(character_entity)
			.use_animation("walk", animation_character_walk)
			.use_motion_track("walk", motion_track_character_walk)
			.use_skeleton(skeleton_character);
		game.mixer.use_listener_transform(character_entity);
		game.rendering.use_camera_transform(character_entity);
		game.rendering.use_camera_bone(character_entity, "mixamorig9:Head");
	}

	void update(context_game& game)
	{
		if (game.objects.async_fetches_waiting() <= 0) {
			if (!game.input.is_touch_supported()) {
				control_animation_play = game.input.button_events()[input_key::keyboard_z].state || game.input.button_events()[input_key::keyboard_w].state;
				control_camera_rotation = game.input.mouse_position_delta();
			}
			static bool _is_walking = false;
			if (control_animation_play && !_is_walking) {
				game.scenes.get_animator(character_entity).get_controller("walk").set_play();
				_is_walking = true;
				game.scenes.each_view<component_animator>(mylgslfunc2);
			} else if (!control_animation_play && _is_walking) {
				game.scenes.get_animator(character_entity).get_controller("walk").set_pause();
				_is_walking = false;
			}
			if (game.input.button_events()[input_key::keyboard_e].state) {
				game.scenes.get_dynamic_rigidbody(character_entity).add_linear_impulse({ 0, 30, 0 });
			}
			game.rendering.set_camera_rotation(control_camera_rotation.x, control_camera_rotation.y);
		}
	}
};

LUCARIA_REGISTER_SCENE_IMPLEMENTATION(MyScene)
LUCARIA_MAIN_SCENE_IMPLEMENTATION(MyScene)
```


