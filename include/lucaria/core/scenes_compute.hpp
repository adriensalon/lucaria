#pragma once

#include <lucaria/core/rendering_backend.hpp>
#include <lucaria/core/scenes_entity.hpp>

namespace lucaria {
namespace detail {

    // #if !defined(LUCARIA_DISABLE_OPENGL_COMPUTE)
    //     template <typename T, typename Entity, std::uint32_t PageSize = 1024>
    //     struct storage_compute_allocator_opengl_ssbo {
    //         using value_type = T;
    //         using entity_type = Entity;
    //         static constexpr std::uint32_t page_size = PageSize;
    //         static constexpr std::uint32_t invalid_page = 0xffffffffu;
    //         struct allocation_type {
    //             std::uint32_t page = invalid_page;
    //             std::uint32_t base_index = 0;
    //         };
    //         GLuint ssbo = 0;
    //         std::uint32_t page_count = 0;
    //         std::uint32_t page_capacity = 0;
    //         std::vector<std::uint32_t> free_pages;
    //         allocation_type allocate_page()
    //         {
    //             std::uint32_t page = invalid_page;
    //             if (!free_pages.empty()) {
    //                 page = free_pages.back();
    //                 free_pages.pop_back();
    //             } else {
    //                 page = page_count++;
    //                 ensure_capacity(page_count);
    //             }
    //             return {
    //                 page,
    //                 page * page_size
    //             };
    //         }
    //         void release_page(allocation_type allocation)
    //         {
    //             if (allocation.page != invalid_page) {
    //                 free_pages.push_back(allocation.page);
    //             }
    //         }
    //         template <typename... Args>
    //         void construct(allocation_type allocation, std::uint32_t offset, Args&&... args)
    //         {
    //             T value { std::forward<Args>(args)... };
    //             glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    //             glBufferSubData(
    //                 GL_SHADER_STORAGE_BUFFER,
    //                 sizeof(T) * (allocation.base_index + offset),
    //                 sizeof(T),
    //                 &value);
    //         }
    //         void destroy(allocation_type, std::uint32_t)
    //         {
    //             // Usually no-op for GPU storage.
    //         }
    //         void move_construct(
    //             allocation_type dst_alloc,
    //             std::uint32_t dst_offset,
    //             allocation_type src_alloc,
    //             std::uint32_t src_offset)
    //         {
    //             glBindBuffer(GL_COPY_READ_BUFFER, ssbo);
    //             glBindBuffer(GL_COPY_WRITE_BUFFER, ssbo);
    //             glCopyBufferSubData(
    //                 GL_COPY_READ_BUFFER,
    //                 GL_COPY_WRITE_BUFFER,
    //                 sizeof(T) * (src_alloc.base_index + src_offset),
    //                 sizeof(T) * (dst_alloc.base_index + dst_offset),
    //                 sizeof(T));
    //         }
    //         std::uint32_t backend_index(allocation_type allocation, std::uint32_t offset) const
    //         {
    //             return allocation.base_index + offset;
    //         }
    //     private:
    //         void ensure_capacity(std::uint32_t required_pages)
    //         {
    //             if (required_pages <= page_capacity) {
    //                 return;
    //             }
    //             std::uint32_t new_capacity = page_capacity ? page_capacity * 2u : 8u;
    //             while (new_capacity < required_pages) {
    //                 new_capacity *= 2u;
    //             }
    //             const std::size_t new_size = sizeof(T) * new_capacity * page_size;
    //             if (!ssbo) {
    //                 glGenBuffers(1, &ssbo);
    //                 glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    //                 glBufferData(GL_SHADER_STORAGE_BUFFER, new_size, nullptr, GL_DYNAMIC_DRAW);
    //             } else {
    //                 // TODO: proper grow-copy path.
    //                 // For first version, over-reserve enough or recreate before data matters.
    //                 glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    //                 glBufferData(GL_SHADER_STORAGE_BUFFER, new_size, nullptr, GL_DYNAMIC_DRAW);
    //             }
    //             page_capacity = new_capacity;
    //         }
    //     };
    // #endif

#if !defined(LUCARIA_DISABLE_OPENGL_COMPUTE_TEXTURE)
    template <typename T, typename Entity, std::uint32_t PageSize = 1024>
    struct storage_compute_allocator_opengl_texture {
        using value_type = T;
        using entity_type = Entity;

        static constexpr std::uint32_t page_size = PageSize;
        static constexpr std::uint32_t invalid_page = 0xffffffffu;

        struct allocation_type {
            std::uint32_t page = invalid_page;
            std::uint32_t base_index = 0;
        };

        GLuint texture_a = 0;
        GLuint texture_b = 0; // ping-pong target if writable
        GLuint framebuffer = 0;

        std::uint32_t width = 0;
        std::uint32_t height = 0;
        std::uint32_t capacity = 0;

        std::uint32_t page_count = 0;
        std::vector<std::uint32_t> free_pages;

        allocation_type allocate_page()
        {
            std::uint32_t page = invalid_page;

            if (!free_pages.empty()) {
                page = free_pages.back();
                free_pages.pop_back();
            } else {
                page = page_count++;
                ensure_capacity(page_count * page_size);
            }

            return {
                page,
                page * page_size
            };
        }

        void release_page(allocation_type allocation)
        {
            if (allocation.page != invalid_page) {
                free_pages.push_back(allocation.page);
            }
        }

        template <typename... Args>
        void construct(allocation_type allocation, std::uint32_t offset, Args&&... args)
        {
            T value { std::forward<Args>(args)... };
            upload_element(allocation.base_index + offset, value);
        }

        void destroy(allocation_type, std::uint32_t)
        {
            // Usually no-op.
        }

        void move_construct(
            allocation_type dst_alloc,
            std::uint32_t dst_offset,
            allocation_type src_alloc,
            std::uint32_t src_offset)
        {
            // WebGL cannot copy arbitrary typed struct elements easily unless
            // you implement a copy pass shader or keep CPU shadow.
            // For first version, keep a CPU shadow or mark dirty and reupload page.
        }

        std::uint32_t backend_index(allocation_type allocation, std::uint32_t offset) const
        {
            return allocation.base_index + offset;
        }

        std::uint32_t x_from_index(std::uint32_t index) const
        {
            return index % width;
        }

        std::uint32_t y_from_index(std::uint32_t index) const
        {
            return index / width;
        }

    private:
        void ensure_capacity(std::uint32_t required_elements)
        {
            if (required_elements <= capacity) {
                return;
            }

            width = 1024;
            height = (required_elements + width - 1u) / width;
            capacity = width * height;

            // Allocate or reallocate floating/integer textures depending on T layout.
            // For first pass, restrict T to vec4-compatible payloads or generated packing.
        }

        void upload_element(std::uint32_t index, const T& value)
        {
            const std::uint32_t x = x_from_index(index);
            const std::uint32_t y = y_from_index(index);

            glBindTexture(GL_TEXTURE_2D, texture_a);

            // This only works if T maps directly to the texture format.
            // In practice, you need generated pack/unpack or CPU staging.
            glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                x,
                y,
                1,
                1,
                /* format */ GL_RGBA,
                /* type */ GL_FLOAT,
                &value);
        }
    };
#endif

}
}
