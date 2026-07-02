#pragma once

#include <lucaria/core/rendering_texture.hpp>

namespace lucaria {
namespace detail {

    struct rendering_renderbuffer {
        rendering_renderbuffer() = delete;
        rendering_renderbuffer(const rendering_renderbuffer& other) = delete;
        rendering_renderbuffer& operator=(const rendering_renderbuffer& other) = delete;
        rendering_renderbuffer(rendering_renderbuffer&& other) = default;
        rendering_renderbuffer& operator=(rendering_renderbuffer&& other) = default;
        ~rendering_renderbuffer();

        rendering_renderbuffer(const uint32x2 size, const uint32 internal_format, const uint32 samples = 1);
        void resize(const uint32x2 new_size);

#if defined(LUCARIA_BACKEND_OPENGL)
		flag_owning ownership = {};
        GLuint id = 0;
        GLuint internal_format = 0;
#endif

#if defined(LUCARIA_BACKEND_VULKAN)
        flag_owning ownership = {};
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView image_view = VK_NULL_HANDLE;
        VkFormat internal_format = VK_FORMAT_D32_SFLOAT;
        VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        flag_owning ownership = {};
        void* pixels = nullptr;
		int psm = GU_PSM_8888;
		int fbw = 512;
#endif

        uint32 sampling_count;
        uint32x2 size;
    };

}
}
