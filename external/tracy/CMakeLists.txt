
if(NOT LUCARIA_BUILD_COMPILERS)

	add_library(lucaria_external_tracy STATIC EXCLUDE_FROM_ALL "upstream/tracy/public/TracyClient.cpp")
	target_compile_features(lucaria_external_tracy PUBLIC cxx_std_11)
	target_include_directories(lucaria_external_tracy SYSTEM PUBLIC "upstream/tracy/public")
	target_compile_definitions(lucaria_external_tracy PUBLIC
		TRACY_ENABLE
		TRACY_ON_DEMAND
		TRACY_ONLY_LOCALHOST)

	if(WIN32)
		target_link_libraries(lucaria_external_tracy PUBLIC ws2_32 dbghelp)
	endif()

	if(UNIX AND NOT APPLE)
		target_link_libraries(lucaria_external_tracy PUBLIC pthread dl)
	endif()

else()

	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tracy_generated/data")
	set(tracy_embed_executable "${CMAKE_CURRENT_BINARY_DIR}/lucaria_tracy_embed${CMAKE_EXECUTABLE_SUFFIX}")
	set(tracy_font_fixed "${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/profiler/src/font/FiraCode-Retina.ttf")
	set(tracy_font_icons "${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/profiler/src/font/Font Awesome 6 Free-Solid-900.otf")
	set(tracy_font_normal "${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/profiler/src/font/Roboto-Regular.ttf")
	set(tracy_font_bold "${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/profiler/src/font/Roboto-Bold.ttf")
	set(tracy_font_italic "${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/profiler/src/font/Roboto-Italic.ttf")
	set(tracy_font_bold_italic "${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/profiler/src/font/Roboto-BoldItalic.ttf")
	set(tracy_manual "${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/manual/tracy.md")
	set(tracy_system_prompt "${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/profiler/src/llm/system.prompt.md")
	set(tracy_system_reminder "${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/profiler/src/llm/system.reminder.md")

	if(NOT EXISTS "${tracy_embed_executable}")
		message(STATUS "Building Tracy embed helper")
		if(MSVC)
			execute_process(
				COMMAND "${CMAKE_CXX_COMPILER}"
					/nologo
					/std:c++17
					/EHsc
					"/I${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/public"
					"${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/profiler/helpers/embed.cpp"
					"${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/public/common/tracy_lz4.cpp"
					"${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/public/common/tracy_lz4hc.cpp"
					"/Fe${tracy_embed_executable}"
				RESULT_VARIABLE tracy_embed_build_result)
		else()
			execute_process(
				COMMAND "${CMAKE_CXX_COMPILER}"
					-std=c++17
					"-I${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/public"
					"${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/profiler/helpers/embed.cpp"
					"${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/public/common/tracy_lz4.cpp"
					"${CMAKE_CURRENT_LIST_DIR}/upstream/tracy/public/common/tracy_lz4hc.cpp"
					-o "${tracy_embed_executable}"
				RESULT_VARIABLE tracy_embed_build_result)
		endif()
		if(NOT tracy_embed_build_result EQUAL 0)
			message(FATAL_ERROR "Failed to build Tracy embed helper")
		endif()
	endif()

	function(lucaria_generate_tracy_embed name input_file)
		set(output_base "${CMAKE_CURRENT_BINARY_DIR}/tracy_generated/data/${name}")
		set(output_header "${output_base}.hpp")
		set(output_source "${output_base}.cpp")
		if(NOT EXISTS "${output_header}" OR NOT EXISTS "${output_source}")
			message(STATUS "Generating Tracy embedded data ${name}")
			execute_process(
				COMMAND "${tracy_embed_executable}"
					"${name}"
					"${input_file}"
					"${output_base}"
				RESULT_VARIABLE tracy_embed_run_result)
			if(NOT tracy_embed_run_result EQUAL 0)
				message(FATAL_ERROR "Failed to generate Tracy embedded data: ${name}")
			endif()
		endif()
		set(tracy_generated_embed_sources
			${tracy_generated_embed_sources}
			"${output_source}"
			PARENT_SCOPE)
		set(tracy_generated_embed_headers
			${tracy_generated_embed_headers}
			"${output_header}"
			PARENT_SCOPE)
	endfunction()

	set(tracy_generated_embed_sources "")
	set(tracy_generated_embed_headers "")
	lucaria_generate_tracy_embed(FontFixed "${tracy_font_fixed}")
	lucaria_generate_tracy_embed(FontIcons "${tracy_font_icons}")
	lucaria_generate_tracy_embed(FontNormal "${tracy_font_normal}")
	lucaria_generate_tracy_embed(FontBold "${tracy_font_bold}")
	lucaria_generate_tracy_embed(FontItalic "${tracy_font_italic}")
	lucaria_generate_tracy_embed(FontBoldItalic "${tracy_font_bold_italic}")
	lucaria_generate_tracy_embed(Manual "${tracy_manual}")
	lucaria_generate_tracy_embed(SystemPrompt "${tracy_system_prompt}")
	lucaria_generate_tracy_embed(SystemReminder "${tracy_system_reminder}")

    set(tracy_common_sources
        "upstream/tracy/public/common/tracy_lz4.cpp"
        "upstream/tracy/public/common/tracy_lz4hc.cpp"
        "upstream/tracy/public/common/TracySocket.cpp"
        "upstream/tracy/public/common/TracyStackFrames.cpp"
        "upstream/tracy/public/common/TracySystem.cpp")
    set(tracy_server_sources
        "upstream/tracy/server/TracyMemory.cpp"
        "upstream/tracy/server/TracyMmap.cpp"
        "upstream/tracy/server/TracyPrint.cpp"
        "upstream/tracy/server/TracySysUtil.cpp"
        "upstream/tracy/server/TracyTaskDispatch.cpp"
        "upstream/tracy/server/TracyTextureCompression.cpp"
        "upstream/tracy/server/TracyThreadCompress.cpp"
        "upstream/tracy/server/TracyWorker.cpp")
	set(tracy_viewer_sources
		"upstream/tracy/profiler/src/profiler/TracyAchievementData.cpp"
		"upstream/tracy/profiler/src/profiler/TracyAchievements.cpp"
		"upstream/tracy/profiler/src/profiler/TracyBadVersion.cpp"
		"upstream/tracy/profiler/src/profiler/TracyColor.cpp"
		"upstream/tracy/profiler/src/profiler/TracyConfig.cpp"
		"upstream/tracy/profiler/src/profiler/TracyEmbed.cpp"
		"upstream/tracy/profiler/src/profiler/TracyEventDebug.cpp"
		"upstream/tracy/profiler/src/profiler/TracyFileselector.cpp"
		"upstream/tracy/profiler/src/profiler/TracyFilesystem.cpp"
		"upstream/tracy/profiler/src/profiler/TracyImGui.cpp"
		"upstream/tracy/profiler/src/profiler/TracyManualData.cpp"
		"upstream/tracy/profiler/src/profiler/TracyMarkdown.cpp"
		"upstream/tracy/profiler/src/profiler/TracyMicroArchitecture.cpp"
		"upstream/tracy/profiler/src/profiler/TracyMouse.cpp"
		"upstream/tracy/profiler/src/profiler/TracyProtoHistory.cpp"
		"upstream/tracy/profiler/src/profiler/TracySourceContents.cpp"
		"upstream/tracy/profiler/src/profiler/TracySourceTokenizer.cpp"
		"upstream/tracy/profiler/src/profiler/TracySourceView.cpp"
		"upstream/tracy/profiler/src/profiler/TracyStorage.cpp"
		"upstream/tracy/profiler/src/profiler/TracyTexture.cpp"
		"upstream/tracy/profiler/src/profiler/TracyTimelineController.cpp"
		"upstream/tracy/profiler/src/profiler/TracyTimelineItem.cpp"
		"upstream/tracy/profiler/src/profiler/TracyTimelineItemCpuData.cpp"
		"upstream/tracy/profiler/src/profiler/TracyTimelineItemGpu.cpp"
		"upstream/tracy/profiler/src/profiler/TracyTimelineItemPlot.cpp"
		"upstream/tracy/profiler/src/profiler/TracyTimelineItemThread.cpp"
		"upstream/tracy/profiler/src/profiler/TracyUserData.cpp"
		"upstream/tracy/profiler/src/profiler/TracyUtility.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_Annotations.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_Callstack.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_Compare.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_ConnectionState.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_ContextSwitch.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_CpuData.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_FindZone.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_FlameGraph.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_FrameOverview.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_FrameTimeline.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_FrameTree.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_GpuTimeline.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_Locks.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_Manual.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_Memory.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_Messages.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_Navigation.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_NotificationArea.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_Options.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_Playback.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_Plots.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_Ranges.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_Samples.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_Statistics.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_Timeline.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_TraceInfo.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_Utility.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_ZoneInfo.cpp"
		"upstream/tracy/profiler/src/profiler/TracyView_ZoneTimeline.cpp"
		"upstream/tracy/profiler/src/profiler/TracyWeb.cpp"
		"upstream/tracy/profiler/src/profiler/TracyLlm.cpp"
		"upstream/tracy/profiler/src/profiler/TracyLlmApi.cpp"
		"upstream/tracy/profiler/src/profiler/TracyLlmChat.cpp"
		"upstream/tracy/profiler/src/profiler/TracyLlmEmbeddings.cpp"
		"upstream/tracy/profiler/src/profiler/TracyLlmTools.cpp")	
	set(tracy_viewer_support_sources
		"upstream/tracy/profiler/src/ConnectionHistory.cpp"
		"upstream/tracy/profiler/src/Filters.cpp"
		"upstream/tracy/profiler/src/Fonts.cpp"
		"upstream/tracy/profiler/src/HttpRequest.cpp"
		"upstream/tracy/profiler/src/ImGuiContext.cpp"
		"upstream/tracy/profiler/src/ini.c"
		"upstream/tracy/profiler/src/IsElevated.cpp"
		"upstream/tracy/profiler/src/ResolvService.cpp"
		"upstream/tracy/profiler/src/RunQueue.cpp"
		"upstream/tracy/profiler/src/WindowPosition.cpp")
	set(tracy_viewer_backend_sources
		"upstream/tracy/profiler/src/BackendGlfw.cpp")
    set(lucaria_tracy_host_sources
		${tracy_common_sources}
		${tracy_server_sources}
		${tracy_viewer_sources}
		${tracy_viewer_support_sources}
		${tracy_viewer_backend_sources}
		${tracy_generated_embed_sources}
		${tracy_generated_embed_headers}
		"source/zigzag_texture.cpp")

	message(STATUS "Using patched ImGui at ${lucaria_patched_imgui_dir}")
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/tracy_generated/GitRef.hpp" "#pragma once\nnamespace tracy { static constexpr const char* GitRef = \"lucaria\"; }\n")
    add_library(lucaria_external_tracy STATIC EXCLUDE_FROM_ALL ${lucaria_tracy_host_sources})
    target_compile_features(lucaria_external_tracy PUBLIC cxx_std_20)
	target_include_directories(lucaria_external_tracy BEFORE PRIVATE "${lucaria_patched_imgui_dir}")
    target_include_directories(lucaria_external_tracy PUBLIC
		"upstream/tracy/public"
		"upstream/tracy/public/common"
		"upstream/tracy/server"
		"upstream/tracy/profiler/src"
		"upstream/tracy/profiler/src/profiler"
		"upstream/tracy/dtl"
		"${CMAKE_CURRENT_BINARY_DIR}/tracy_generated")

	target_compile_definitions(lucaria_external_imgui PUBLIC -DIMGUI_USE_WCHAR32)

    target_link_libraries(lucaria_external_tracy PUBLIC
        lucaria_external_base64
        lucaria_external_capstone
        lucaria_external_curl
        lucaria_external_zstd
        lucaria_external_ppqsort
        lucaria_external_imgui
        lucaria_external_glfw
        lucaria_external_json
        lucaria_external_md4c
        lucaria_external_usearch
        lucaria_external_pugixml
        lucaria_external_tidy
        lucaria_external_nfd)

    if(WIN32)
        target_compile_definitions(lucaria_external_tracy PUBLIC
            NOMINMAX
            WIN32_LEAN_AND_MEAN
            _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)
        target_link_libraries(lucaria_external_tracy PUBLIC ws2_32 dbghelp opengl32)
    elseif(UNIX)
        target_link_libraries(lucaria_external_tracy PUBLIC pthread dl)
    endif()

	if(MSVC)
		target_compile_options(lucaria_external_tracy PRIVATE 
			/wd4005
			/wd4018
			/wd4101
			/wd4146
			/wd4305)
		target_compile_options(lucaria_external_tracy PUBLIC 
			/wd4244
			/wd4267
			/wd4996)
	endif()

endif()