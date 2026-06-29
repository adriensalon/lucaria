#include <lucaria/core/rendering_framebuffer.hpp>

namespace lucaria {
namespace detail {

    rendering_framebuffer::~rendering_framebuffer() = default;

    rendering_framebuffer::rendering_framebuffer()
    {
        ownership.emplace();
    }

    void rendering_framebuffer::use_default()
    {
        sceGuDrawBuffer(GU_PSM_8888, reinterpret_cast<void*>(0), 512);
        sceGuDepthBuffer(reinterpret_cast<void*>(2 * 512 * 272 * 4), 512);
    }

    void rendering_framebuffer::use()
    {
        if (has_color && color != nullptr) {
            sceGuDrawBufferList(psm, color, fbw);
        }
        if (has_depth && depth != nullptr) {
            sceGuDepthBuffer(depth, fbw);
        }
    }

    void rendering_framebuffer::bind_color(const rendering_texture& from)
    {
        color = from.pixels;
        size = from.size;
        psm = from.psm;
        fbw = from.tbw;
        has_color = color != nullptr;
    }

    void rendering_framebuffer::bind_color(rendering_renderbuffer& from)
    {
        color = from.pixels;
        size = from.size;
        psm = from.psm;
        fbw = from.fbw;
        has_color = color != nullptr;
    }

    void rendering_framebuffer::bind_depth(rendering_texture& from)
    {
        depth = from.pixels;
        has_depth = depth != nullptr;
    }

    void rendering_framebuffer::bind_depth(rendering_renderbuffer& from)
    {
        depth = from.pixels;
        has_depth = depth != nullptr;
    }

}
}
