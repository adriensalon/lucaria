#if defined(LUCARIA_BACKEND_VULKAN)

#include <algorithm>
#include <cstring>

#include <backends/imgui_impl_vulkan.h>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/rendering_cubemap.hpp>
#include <lucaria/core/rendering_vulkan.hpp>

namespace lucaria {
namespace detail {
    namespace {

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

        void _create_cubemap_upload_buffer(const VkDeviceSize size, VkBuffer& buffer, VkDeviceMemory& memory)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();

            VkBufferCreateInfo _create = {};
            _create.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            _create.size = size;
            _create.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            _create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            if (vkCreateBuffer(_vulkan.device, &_create, nullptr, &buffer) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan cubemap upload buffer")
                return;
            }

            VkMemoryRequirements _requirements = {};
            vkGetBufferMemoryRequirements(_vulkan.device, buffer, &_requirements);

            VkMemoryAllocateInfo _allocate = {};
            _allocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            _allocate.allocationSize = _requirements.size;
            _allocate.memoryTypeIndex = rendering_vulkan_find_memory_type(_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            if (vkAllocateMemory(_vulkan.device, &_allocate, nullptr, &memory) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to allocate Vulkan cubemap upload buffer memory")
                return;
            }
            vkBindBufferMemory(_vulkan.device, buffer, memory, 0);
        }

        void _destroy_cubemap_upload_buffer(VkBuffer& buffer, VkDeviceMemory& memory)
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

        [[nodiscard]] VkFormat _cubemap_format(const data_image_profile profile)
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

        void _create_cubemap_image(rendering_cubemap& cubemap)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();

            VkImageCreateInfo _create = {};
            _create.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            _create.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            _create.imageType = VK_IMAGE_TYPE_2D;
            _create.format = cubemap.format;
            _create.extent = { cubemap.size.x, cubemap.size.y, 1 };
            _create.mipLevels = 1;
            _create.arrayLayers = 6;
            _create.samples = VK_SAMPLE_COUNT_1_BIT;
            _create.tiling = VK_IMAGE_TILING_OPTIMAL;
            _create.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            _create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            _create.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            if (vkCreateImage(_vulkan.device, &_create, nullptr, &cubemap.image) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan cubemap image")
                return;
            }

            VkMemoryRequirements _requirements = {};
            vkGetImageMemoryRequirements(_vulkan.device, cubemap.image, &_requirements);

            VkMemoryAllocateInfo _allocate = {};
            _allocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            _allocate.allocationSize = _requirements.size;
            _allocate.memoryTypeIndex = rendering_vulkan_find_memory_type(_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            if (vkAllocateMemory(_vulkan.device, &_allocate, nullptr, &cubemap.memory) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to allocate Vulkan cubemap image memory")
                return;
            }
            vkBindImageMemory(_vulkan.device, cubemap.image, cubemap.memory, 0);

            VkImageViewCreateInfo _view = {};
            _view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            _view.image = cubemap.image;
            _view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            _view.format = cubemap.format;
            _view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            _view.subresourceRange.baseMipLevel = 0;
            _view.subresourceRange.levelCount = 1;
            _view.subresourceRange.baseArrayLayer = 0;
            _view.subresourceRange.layerCount = 6;
            if (vkCreateImageView(_vulkan.device, &_view, nullptr, &cubemap.image_view) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan cubemap image view")
            }
            cubemap.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        }

        void _destroy_cubemap_image(rendering_cubemap& cubemap)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            if (_vulkan.device == VK_NULL_HANDLE) {
                cubemap.image = VK_NULL_HANDLE;
                cubemap.memory = VK_NULL_HANDLE;
                cubemap.image_view = VK_NULL_HANDLE;
                return;
            }
            if (cubemap.image_view != VK_NULL_HANDLE) {
                vkDestroyImageView(_vulkan.device, cubemap.image_view, nullptr);
                cubemap.image_view = VK_NULL_HANDLE;
            }
            if (cubemap.image != VK_NULL_HANDLE) {
                vkDestroyImage(_vulkan.device, cubemap.image, nullptr);
                cubemap.image = VK_NULL_HANDLE;
            }
            if (cubemap.memory != VK_NULL_HANDLE) {
                vkFreeMemory(_vulkan.device, cubemap.memory, nullptr);
                cubemap.memory = VK_NULL_HANDLE;
            }
        }

        void _create_cubemap_sampler(rendering_cubemap& cubemap)
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
            if (vkCreateSampler(_vulkan.device, &_create, nullptr, &cubemap.sampler) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan cubemap sampler")
            }
        }

        void _destroy_cubemap_sampler(rendering_cubemap& cubemap)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            if (_vulkan.device == VK_NULL_HANDLE) {
                cubemap.sampler = VK_NULL_HANDLE;
                return;
            }
            if (cubemap.sampler != VK_NULL_HANDLE) {
                vkDestroySampler(_vulkan.device, cubemap.sampler, nullptr);
                cubemap.sampler = VK_NULL_HANDLE;
            }
        }

        void _transition_cubemap_layout(rendering_cubemap& cubemap, const VkImageLayout new_layout)
        {
            if (cubemap.image == VK_NULL_HANDLE || cubemap.layout == new_layout) {
                return;
            }

            VkCommandBuffer _commands = rendering_vulkan_begin_upload_commands();
            if (_commands == VK_NULL_HANDLE) {
                return;
            }

            VkImageMemoryBarrier _barrier = {};
            _barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            _barrier.oldLayout = cubemap.layout;
            _barrier.newLayout = new_layout;
            _barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            _barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            _barrier.image = cubemap.image;
            _barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            _barrier.subresourceRange.baseMipLevel = 0;
            _barrier.subresourceRange.levelCount = 1;
            _barrier.subresourceRange.baseArrayLayer = 0;
            _barrier.subresourceRange.layerCount = 6;

            VkPipelineStageFlags _source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            VkPipelineStageFlags _destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            if (cubemap.layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                _barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                _barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                _source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                _destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else if (cubemap.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                _barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                _barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                _source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                _destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } else if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                _barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            }

            vkCmdPipelineBarrier(_commands, _source_stage, _destination_stage, 0, 0, nullptr, 0, nullptr, 1, &_barrier);
            rendering_vulkan_end_upload_commands(_commands);
            cubemap.layout = new_layout;
        }

        void _upload_cubemap_face(rendering_cubemap& cubemap, const uint32 layer, const uint32x2 size, const void* data, const VkDeviceSize size_bytes)
        {
            if (cubemap.image == VK_NULL_HANDLE || data == nullptr || size_bytes == 0) {
                return;
            }
            rendering_vulkan_context& _vulkan = rendering_vulkan();

            VkBuffer _staging_buffer = VK_NULL_HANDLE;
            VkDeviceMemory _staging_memory = VK_NULL_HANDLE;
            _create_cubemap_upload_buffer(size_bytes, _staging_buffer, _staging_memory);

            void* _mapped = nullptr;
            vkMapMemory(_vulkan.device, _staging_memory, 0, size_bytes, 0, &_mapped);
            std::memcpy(_mapped, data, static_cast<std::size_t>(size_bytes));
            vkUnmapMemory(_vulkan.device, _staging_memory);

            _transition_cubemap_layout(cubemap, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            VkCommandBuffer _commands = rendering_vulkan_begin_upload_commands();
            VkBufferImageCopy _copy = {};
            _copy.bufferOffset = 0;
            _copy.bufferRowLength = 0;
            _copy.bufferImageHeight = 0;
            _copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            _copy.imageSubresource.mipLevel = 0;
            _copy.imageSubresource.baseArrayLayer = layer;
            _copy.imageSubresource.layerCount = 1;
            _copy.imageExtent = { size.x, size.y, 1 };
            vkCmdCopyBufferToImage(_commands, _staging_buffer, cubemap.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &_copy);
            rendering_vulkan_end_upload_commands(_commands);

            _transition_cubemap_layout(cubemap, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            _destroy_cubemap_upload_buffer(_staging_buffer, _staging_memory);
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
                LUCARIA_DEBUG_ERROR("Cubemap pixel data is smaller than its dimensions and profile require")
                return {};
            }
            if (_source_bytes_per_pixel == _bytes_per_pixel) {
                return image.pixels;
            }
            if (image.profile != data_image_profile::rgba8888 || image.channels != 3) {
                LUCARIA_DEBUG_ERROR("Cubemap pixel data cannot be converted to its storage profile")
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

        void _upload_cubemap(rendering_cubemap& cubemap, const std::array<data_image, 6>& images)
        {
            constexpr std::array<uint32, 6> _vulkan_layer_to_lucaria_face = {
                0, // +X
                3, // -X
                1, // +Y
                4, // -Y
                2, // +Z
                5  // -Z
            };
            cubemap.profile = images[0].profile;
            cubemap.size = { images[0].width, images[0].height };
            cubemap.format = _cubemap_format(cubemap.profile);
            _create_cubemap_image(cubemap);
            _create_cubemap_sampler(cubemap);
            for (uint32 _layer = 0; _layer < 6; ++_layer) {
                const data_image& _image = images[_vulkan_layer_to_lucaria_face[_layer]];
                const std::vector<uint8> _pixels = _make_upload_pixels(_image);
                if (_pixels.empty()) {
                    continue;
                }
                _upload_cubemap_face(cubemap, _layer, { _image.width, _image.height }, _pixels.data(), static_cast<VkDeviceSize>(_pixels.size()));
            }
            cubemap.descriptor = _add_imgui_texture(cubemap.sampler, cubemap.image_view, cubemap.layout);
        }

    }

    rendering_cubemap::~rendering_cubemap()
    {
        if (_ownership.owns()) {
            _remove_imgui_texture(descriptor);
            _destroy_cubemap_sampler(*this);
            _destroy_cubemap_image(*this);
        }
    }

    rendering_cubemap::rendering_cubemap(const std::array<data_image, 6>& images)
    {
        _upload_cubemap(*this, images);
        _ownership.emplace();
    }

    rendering_cubemap::rendering_cubemap(const std::array<asset_image, 6>& images)
    {
        std::array<data_image, 6> _images = {};
        for (uint32 _index = 0; _index < 6; ++_index) {
            _images[_index] = images[_index].data;
        }
        _upload_cubemap(*this, _images);
        _ownership.emplace();
    }

}
}

#endif
