#include <AL/al.h>
#include <AL/alc.h>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/manager_app.hpp>
#include <lucaria/core/rendering_backend.hpp>
#include <lucaria/core/rendering_vulkan.hpp>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <backends/imgui_impl_opengl3.h>
struct ImGui_ImplOpenGL3_Data {
    GLuint GlVersion;
    char GlslVersionString[32];
    bool GlProfileIsES2;
    bool GlProfileIsES3;
    bool GlProfileIsCompat;
    GLint GlProfileMask;
    GLuint FontTexture;
    GLuint ShaderHandle;
    GLint AttribLocationTex;
    GLint AttribLocationProjMtx;
    GLuint AttribLocationVtxPos;
    GLuint AttribLocationVtxUV;
    GLuint AttribLocationVtxColor;
    unsigned int VboHandle, ElementsHandle;
    GLsizeiptr VertexBufferSize;
    GLsizeiptr IndexBufferSize;
    bool HasClipOrigin;
    bool UseBufferSubData;
    ImGui_ImplOpenGL3_Data() { memset((void*)this, 0, sizeof(*this)); }
};
#endif

#if defined(LUCARIA_BACKEND_VULKAN)
#include <backends/imgui_impl_vulkan.h>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <backends/imgui_impl_psp.h>
#endif

namespace lucaria {
namespace detail {

    manager_window::~manager_window()
    {
        if (screen_context != nullptr) {
            ImGui::SetCurrentContext(screen_context);
        }
        shared_font_texture.reset();
        destroy_imgui();
        shared_font_atlas.reset();
        destroy_openal();
#if defined(LUCARIA_PLATFORM_WIN32) || defined(LUCARIA_PLATFORM_LINUX)
#if defined(LUCARIA_BACKEND_VULKAN)
        rendering_vulkan_shutdown();
#endif
        if (window != nullptr) {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
#endif
    }

    ImGuiContext* manager_window::create_shared_imgui_context()
    {
        ImGuiContext* _context = ImGui::CreateContext(shared_font_atlas.get());
        ImGui::SetCurrentContext(_context);
#if defined(LUCARIA_PLATFORM_ANDROID)
        if (must_install_imgui_callbacks) {
            ImGui_ImplAndroid_Init(app->window);
            must_install_imgui_callbacks = false;
        }
#endif

#if defined(LUCARIA_PLATFORM_WIN32) || defined(LUCARIA_PLATFORM_LINUX) || defined(LUCARIA_PLATFORM_PSP)
        if (must_install_imgui_callbacks) {
#if defined(LUCARIA_BACKEND_OPENGL)
            ImGui_ImplGlfw_InitForOpenGL(window, true);
#endif
#if defined(LUCARIA_BACKEND_VULKAN)
            ImGui_ImplGlfw_InitForVulkan(window, true);
#endif
#if defined(LUCARIA_PLATFORM_PSP)
            ImGui_ImplPSP_Init();
#endif
            must_install_imgui_callbacks = false;
        }
#endif

#if defined(LUCARIA_BACKEND_OPENGL)
        ImGui_ImplOpenGL3_Init("#version 300 es");
        ImGui_ImplOpenGL3_DestroyFontsTexture();
#endif
#if defined(LUCARIA_BACKEND_VULKAN)
        rendering_vulkan_initialize_imgui();
#endif
        if (shared_font_texture) {
            ImGui::GetIO().Fonts->SetTexID(shared_font_texture->imgui_texture());
        }

#if defined(LUCARIA_BACKEND_OPENGL)
        if (shared_font_texture) {
            if (ImGui_ImplOpenGL3_Data* _backend_data = static_cast<ImGui_ImplOpenGL3_Data*>(ImGui::GetIO().BackendRendererUserData)) {
                _backend_data->FontTexture = (GLuint)(uintptr_t)(shared_font_texture->imgui_texture());
            }
        }
#endif
        return _context;
    }

    void manager_window::reupload_shared_imgui_font_texture()
    {
        unsigned char* _pixels = nullptr;
        int _width, _height;
        shared_font_atlas->GetTexDataAsRGBA32(&_pixels, &_width, &_height);
        data_image _font_atlas_data = {};
        _font_atlas_data.channels = 4;
        _font_atlas_data.profile = data_image_profile::rgba8888;
        _font_atlas_data.width = _width;
        _font_atlas_data.height = _height;
        _font_atlas_data.pixels = std::vector<glm::uint8>(_pixels, _pixels + (_width * _height * 4));
        asset_image _font_atlas_image(std::move(_font_atlas_data));
        if (!shared_font_texture) {
            shared_font_texture.emplace(uint32x2(_width, _height));
        }
        shared_font_texture->update(_font_atlas_image);
        shared_font_atlas->SetTexID(shared_font_texture->imgui_texture());
#if defined(LUCARIA_BACKEND_PSPGU)
        ImGui_ImplPSP_UpdateFontsTexture(shared_font_atlas.get());
#endif
    }

    void manager_window::initialize_imgui()
    {
        IMGUI_CHECKVERSION();
        shared_font_atlas = std::make_unique<ImFontAtlas>();
        shared_font_atlas->AddFontDefault();
#if defined(LUCARIA_BACKEND_VULKAN)
        screen_context = create_shared_imgui_context();
        reupload_shared_imgui_font_texture();
#else
        reupload_shared_imgui_font_texture();
        screen_context = create_shared_imgui_context();
#endif
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::StyleColorsLight();
    }

    void manager_window::destroy_imgui()
    {
        if (screen_context == nullptr) {
            return;
        }
#if defined(LUCARIA_BACKEND_VULKAN)
        ImGui::SetCurrentContext(screen_context);
        rendering_vulkan_shutdown_imgui();
#endif
        ImGui::DestroyContext(screen_context);
        screen_context = nullptr;
    }

    void manager_window::initialize_openal()
    {
        ALCdevice* _webaudio_device = alcOpenDevice(NULL);
        LUCARIA_DEBUG_ASSERT(_webaudio_device, "Impossible to create an OpenAL device")
        ALCcontext* _webaudio_context = alcCreateContext(_webaudio_device, NULL);
        LUCARIA_DEBUG_ASSERT(_webaudio_context, "Impossible to create an OpenAL context")
        LUCARIA_DEBUG_ASSERT(alcMakeContextCurrent(_webaudio_context), "Impossible to make the OpenAL context current")
        LUCARIA_DEBUG_ASSERT(alIsExtensionPresent("AL_EXT_float32") == AL_TRUE, "OpenAL extension 'AL_EXT_float32' is not supported");
    }

    void manager_window::destroy_openal()
    {
        ALCcontext* _webaudio_context = alcGetCurrentContext();
        if (_webaudio_context == nullptr) {
            return;
        }
        ALCdevice* _webaudio_device = alcGetContextsDevice(_webaudio_context);
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(_webaudio_context);
        if (_webaudio_device != nullptr) {
            alcCloseDevice(_webaudio_device);
        }
    }

}
}
