#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <malloc.h>

#include <pspkernel.h>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/rendering_renderbuffer.hpp>

namespace lucaria {
namespace detail {
    namespace {

        [[nodiscard]] static uint32 _next_pow2(uint32 value)
        {
            uint32 _result = 1;
            while (_result < value) {
                _result <<= 1;
            }
            return _result;
        }

        [[nodiscard]] static uint32 _bytes_per_pixel(const int psm)
        {
            switch (psm) {
            case GU_PSM_5650:
            case GU_PSM_5551:
            case GU_PSM_4444:
                return 2;
            default:
                return 4;
            }
        }

        static void _allocate(rendering_renderbuffer& renderbuffer)
        {
            if (renderbuffer.pixels != nullptr) {
                std::free(renderbuffer.pixels);
                renderbuffer.pixels = nullptr;
            }
            renderbuffer.fbw = static_cast<int>(_next_pow2(std::max<uint32>(renderbuffer.size.x, 1)));
            const uint32 _height = _next_pow2(std::max<uint32>(renderbuffer.size.y, 1));
            const std::size_t _bytes = static_cast<std::size_t>(renderbuffer.fbw) * _height * _bytes_per_pixel(renderbuffer.psm);
            renderbuffer.pixels = memalign(16, _bytes);
            if (renderbuffer.pixels == nullptr) {
                LUCARIA_DEBUG_ERROR("Failed to allocate PSP renderbuffer")
                return;
            }
            std::memset(renderbuffer.pixels, 0, _bytes);
            sceKernelDcacheWritebackInvalidateAll();
        }

    }

    rendering_renderbuffer::~rendering_renderbuffer()
    {
        if (ownership.owns() && pixels != nullptr) {
            std::free(pixels);
            pixels = nullptr;
        }
    }

    rendering_renderbuffer::rendering_renderbuffer(const uint32x2 from_size, const uint32 internal_format, const uint32 samples)
        : sampling_count(samples)
        , size(from_size)
    {
        psm = static_cast<int>(internal_format);
        _allocate(*this);
        ownership.emplace();
    }

    void rendering_renderbuffer::resize(const uint32x2 new_size)
    {
        if (size == new_size) {
            return;
        }
        size = new_size;
        _allocate(*this);
    }

}
}
