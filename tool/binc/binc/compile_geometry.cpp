#include <algorithm>
#include <exception>
#include <iostream>
#include <vector>

#include <meshoptimizer.h>

#include <binc/compile_geometry.hpp>

namespace detail {
namespace {

    [[nodiscard]] lucaria::uint32 vertices_count(const lucaria::data_geometry& geometry)
    {
        if (geometry.vertices_count != 0) {
            return geometry.vertices_count;
        }
        return static_cast<lucaria::uint32>(geometry.positions.size());
    }

    [[nodiscard]] std::vector<unsigned int> flatten_indices(const std::vector<lucaria::uint32x3>& indices)
    {
        std::vector<unsigned int> flat;
        flat.reserve(indices.size() * 3);
        for (const lucaria::uint32x3& index : indices) {
            flat.emplace_back(index.x);
            flat.emplace_back(index.y);
            flat.emplace_back(index.z);
        }
        return flat;
    }

    [[nodiscard]] std::vector<lucaria::uint32x3> make_triangle_indices(const std::vector<unsigned int>& indices)
    {
        std::vector<lucaria::uint32x3> triangles;
        triangles.reserve(indices.size() / 3);
        for (std::size_t index = 0; index + 2 < indices.size(); index += 3) {
            triangles.emplace_back(indices[index], indices[index + 1], indices[index + 2]);
        }
        return triangles;
    }

    template <typename value_t>
    void remap_attribute(
        std::vector<value_t>& values,
        const std::size_t old_vertices_count,
        const std::size_t new_vertices_count,
        const std::vector<unsigned int>& remap)
    {
        if (values.empty()) {
            return;
        }
        if (values.size() != old_vertices_count) {
            std::cout << "Geometry attribute count does not match vertex count" << std::endl;
            std::terminate();
        }

        std::vector<value_t> remapped(new_vertices_count);
        meshopt_remapVertexBuffer(remapped.data(), values.data(), old_vertices_count, sizeof(value_t), remap.data());
        values = std::move(remapped);
    }

    void apply_vertex_remap(lucaria::data_geometry& geometry, std::vector<unsigned int>& indices)
    {
        const std::size_t old_vertices_count = vertices_count(geometry);
        std::vector<unsigned int> remap(old_vertices_count);
        const std::size_t new_vertices_count = meshopt_optimizeVertexFetchRemap(remap.data(), indices.data(), indices.size(), old_vertices_count);

        meshopt_remapIndexBuffer(indices.data(), indices.data(), indices.size(), remap.data());
        remap_attribute(geometry.positions, old_vertices_count, new_vertices_count, remap);
        remap_attribute(geometry.colors, old_vertices_count, new_vertices_count, remap);
        remap_attribute(geometry.normals, old_vertices_count, new_vertices_count, remap);
        remap_attribute(geometry.tangents, old_vertices_count, new_vertices_count, remap);
        remap_attribute(geometry.bitangents, old_vertices_count, new_vertices_count, remap);
        remap_attribute(geometry.texcoords, old_vertices_count, new_vertices_count, remap);
        remap_attribute(geometry.bones, old_vertices_count, new_vertices_count, remap);
        remap_attribute(geometry.weights, old_vertices_count, new_vertices_count, remap);
        geometry.vertices_count = static_cast<lucaria::uint32>(new_vertices_count);
    }

    [[nodiscard]] std::size_t target_index_count(
        const std::size_t index_count,
        const std::size_t vertex_count,
        const std::size_t max_vertices,
        const float scale)
    {
        if (vertex_count == 0) {
            return index_count;
        }
        std::size_t target = static_cast<std::size_t>(static_cast<double>(index_count) * static_cast<double>(max_vertices) / static_cast<double>(vertex_count) * scale);
        target = std::max<std::size_t>(3, target);
        return target - (target % 3);
    }

    void simplify_to_vertex_limit(lucaria::data_geometry& geometry, std::vector<unsigned int>& indices, const lucaria::uint32 max_vertices)
    {
        if (max_vertices == 0 || vertices_count(geometry) <= max_vertices) {
            return;
        }
        if (geometry.positions.empty()) {
            std::cout << "Cannot simplify geometry without positions" << std::endl;
            std::terminate();
        }
        if (indices.empty()) {
            std::cout << "Cannot simplify geometry without triangle indices" << std::endl;
            std::terminate();
        }

        const lucaria::uint32 original_vertices_count = vertices_count(geometry);
        float target_error = 0.02f;
        for (int attempt = 0; attempt < 8 && vertices_count(geometry) > max_vertices; ++attempt) {
            const std::size_t current_vertices_count = vertices_count(geometry);
            const std::size_t target_indices = target_index_count(indices.size(), current_vertices_count, max_vertices, 0.92f);
            std::vector<unsigned int> simplified(indices.size());
            float result_error = 0.f;

            std::size_t simplified_count = meshopt_simplify(
                simplified.data(),
                indices.data(),
                indices.size(),
                &geometry.positions[0].x,
                current_vertices_count,
                sizeof(lucaria::float32x3),
                target_indices,
                target_error,
                0,
                &result_error);

            if (simplified_count >= indices.size()) {
                simplified_count = meshopt_simplifySloppy(
                    simplified.data(),
                    indices.data(),
                    indices.size(),
                    &geometry.positions[0].x,
                    current_vertices_count,
                    sizeof(lucaria::float32x3),
                    target_indices,
                    target_error,
                    &result_error);
            }

            simplified_count -= simplified_count % 3;
            if (simplified_count < 3 || simplified_count >= indices.size()) {
                target_error *= 2.f;
                continue;
            }

            simplified.resize(simplified_count);
            indices = std::move(simplified);
            apply_vertex_remap(geometry, indices);
            target_error = std::max(target_error * 1.5f, result_error * 1.5f);
        }

        if (vertices_count(geometry) > max_vertices) {
            std::cout << "Could not simplify geometry to platform vertex limit " << max_vertices << std::endl;
            std::terminate();
        }

        std::cout << "   Geometry lod0 vertices " << original_vertices_count << " -> " << vertices_count(geometry) << std::endl;
    }

    void optimize_indices_and_vertices(lucaria::data_geometry& geometry, std::vector<unsigned int>& indices)
    {
        if (indices.empty() || geometry.positions.empty()) {
            return;
        }

        const lucaria::uint32 current_vertices_count = vertices_count(geometry);
        meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), current_vertices_count);
        meshopt_optimizeOverdraw(
            indices.data(),
            indices.data(),
            indices.size(),
            &geometry.positions[0].x,
            current_vertices_count,
            sizeof(lucaria::float32x3),
            1.05f);
        apply_vertex_remap(geometry, indices);
    }

}

std::filesystem::path geometry_output_path(const std::filesystem::path& output_file)
{
    return output_file.parent_path() / (output_file.stem().string() + ".lod0.bin");
}

lucaria::data_geometry compile_geometry_lod0(lucaria::data_geometry geometry, const lucaria::uint32 max_vertices)
{
    geometry.vertices_count = vertices_count(geometry);
    std::vector<unsigned int> indices = flatten_indices(geometry.indices);
    simplify_to_vertex_limit(geometry, indices, max_vertices);
    optimize_indices_and_vertices(geometry, indices);
    geometry.indices = make_triangle_indices(indices);
    return geometry;
}

}
