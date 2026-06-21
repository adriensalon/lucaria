#include <lucaria/core/object_texture.hpp>

namespace lucaria {
namespace detail {

	namespace {

		static int next_pow2(int v) 
		{
			int r = 1;
			while (r < v) r <<= 1;
			return r;
		}

		static void* gu_alloc_aligned(size_t bytes) 
		{
			// return memalign(16, bytes);
			return nullptr;
		}
	}

    asset_texture::asset_texture(asset_texture&& other)
    {
        implementation_pspgu.is_owning = false;
        *this = std::move(other);
    }

    asset_texture& asset_texture::operator=(asset_texture&& other)
    {
        if (implementation_pspgu.is_owning) {
            LUCARIA_DEBUG_ERROR("Object already owning resources")
        }

        implementation_pspgu.is_owning = other.implementation_pspgu.is_owning;
        // implementation_pspgu.id = other.implementation_pspgu.id;
        size = other.size;

        other.implementation_pspgu.is_owning = false;
        return *this;
    }

    asset_texture::~asset_texture()
    {
        if (implementation_pspgu.is_owning) {
            // glBindTexture(GL_TEXTURE_2D, 0);
            // glDeleteTextures(1, &implementation_pspgu.id);
        }
    }

    asset_texture::asset_texture(const asset_image& from)
    {
        
        implementation_pspgu.is_owning = true;
    }

    asset_texture::asset_texture(const uint32x2 size)
        : size(size)
    {
		implementation_pspgu.psm = GU_PSM_8888;
		implementation_pspgu.tbw = next_pow2(size.x);

		// const int h = next_pow2(size_.y);
		// const size_t bytes = implementation_pspgu.tbw * h * 4;
		// implementation_pspgu.pixels = gu_alloc_aligned(bytes);
		// std::memset(implementation_pspgu.pixels, 0, bytes);
		// sceKernelDcacheWritebackInvalidateAll();
		
        implementation_pspgu.is_owning = true;
    }

    void asset_texture::resize(const uint32x2 new_size)
    {
        if (size != new_size) {
            size = new_size;
			
        }
    }

    void asset_texture::update(const asset_image& image)
    {
		// if (image.size != size) {
			// resize(image.data.size);
		// }

		// Assuming image is RGBA8.
		for (uint32 y = 0; y < size.y; ++y) {
			// auto* dst = reinterpret_cast<uint32_t*>(implementation_pspgu.pixels) + y * implementation_pspgu.tbw;
			// auto* src = reinterpret_cast<const uint32_t*>(image.pixels) + y * size.x;
			// std::memcpy(dst, src, size.x * 4);
		}

		// sceKernelDcacheWritebackInvalidateAll();
    }

    ImTextureID asset_texture::imgui_texture() const
    {
        return reinterpret_cast<ImTextureID>(implementation_pspgu.pixels);
    }

}
}