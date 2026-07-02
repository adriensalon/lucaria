#if defined(LUCARIA_BACKEND_VULKAN)

#include <algorithm>
#include <cstring>

#include <backends/imgui_impl_vulkan.h>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/rendering_texture.hpp>
#include <lucaria/core/rendering_vulkan.hpp>

namespace lucaria {
namespace detail {
    namespace {

        [[nodiscard]] bool _can_atlas_texture(const data_image& image)
        {
            switch (image.profile) {
            case data_image_profile::rgba8888:
            case data_image_profile::rgb565:
            case data_image_profile::rgba5551:
            case data_image_profile::rgba4444:
                return true;
            default:
                return false;
            }
        }

        [[nodiscard]] bool _is_compressed_texture(const data_image_profile profile)
        {
            switch (profile) {
            case data_image_profile::s3tc_rgb4:
            case data_image_profile::s3tc_rgba8:
            case data_image_profile::etc2_rgb4:
            case data_image_profile::etc2_rgba8:
                return true;
            default:
                return false;
            }
        }

        [[nodiscard]] uint32 _texture_bytes_per_pixel(const data_image& image)
        {
            switch (image.profile) {
            case data_image_profile::rgb565:
            case data_image_profile::rgba5551:
            case data_image_profile::rgba4444:
                return 2;
            default:
                return 4;
            }
        }

        [[nodiscard]] bool _imgui_vulkan_renderer_ready()
        {
            return ImGui::GetCurrentContext() != nullptr
                && ImGui::GetIO().BackendRendererUserData != nullptr;
        }

        [[nodiscard]] VkDescriptorSet _add_imgui_texture(const VkSampler sampler, const VkImageView image_view, const VkImageLayout image_layout)
        {
            if (sampler == VK_NULL_HANDLE || image_view == VK_NULL_HANDLE || !_imgui_vulkan_renderer_ready()) {
                return VK_NULL_HANDLE;
            }
            return ImGui_ImplVulkan_AddTexture(sampler, image_view, image_layout);
        }

        void _remove_imgui_texture(VkDescriptorSet& descriptor)
        {
            if (descriptor != VK_NULL_HANDLE) {
                if (_imgui_vulkan_renderer_ready()) {
                    ImGui_ImplVulkan_RemoveTexture(descriptor);
                }
                descriptor = VK_NULL_HANDLE;
            }
        }

        void _create_texture_upload_buffer(const VkDeviceSize size, VkBuffer& buffer, VkDeviceMemory& memory)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();

            VkBufferCreateInfo _create = {};
            _create.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            _create.size = size;
            _create.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            _create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            if (vkCreateBuffer(_vulkan.device, &_create, nullptr, &buffer) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan texture upload buffer")
                return;
            }

            VkMemoryRequirements _requirements = {};
            vkGetBufferMemoryRequirements(_vulkan.device, buffer, &_requirements);

            VkMemoryAllocateInfo _allocate = {};
            _allocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            _allocate.allocationSize = _requirements.size;
            _allocate.memoryTypeIndex = rendering_vulkan_find_memory_type(_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            if (vkAllocateMemory(_vulkan.device, &_allocate, nullptr, &memory) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to allocate Vulkan texture upload buffer memory")
                return;
            }
            vkBindBufferMemory(_vulkan.device, buffer, memory, 0);
        }

        void _destroy_texture_upload_buffer(VkBuffer& buffer, VkDeviceMemory& memory)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            if (_vulkan.device == VK_NULL_HANDLE) {
                buffer = VK_NULL_HANDLE;
                memory = VK_NULL_HANDLE;
                return;
            }
            if (buffer != VK_NULL_HANDLE) {
                vkDestroyBuffer(_vulkan.device, buffer, nullptr);
                buffer = VK_NULL_HANDLE;
            }
            if (memory != VK_NULL_HANDLE) {
                vkFreeMemory(_vulkan.device, memory, nullptr);
                memory = VK_NULL_HANDLE;
            }
        }

        [[nodiscard]] VkFormat _texture_format(const data_image_profile profile)
        {
            switch (profile) {
            case data_image_profile::rgb565:
                return VK_FORMAT_R5G6B5_UNORM_PACK16;
            case data_image_profile::rgba5551:
                return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
            case data_image_profile::rgba4444:
                return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
            case data_image_profile::s3tc_rgb4:
                return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
            case data_image_profile::s3tc_rgba8:
                return VK_FORMAT_BC3_UNORM_BLOCK;
            case data_image_profile::etc2_rgb4:
                return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
            case data_image_profile::etc2_rgba8:
                return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
            default:
                return VK_FORMAT_R8G8B8A8_UNORM;
            }
        }

        void _create_texture_image(const uint32x2 size, const VkFormat format, const VkImageUsageFlags usage, VkImage& image, VkDeviceMemory& memory, VkImageView& view, VkImageLayout& layout)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();

            VkImageCreateInfo _create = {};
            _create.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            _create.imageType = VK_IMAGE_TYPE_2D;
            _create.format = format;
            _create.extent = { size.x, size.y, 1 };
            _create.mipLevels = 1;
            _create.arrayLayers = 1;
            _create.samples = VK_SAMPLE_COUNT_1_BIT;
            _create.tiling = VK_IMAGE_TILING_OPTIMAL;
            _create.usage = usage;
            _create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            _create.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            if (vkCreateImage(_vulkan.device, &_create, nullptr, &image) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan texture image")
                return;
            }

            VkMemoryRequirements _requirements = {};
            vkGetImageMemoryRequirements(_vulkan.device, image, &_requirements);

            VkMemoryAllocateInfo _allocate = {};
            _allocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            _allocate.allocationSize = _requirements.size;
            _allocate.memoryTypeIndex = rendering_vulkan_find_memory_type(_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            if (vkAllocateMemory(_vulkan.device, &_allocate, nullptr, &memory) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to allocate Vulkan texture image memory")
                return;
            }
            vkBindImageMemory(_vulkan.device, image, memory, 0);

            VkImageViewCreateInfo _view = {};
            _view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            _view.image = image;
            _view.viewType = VK_IMAGE_VIEW_TYPE_2D;
            _view.format = format;
            _view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            _view.subresourceRange.baseMipLevel = 0;
            _view.subresourceRange.levelCount = 1;
            _view.subresourceRange.baseArrayLayer = 0;
            _view.subresourceRange.layerCount = 1;
            if (vkCreateImageView(_vulkan.device, &_view, nullptr, &view) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan texture image view")
            }
            layout = VK_IMAGE_LAYOUT_UNDEFINED;
        }

        void _destroy_texture_image(VkImage& image, VkDeviceMemory& memory, VkImageView& view)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            if (_vulkan.device == VK_NULL_HANDLE) {
                image = VK_NULL_HANDLE;
                memory = VK_NULL_HANDLE;
                view = VK_NULL_HANDLE;
                return;
            }
            if (view != VK_NULL_HANDLE) {
                vkDestroyImageView(_vulkan.device, view, nullptr);
                view = VK_NULL_HANDLE;
            }
            if (image != VK_NULL_HANDLE) {
                vkDestroyImage(_vulkan.device, image, nullptr);
                image = VK_NULL_HANDLE;
            }
            if (memory != VK_NULL_HANDLE) {
                vkFreeMemory(_vulkan.device, memory, nullptr);
                memory = VK_NULL_HANDLE;
            }
        }

        void _create_texture_sampler(VkSampler& sampler)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();

            VkSamplerCreateInfo _create = {};
            _create.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            _create.magFilter = VK_FILTER_LINEAR;
            _create.minFilter = VK_FILTER_LINEAR;
            _create.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            _create.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            _create.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            _create.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            _create.maxLod = 1.0f;
            if (vkCreateSampler(_vulkan.device, &_create, nullptr, &sampler) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan texture sampler")
            }
        }

        void _destroy_texture_sampler(VkSampler& sampler)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            if (_vulkan.device == VK_NULL_HANDLE) {
                sampler = VK_NULL_HANDLE;
                return;
            }
            if (sampler != VK_NULL_HANDLE) {
                vkDestroySampler(_vulkan.device, sampler, nullptr);
                sampler = VK_NULL_HANDLE;
            }
        }

        void _transition_texture_layout(rendering_texture& texture, const VkImageLayout new_layout)
        {
            if (texture.image == VK_NULL_HANDLE || texture.layout == new_layout) {
                return;
            }

            VkCommandBuffer _commands = rendering_vulkan_begin_upload_commands();
            if (_commands == VK_NULL_HANDLE) {
                return;
            }

            VkImageMemoryBarrier _barrier = {};
            _barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            _barrier.oldLayout = texture.layout;
            _barrier.newLayout = new_layout;
            _barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            _barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            _barrier.image = texture.image;
            _barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            _barrier.subresourceRange.baseMipLevel = 0;
            _barrier.subresourceRange.levelCount = 1;
            _barrier.subresourceRange.baseArrayLayer = 0;
            _barrier.subresourceRange.layerCount = 1;

            VkPipelineStageFlags _source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            VkPipelineStageFlags _destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            if (texture.layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                _barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                _barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                _source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                _destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else if (texture.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                _barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                _barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                _source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                _destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } else if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                _barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            }

            vkCmdPipelineBarrier(_commands, _source_stage, _destination_stage, 0, 0, nullptr, 0, nullptr, 1, &_barrier);
            rendering_vulkan_end_upload_commands(_commands);
            texture.layout = new_layout;
        }

        void _upload_texture_image(rendering_texture& texture, const void* data, const VkDeviceSize size_bytes)
        {
            if (texture.image == VK_NULL_HANDLE || data == nullptr || size_bytes == 0) {
                return;
            }
            rendering_vulkan_context& _vulkan = rendering_vulkan();

            VkBuffer _staging_buffer = VK_NULL_HANDLE;
            VkDeviceMemory _staging_memory = VK_NULL_HANDLE;
            _create_texture_upload_buffer(size_bytes, _staging_buffer, _staging_memory);

            void* _mapped = nullptr;
            vkMapMemory(_vulkan.device, _staging_memory, 0, size_bytes, 0, &_mapped);
            std::memcpy(_mapped, data, static_cast<std::size_t>(size_bytes));
            vkUnmapMemory(_vulkan.device, _staging_memory);

            _transition_texture_layout(texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            VkCommandBuffer _commands = rendering_vulkan_begin_upload_commands();
            VkBufferImageCopy _copy = {};
            _copy.bufferOffset = 0;
            _copy.bufferRowLength = 0;
            _copy.bufferImageHeight = 0;
            _copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            _copy.imageSubresource.mipLevel = 0;
            _copy.imageSubresource.baseArrayLayer = 0;
            _copy.imageSubresource.layerCount = 1;
            _copy.imageExtent = { texture.size.x, texture.size.y, 1 };
            vkCmdCopyBufferToImage(_commands, _staging_buffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &_copy);
            rendering_vulkan_end_upload_commands(_commands);

            _transition_texture_layout(texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            _destroy_texture_upload_buffer(_staging_buffer, _staging_memory);
        }

        [[nodiscard]] std::vector<uint8> _make_upload_pixels(const data_image& image)
        {
            if (_is_compressed_texture(image.profile)) {
                return image.pixels;
            }
            const uint32 _source_bytes_per_pixel = image.channels;
            const uint32 _bytes_per_pixel = _texture_bytes_per_pixel(image);
            const std::size_t _required_bytes = static_cast<std::size_t>(image.width) * image.height * _source_bytes_per_pixel;
            if (image.pixels.size() < _required_bytes) {
                LUCARIA_DEBUG_ERROR("Texture pixel data is smaller than its dimensions and profile require")
                return {};
            }
            if (_source_bytes_per_pixel == _bytes_per_pixel) {
                return image.pixels;
            }
            if (image.profile != data_image_profile::rgba8888 || image.channels != 3) {
                LUCARIA_DEBUG_ERROR("Texture pixel data cannot be converted to its storage profile")
                return {};
            }
            std::vector<uint8> _pixels(static_cast<std::size_t>(image.width) * image.height * 4);
            for (uint32 _index = 0; _index < image.width * image.height; ++_index) {
                const std::size_t _source = static_cast<std::size_t>(_index) * 3;
                const std::size_t _destination = static_cast<std::size_t>(_index) * 4;
                std::copy_n(image.pixels.data() + _source, 3, _pixels.data() + _destination);
                _pixels[_destination + 3] = 255;
            }
            return _pixels;
        }

        void _destroy_dedicated_texture(rendering_texture& texture)
        {
            _remove_imgui_texture(texture.imgui_descriptor);
            _destroy_texture_sampler(texture.sampler);
            _destroy_texture_image(texture.image, texture.memory, texture.image_view);
            texture.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        void _create_empty_dedicated_texture(rendering_texture& texture, const uint32x2 size)
        {
            texture.profile = data_image_profile::rgba8888;
            texture.size = size;
            texture.uv_rect = { 0.f, 1.f, 1.f, -1.f };
            texture.format = _texture_format(texture.profile);
            texture.is_dedicated_storage = true;
            _create_texture_image(size, texture.format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, texture.image, texture.memory, texture.image_view, texture.layout);
            _create_texture_sampler(texture.sampler);
        }

        void _upload_dedicated_texture(rendering_texture& texture, const data_image& image)
        {
            const std::vector<uint8> _pixels = _make_upload_pixels(image);
            if (_pixels.empty()) {
                LUCARIA_DEBUG_ERROR("Failed to prepare dedicated Vulkan texture")
                return;
            }
            _destroy_dedicated_texture(texture);
            texture.profile = image.profile;
            texture.size = { image.width, image.height };
            texture.format = _texture_format(image.profile);
            texture.is_dedicated_storage = true;
            texture.allocation = {};
            texture.uv_rect = { 0.f, 0.f, 1.f, 1.f };
            _create_texture_image(texture.size, texture.format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, texture.image, texture.memory, texture.image_view, texture.layout);
            _create_texture_sampler(texture.sampler);
            _upload_texture_image(texture, _pixels.data(), static_cast<VkDeviceSize>(_pixels.size()));
            texture.imgui_descriptor = _add_imgui_texture(texture.sampler, texture.image_view, texture.layout);
        }

    }

    rendering_texture::~rendering_texture()
    {
        _release();
    }

    void rendering_texture::_release() noexcept
    {
        if (!_ownership.owns()) {
            return;
        }
        if (!is_dedicated_storage && _registry != nullptr) {
            _registry->release(*this);
        } else if (is_dedicated_storage) {
            _destroy_dedicated_texture(*this);
        }
    }

    rendering_texture::rendering_texture(rendering_textures_registry& registry, const data_image& from)
        : profile(from.profile)
        , size(from.width, from.height)
    {
        if (_can_atlas_texture(from)) {
            is_dedicated_storage = false;
            _registry = &registry;
            _registry->upload(*this, from);
        } else {
            _upload_dedicated_texture(*this, from);
        }
        _ownership.emplace();
    }

    rendering_texture::rendering_texture(const uint32x2 size)
        : profile(data_image_profile::rgba8888)
        , size(size)
    {
        _create_empty_dedicated_texture(*this, size);
        is_dedicated_storage = true;
        _ownership.emplace();
    }

    void rendering_texture::resize(const uint32x2 new_size)
    {
        if (size == new_size) {
            return;
        }
        _release();
        _registry = nullptr;
        profile = data_image_profile::rgba8888;
        size = new_size;
        is_dedicated_storage = true;
        allocation = {};
        _create_empty_dedicated_texture(*this, new_size);
    }

    void rendering_texture::update(const data_image& from)
    {
        const bool _use_atlas = !is_dedicated_storage && _registry != nullptr && _can_atlas_texture(from);
        if (_use_atlas) {
            _release();
            profile = from.profile;
            size = { from.width, from.height };
            _registry->upload(*this, from);
            return;
        }
        if (!is_dedicated_storage) {
            _release();
            _registry = nullptr;
            image = VK_NULL_HANDLE;
            memory = VK_NULL_HANDLE;
            image_view = VK_NULL_HANDLE;
            sampler = VK_NULL_HANDLE;
            imgui_descriptor = VK_NULL_HANDLE;
        }
        _upload_dedicated_texture(*this, from);
    }

    ImTextureID rendering_texture::imgui_texture() const
    {
        return reinterpret_cast<ImTextureID>(imgui_descriptor);
    }

    ImVec2 rendering_texture::imgui_uv0() const
    {
        return { uv_rect.x, uv_rect.y };
    }

    ImVec2 rendering_texture::imgui_uv1() const
    {
        return { uv_rect.x + uv_rect.z, uv_rect.y + uv_rect.w };
    }

}
}

#endif
