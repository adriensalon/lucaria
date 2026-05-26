#include <AL/al.h>
#include <AL/alc.h>

#include <lucaria/core/manager_window.hpp>
#include <lucaria/core/utils_error.hpp>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <backends/imgui_impl_opengl3.h>
#include <lucaria/core/backend_opengl.hpp>
#endif

namespace lucaria {
namespace detail {

    ImGuiContext* manager_window::create_shared_imgui_context()
    {
        ImGuiContext* _context = ImGui::CreateContext(shared_font_atlas.get());
        ImGui::SetCurrentContext(_context);

#if defined(LUCARIA_PLATFORM_ANDROID)
        if (implementation_android.must_install_imgui_callbacks) {
            ImGui_ImplAndroid_Init(implementation_android.app->window);
            implementation_android.must_install_imgui_callbacks = false;
        }
#endif

#if defined(LUCARIA_PLATFORM_GLFW)
        if (must_install_imgui_callbacks) {
            ImGui_ImplGlfw_InitForOpenGL(window, true);
            must_install_imgui_callbacks = false;
        }
#endif

#if defined(LUCARIA_PLATFORM_PSP)
        if (implementation_psp.must_install_imgui_callbacks) {
            ImGui_ImplPSP_Init();
            implementation_psp.must_install_imgui_callbacks = false;
        }
#endif

#if defined(LUCARIA_BACKEND_OPENGL)
        ImGui_ImplOpenGL3_Init("#version 300 es");
        ImGui_ImplOpenGL3_DestroyFontsTexture();
#endif

        ImGui::GetIO().Fonts->SetTexID(shared_font_texture->imgui_texture());

#if defined(LUCARIA_BACKEND_OPENGL)
        struct ImGui_ImplOpenGL3_Data {
            GLuint GlVersion;
            char GlslVersionString[32];
            bool GlProfileIsES2;
            bool GlProfileIsES3;
            bool GlProfileIsCompat;
            GLint GlProfileMask;
            GLuint FontTexture;
            GLuint ShaderHandle;
            GLint AttribLocationTex; // Uniforms location
            GLint AttribLocationProjMtx;
            GLuint AttribLocationVtxPos; // Vertex attributes location
            GLuint AttribLocationVtxUV;
            GLuint AttribLocationVtxColor;
            unsigned int VboHandle, ElementsHandle;
            GLsizeiptr VertexBufferSize;
            GLsizeiptr IndexBufferSize;
            bool HasClipOrigin;
            bool UseBufferSubData;

            ImGui_ImplOpenGL3_Data() { memset((void*)this, 0, sizeof(*this)); }
        };

        if (ImGui_ImplOpenGL3_Data* _backend_data = static_cast<ImGui_ImplOpenGL3_Data*>(ImGui::GetIO().BackendRendererUserData)) {
            _backend_data->FontTexture = (GLuint)(uintptr_t)(shared_font_texture->imgui_texture());
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
        _font_atlas_data.profile = data_image_profile::binary;
        _font_atlas_data.width = _width;
        _font_atlas_data.height = _height;
        _font_atlas_data.pixels = std::vector<glm::uint8>(_pixels, _pixels + (_width * _height * 4));

        object_image _font_atlas_image(std::move(_font_atlas_data));

        if (!shared_font_texture) {
            shared_font_texture.emplace(_font_atlas_image);
        } else {
            shared_font_texture->update(_font_atlas_image);
        }

        shared_font_atlas->SetTexID(shared_font_texture->imgui_texture());
    }

    void manager_window::initialize_imgui()
    {
        IMGUI_CHECKVERSION();

        shared_font_atlas = std::make_unique<ImFontAtlas>();
        shared_font_atlas->AddFontDefault();

        reupload_shared_imgui_font_texture();
        screen_context = create_shared_imgui_context();

        ImGui::GetIO().IniFilename = nullptr;
        ImGui::StyleColorsLight();
    }

    void manager_window::destroy_imgui()
    {
        ImGui::DestroyContext(screen_context);
    }

    void manager_window::initialize_openal()
    {
        ALCdevice* _webaudio_device = alcOpenDevice(NULL);
        if (!_webaudio_device) {
            LUCARIA_DEBUG_ERROR("Impossible to create an OpenAL device");
        }

        ALCcontext* _webaudio_context = alcCreateContext(_webaudio_device, NULL);
        if (!_webaudio_context) {
            LUCARIA_DEBUG_ERROR("Impossible to create an OpenAL context");
        }

        if (!alcMakeContextCurrent(_webaudio_context)) {
            LUCARIA_DEBUG_ERROR("Impossible to use an OpenAL context");
        }

        bool _is_float32_supported = (alIsExtensionPresent("AL_EXT_float32") == AL_TRUE);
        if (!_is_float32_supported) {
            LUCARIA_DEBUG_ERROR("OpenAL extension 'AL_EXT_float32' is not supported");
        }
    }

    void manager_window::destroy_openal()
    {
        ALCcontext* _webaudio_context = alcGetCurrentContext();
        ALCdevice* _webaudio_device = alcGetContextsDevice(_webaudio_context);
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(_webaudio_context);
        alcCloseDevice(_webaudio_device);
    }

}
}