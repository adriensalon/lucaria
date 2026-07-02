#if defined(LUCARIA_BACKEND_VULKAN)

#include <algorithm>
#include <cstring>

#include <backends/imgui_impl_vulkan.h>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/rendering_mesh.hpp>
#include <lucaria/core/rendering_storage.hpp>
#include <lucaria/core/rendering_texture.hpp>
#include <lucaria/core/rendering_vulkan.hpp>

namespace lucaria {
namespace detail {
    namespace {

        constexpr uint32 _default_vertex_page_size = 64u * 1024u * 1024u;
        constexpr uint32 _default_element_page_size = 16u * 1024u * 1024u;
        constexpr uint32 _buffer_page_alignment = 256u;
        constexpr uint32 _default_texture_page_width = 4096;
        constexpr uint32 _default_texture_page_height = 4096;
        constexpr uint32 _texture_gutter = 1;

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

        [[nodiscard]] uint32 _texture_source_bytes_per_pixel(const data_image& image)
        {
            switch (image.profile) {
            case data_image_profile::rgb565:
            case data_image_profile::rgba5551:
            case data_image_profile::rgba4444:
                return 2;
            default:
                return image.channels;
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

        void _create_storage_buffer(const VkDeviceSize size, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();

            VkBufferCreateInfo _create = {};
            _create.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            _create.size = size;
            _create.usage = usage;
            _create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            if (vkCreateBuffer(_vulkan.device, &_create, nullptr, &buffer) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan storage buffer")
                return;
            }

            VkMemoryRequirements _requirements = {};
            vkGetBufferMemoryRequirements(_vulkan.device, buffer, &_requirements);

            VkMemoryAllocateInfo _allocate = {};
            _allocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            _allocate.allocationSize = _requirements.size;
            _allocate.memoryTypeIndex = rendering_vulkan_find_memory_type(_requirements.memoryTypeBits, properties);
            if (vkAllocateMemory(_vulkan.device, &_allocate, nullptr, &memory) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to allocate Vulkan storage buffer memory")
                return;
            }
            vkBindBufferMemory(_vulkan.device, buffer, memory, 0);
        }

        void _destroy_storage_buffer(VkBuffer& buffer, VkDeviceMemory& memory)
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

        void _upload_storage_buffer(const VkBuffer destination, const VkDeviceSize destination_offset, const void* data, const VkDeviceSize size)
        {
            if (destination == VK_NULL_HANDLE || data == nullptr || size == 0) {
                return;
            }
            rendering_vulkan_context& _vulkan = rendering_vulkan();

            VkBuffer _staging_buffer = VK_NULL_HANDLE;
            VkDeviceMemory _staging_memory = VK_NULL_HANDLE;
            _create_storage_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _staging_buffer, _staging_memory);

            void* _mapped = nullptr;
            vkMapMemory(_vulkan.device, _staging_memory, 0, size, 0, &_mapped);
            std::memcpy(_mapped, data, static_cast<std::size_t>(size));
            vkUnmapMemory(_vulkan.device, _staging_memory);

            VkCommandBuffer _commands = rendering_vulkan_begin_upload_commands();
            VkBufferCopy _copy = {};
            _copy.srcOffset = 0;
            _copy.dstOffset = destination_offset;
            _copy.size = size;
            vkCmdCopyBuffer(_commands, _staging_buffer, destination, 1, &_copy);
            rendering_vulkan_end_upload_commands(_commands);

            _destroy_storage_buffer(_staging_buffer, _staging_memory);
        }

        [[nodiscard]] VkFormat _texture_page_format(const data_image_profile profile)
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

        void _create_texture_page_image(rendering_textures_page& page)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();

            VkImageCreateInfo _create = {};
            _create.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            _create.imageType = VK_IMAGE_TYPE_2D;
            _create.format = page.format;
            _create.extent = { page.capacity.x, page.capacity.y, 1 };
            _create.mipLevels = 1;
            _create.arrayLayers = 1;
            _create.samples = VK_SAMPLE_COUNT_1_BIT;
            _create.tiling = VK_IMAGE_TILING_OPTIMAL;
            _create.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            _create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            _create.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            if (vkCreateImage(_vulkan.device, &_create, nullptr, &page.image) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan texture page image")
                return;
            }

            VkMemoryRequirements _requirements = {};
            vkGetImageMemoryRequirements(_vulkan.device, page.image, &_requirements);

            VkMemoryAllocateInfo _allocate = {};
            _allocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            _allocate.allocationSize = _requirements.size;
            _allocate.memoryTypeIndex = rendering_vulkan_find_memory_type(_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            if (vkAllocateMemory(_vulkan.device, &_allocate, nullptr, &page.memory) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to allocate Vulkan texture page image memory")
                return;
            }
            vkBindImageMemory(_vulkan.device, page.image, page.memory, 0);

            VkImageViewCreateInfo _view = {};
            _view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            _view.image = page.image;
            _view.viewType = VK_IMAGE_VIEW_TYPE_2D;
            _view.format = page.format;
            _view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            _view.subresourceRange.baseMipLevel = 0;
            _view.subresourceRange.levelCount = 1;
            _view.subresourceRange.baseArrayLayer = 0;
            _view.subresourceRange.layerCount = 1;
            if (vkCreateImageView(_vulkan.device, &_view, nullptr, &page.image_view) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan texture page image view")
            }
            page.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        }

        void _destroy_texture_page_image(rendering_textures_page& page)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            if (_vulkan.device == VK_NULL_HANDLE) {
                page.image = VK_NULL_HANDLE;
                page.memory = VK_NULL_HANDLE;
                page.image_view = VK_NULL_HANDLE;
                return;
            }
            if (page.image_view != VK_NULL_HANDLE) {
                vkDestroyImageView(_vulkan.device, page.image_view, nullptr);
                page.image_view = VK_NULL_HANDLE;
            }
            if (page.image != VK_NULL_HANDLE) {
                vkDestroyImage(_vulkan.device, page.image, nullptr);
                page.image = VK_NULL_HANDLE;
            }
            if (page.memory != VK_NULL_HANDLE) {
                vkFreeMemory(_vulkan.device, page.memory, nullptr);
                page.memory = VK_NULL_HANDLE;
            }
        }

        void _create_texture_page_sampler(rendering_textures_page& page)
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
            if (vkCreateSampler(_vulkan.device, &_create, nullptr, &page.sampler) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan texture page sampler")
            }
        }

        void _destroy_texture_page_sampler(rendering_textures_page& page)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            if (_vulkan.device == VK_NULL_HANDLE) {
                page.sampler = VK_NULL_HANDLE;
                return;
            }
            if (page.sampler != VK_NULL_HANDLE) {
                vkDestroySampler(_vulkan.device, page.sampler, nullptr);
                page.sampler = VK_NULL_HANDLE;
            }
        }

        void _transition_texture_page_layout(rendering_textures_page& page, const VkImageLayout new_layout)
        {
            if (page.image == VK_NULL_HANDLE || page.layout == new_layout) {
                return;
            }

            VkCommandBuffer _commands = rendering_vulkan_begin_upload_commands();
            if (_commands == VK_NULL_HANDLE) {
                return;
            }

            VkImageMemoryBarrier _barrier = {};
            _barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            _barrier.oldLayout = page.layout;
            _barrier.newLayout = new_layout;
            _barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            _barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            _barrier.image = page.image;
            _barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            _barrier.subresourceRange.baseMipLevel = 0;
            _barrier.subresourceRange.levelCount = 1;
            _barrier.subresourceRange.baseArrayLayer = 0;
            _barrier.subresourceRange.layerCount = 1;

            VkPipelineStageFlags _source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            VkPipelineStageFlags _destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            if (page.layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                _barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                _barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                _source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                _destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else if (page.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                _barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                _barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                _source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                _destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } else if (new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                _barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            }

            vkCmdPipelineBarrier(_commands, _source_stage, _destination_stage, 0, 0, nullptr, 0, nullptr, 1, &_barrier);
            rendering_vulkan_end_upload_commands(_commands);
            page.layout = new_layout;
        }

        void _upload_texture_page_region(rendering_textures_page& page, const uint32x2 offset, const uint32x2 size, const void* data, const VkDeviceSize size_bytes)
        {
            if (page.image == VK_NULL_HANDLE || data == nullptr || size_bytes == 0) {
                return;
            }
            rendering_vulkan_context& _vulkan = rendering_vulkan();

            VkBuffer _staging_buffer = VK_NULL_HANDLE;
            VkDeviceMemory _staging_memory = VK_NULL_HANDLE;
            _create_storage_buffer(size_bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _staging_buffer, _staging_memory);

            void* _mapped = nullptr;
            vkMapMemory(_vulkan.device, _staging_memory, 0, size_bytes, 0, &_mapped);
            std::memcpy(_mapped, data, static_cast<std::size_t>(size_bytes));
            vkUnmapMemory(_vulkan.device, _staging_memory);

            _transition_texture_page_layout(page, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            VkCommandBuffer _commands = rendering_vulkan_begin_upload_commands();
            VkBufferImageCopy _copy = {};
            _copy.bufferOffset = 0;
            _copy.bufferRowLength = 0;
            _copy.bufferImageHeight = 0;
            _copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            _copy.imageSubresource.mipLevel = 0;
            _copy.imageSubresource.baseArrayLayer = 0;
            _copy.imageSubresource.layerCount = 1;
            _copy.imageOffset = { static_cast<int32>(offset.x), static_cast<int32>(offset.y), 0 };
            _copy.imageExtent = { size.x, size.y, 1 };
            vkCmdCopyBufferToImage(_commands, _staging_buffer, page.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &_copy);
            rendering_vulkan_end_upload_commands(_commands);

            _transition_texture_page_layout(page, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            _destroy_storage_buffer(_staging_buffer, _staging_memory);
        }

        [[nodiscard]] std::vector<uint8> _make_padded_pixels(const data_image& image)
        {
            const uint32 _bytes_per_pixel = _texture_bytes_per_pixel(image);
            const uint32 _source_bytes_per_pixel = _texture_source_bytes_per_pixel(image);
            if (image.width == 0 || image.height == 0 || _bytes_per_pixel == 0 || _source_bytes_per_pixel == 0) {
                return {};
            }
            const std::size_t _required_bytes = static_cast<std::size_t>(image.width) * image.height * _source_bytes_per_pixel;
            if (image.pixels.size() < _required_bytes) {
                LUCARIA_DEBUG_ERROR("Texture pixel data is smaller than its dimensions and profile require")
                return {};
            }
            if (_source_bytes_per_pixel != _bytes_per_pixel && !(image.profile == data_image_profile::rgba8888 && image.channels == 3)) {
                LUCARIA_DEBUG_ERROR("Texture pixel data cannot be converted to its storage profile")
                return {};
            }
            const uint32 _padded_width = image.width + _texture_gutter * 2;
            const uint32 _padded_height = image.height + _texture_gutter * 2;
            std::vector<uint8> _padded(static_cast<std::size_t>(_padded_width) * _padded_height * _bytes_per_pixel);
            for (uint32 _y = 0; _y < _padded_height; ++_y) {
                const uint32 _source_y = std::min(std::max(_y, _texture_gutter) - _texture_gutter, image.height - 1);
                for (uint32 _x = 0; _x < _padded_width; ++_x) {
                    const uint32 _source_x = std::min(std::max(_x, _texture_gutter) - _texture_gutter, image.width - 1);
                    const std::size_t _source_offset = (static_cast<std::size_t>(_source_y) * image.width + _source_x) * _source_bytes_per_pixel;
                    const std::size_t _destination_offset = (static_cast<std::size_t>(_y) * _padded_width + _x) * _bytes_per_pixel;
                    if (_source_bytes_per_pixel == _bytes_per_pixel) {
                        std::copy_n(image.pixels.data() + _source_offset, _bytes_per_pixel, _padded.data() + _destination_offset);
                    } else {
                        std::copy_n(image.pixels.data() + _source_offset, _source_bytes_per_pixel, _padded.data() + _destination_offset);
                        _padded[_destination_offset + 3] = 255;
                    }
                }
            }
            return _padded;
        }

        [[nodiscard]] rendering_meshes_page _create_geometry_page(const uint32 required_vertex_size, const uint32 required_element_size)
        {
            rendering_meshes_page _page = {};
            _page.vertex_capacity = rendering_meshes_align_up(std::max(required_vertex_size, _default_vertex_page_size), _buffer_page_alignment);
            _page.element_capacity = rendering_meshes_align_up(std::max(required_element_size, _default_element_page_size), _buffer_page_alignment);
            _create_storage_buffer(_page.vertex_capacity, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _page.vertices_buffer, _page.vertices_memory);
            _create_storage_buffer(_page.element_capacity, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _page.elements_buffer, _page.elements_memory);
            _page.free_vertices.free({ 0, _page.vertex_capacity });
            _page.free_elements.free({ 0, _page.element_capacity });
            return _page;
        }

        [[nodiscard]] rendering_textures_page _create_texture_page(const data_image_profile profile, const uint32x2 required_size)
        {
            rendering_textures_page _page = {};
            _page.profile = profile;
            _page.capacity = { std::max(required_size.x, _default_texture_page_width), std::max(required_size.y, _default_texture_page_height) };
            _page.format = _texture_page_format(profile);
            _create_texture_page_image(_page);
            _create_texture_page_sampler(_page);
            _page.free_pixels.push_back({ { 0, 0 }, _page.capacity });
            return _page;
        }

    }

    rendering_meshes_registry::~rendering_meshes_registry()
    {
        for (std::pair<const data_geometry_profile, rendering_meshes_buffer>& _pair : buffers) {
            rendering_meshes_buffer& _pool = _pair.second;
            for (rendering_meshes_page& _page : _pool.pages) {
                _destroy_storage_buffer(_page.vertices_buffer, _page.vertices_memory);
                _destroy_storage_buffer(_page.elements_buffer, _page.elements_memory);
            }
        }
    }

    void rendering_meshes_registry::upload(rendering_mesh& mesh, const std::vector<uint8>& vertices, const std::vector<uint32>& elements)
    {
        const uint32 _vertex_size = static_cast<uint32>(vertices.size());
        const uint32 _element_size = static_cast<uint32>(elements.size() * sizeof(uint32));
        rendering_meshes_buffer& _pool = assure_buffer(mesh.profile);
        std::optional<rendering_mesh_allocation> _allocation = _pool.allocate(_vertex_size, _element_size, _buffer_page_alignment, sizeof(uint32));
        if (!_allocation) {
            _pool.pages.push_back(_create_geometry_page(_vertex_size, _element_size));
            _allocation = _pool.allocate(_vertex_size, _element_size, _buffer_page_alignment, sizeof(uint32));
        }
        LUCARIA_DEBUG_ASSERT(_allocation, "Failed to allocate rendering mesh storage")
        if (!_allocation) {
            return;
        }
        rendering_meshes_page& _page = _pool.pages[_allocation->page];
        _upload_storage_buffer(_page.vertices_buffer, _allocation->vertices.offset, vertices.data(), _allocation->vertices.size);
        _upload_storage_buffer(_page.elements_buffer, _allocation->elements.offset, elements.data(), _allocation->elements.size);
        mesh.vertices_buffer = _page.vertices_buffer;
        mesh.elements_buffer = _page.elements_buffer;
        mesh.allocation = *_allocation;
    }

    void rendering_meshes_registry::release(rendering_mesh& mesh)
    {
        rendering_meshes_buffer* _pool = find_buffer(mesh.profile);
        if (_pool != nullptr) {
            _pool->free(mesh.allocation);
        }
        mesh.allocation = {};
        mesh.vertices_buffer = VK_NULL_HANDLE;
        mesh.elements_buffer = VK_NULL_HANDLE;
    }

    rendering_textures_registry::~rendering_textures_registry()
    {
        for (std::pair<const data_image_profile, rendering_textures_buffer>& _pair : buffers) {
            rendering_textures_buffer& _buffer = _pair.second;
            for (rendering_textures_page& _page : _buffer.pages) {
                _remove_imgui_texture(_page.descriptor);
                _destroy_texture_page_sampler(_page);
                _destroy_texture_page_image(_page);
            }
        }
    }

    void rendering_textures_registry::upload(rendering_texture& texture, const data_image& image)
    {
        const uint32x2 _size = { image.width, image.height };
        const uint32x2 _padded_size = { _size.x + _texture_gutter * 2, _size.y + _texture_gutter * 2 };
        const std::vector<uint8> _padded_pixels = _make_padded_pixels(image);
        if (_padded_pixels.empty()) {
            LUCARIA_DEBUG_ERROR("Failed to prepare rendering texture storage")
            return;
        }
        rendering_textures_buffer& _buffer = assure_buffer(image.profile);
        std::optional<rendering_texture_allocation> _allocation = _buffer.allocate(_padded_size);
        if (!_allocation) {
            _buffer.pages.push_back(_create_texture_page(image.profile, _padded_size));
            _allocation = _buffer.allocate(_padded_size);
        }
        LUCARIA_DEBUG_ASSERT(_allocation, "Failed to allocate rendering texture storage")
        if (!_allocation) {
            return;
        }
        rendering_textures_page& _page = _buffer.pages[_allocation->page];
        _upload_texture_page_region(_page, _allocation->pixels.offset, _padded_size, _padded_pixels.data(), static_cast<VkDeviceSize>(_padded_pixels.size()));
        if (_page.descriptor == VK_NULL_HANDLE) {
            _page.descriptor = _add_imgui_texture(_page.sampler, _page.image_view, _page.layout);
        }
        texture.profile = image.profile;
        texture.size = _size;
        texture.allocation = *_allocation;
        texture.image = _page.image;
        texture.image_view = _page.image_view;
        texture.sampler = _page.sampler;
        texture.imgui_descriptor = _page.descriptor;
        texture.format = _page.format;
        texture.layout = _page.layout;
        texture.uv_rect = {
            static_cast<float32>(_allocation->pixels.offset.x + _texture_gutter) / static_cast<float32>(_page.capacity.x),
            static_cast<float32>(_allocation->pixels.offset.y + _texture_gutter) / static_cast<float32>(_page.capacity.y),
            static_cast<float32>(_size.x) / static_cast<float32>(_page.capacity.x),
            static_cast<float32>(_size.y) / static_cast<float32>(_page.capacity.y)
        };
    }

    void rendering_textures_registry::release(rendering_texture& texture)
    {
        rendering_textures_buffer* _buffer = find_buffer(texture.profile);
        if (_buffer != nullptr) {
            _buffer->free(texture.allocation);
        }
        texture.allocation = {};
        texture.image = VK_NULL_HANDLE;
        texture.memory = VK_NULL_HANDLE;
        texture.image_view = VK_NULL_HANDLE;
        texture.sampler = VK_NULL_HANDLE;
        texture.imgui_descriptor = VK_NULL_HANDLE;
        texture.size = {};
        texture.uv_rect = { 0.f, 0.f, 1.f, 1.f };
    }

}
}

#endif
