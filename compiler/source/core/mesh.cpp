#include <filesystem>
#include <fstream>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <tiny_gltf.h>
#include <tiny_obj_loader.h>


#include <data/mesh.hpp>

namespace detail {

bool obj_parse(const std::filesystem::path& mesh_path, tinyobj::ObjReader& mesh_reader, const tinyobj::ObjReaderConfig& mesh_reader_config)
{
    return mesh_reader.ParseFromFile(mesh_path.generic_string(), mesh_reader_config);
}

void obj_verify(const tinyobj::ObjReader& mesh_reader)
{
    if (!mesh_reader.Error().empty()) {
        std::cout << mesh_reader.Error() << " while importing mesh" << std::endl;
        std::terminate();
    }
    if (!mesh_reader.Error().empty()) {
        std::cout << mesh_reader.Warning() << " while importing mesh" << std::endl;
        std::terminate();
    }
}

mesh_data import_obj(const std::filesystem::path& input)
{
    mesh_data _data;
    _data.count = 0;
    tinyobj::ObjReaderConfig _mesh_reader_config;
    tinyobj::ObjReader _mesh_reader;
    detail::obj_parse(input, _mesh_reader, _mesh_reader_config);
    detail::obj_verify(_mesh_reader);
    auto& _attributes = _mesh_reader.GetAttrib();
    auto& _shapes = _mesh_reader.GetShapes();
    for (std::size_t _s = 0; _s < _shapes.size(); _s++) {
        std::size_t _index_offset = 0;
        for (std::size_t _f = 0; _f < _shapes[_s].mesh.num_face_vertices.size(); _f++) { // loop over faces (polygon)
            std::size_t _face_vertices_count = std::size_t(_shapes[_s].mesh.num_face_vertices[_f]);
            if (_face_vertices_count != 3) {
                std::cerr << "Error while loading mesh because not triangle face" << std::endl;
                std::terminate();
            }
            for (std::size_t _v = 0; _v < _face_vertices_count; _v++) { // loop over vertices in the face
                tinyobj::index_t _index = _shapes[_s].mesh.indices[_index_offset + _v];
                _data.count++;

                // positions
                float _vx = _attributes.vertices[3 * std::size_t(_index.vertex_index) + 0];
                float _vy = _attributes.vertices[3 * std::size_t(_index.vertex_index) + 1];
                float _vz = _attributes.vertices[3 * std::size_t(_index.vertex_index) + 2];
                _data.positions.emplace_back(_vx);
                _data.positions.emplace_back(_vy);
                _data.positions.emplace_back(_vz);

                // uvs
                if (_index.texcoord_index >= 0) {
                    float _tx = _attributes.texcoords[2 * std::size_t(_index.texcoord_index) + 0];
                    float _ty = _attributes.texcoords[2 * std::size_t(_index.texcoord_index) + 1];
                    _data.texcoords.emplace_back(_tx);
                    _data.texcoords.emplace_back(1.f - _ty);
                }

                // indices
                _data.indices.emplace_back(static_cast<unsigned int>(_data.indices.size()));
            }
            _index_offset += _face_vertices_count;
        }
    }

    return _data;
}

mesh_data import_gltf(const std::filesystem::path& input)
{
    mesh_data _data;
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, input.string());
    if (!warn.empty()) {
        std::cout << "Warning: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cout << "Error: " << err << std::endl;
    }
    if (!ret) {
        std::cout << "Failed to load glTF file: " << input << std::endl;
        std::terminate();
    }
    _data.count = 0;
    if (model.meshes.empty()) {
        std::cout << "No meshes found in glTF file." << std::endl;
        std::terminate();
    }
    const tinygltf::Mesh& mesh = model.meshes[0];
    for (const tinygltf::Primitive& primitive : mesh.primitives) {
        if (primitive.mode != TINYGLTF_MODE_TRIANGLES) {
            std::cout << "Unsupported primitive mode. Only triangles are supported." << std::endl;
            std::terminate();
        }
        const tinygltf::Accessor& pos_accessor = model.accessors[primitive.attributes.find("POSITION")->second];
        const tinygltf::BufferView& pos_view = model.bufferViews[pos_accessor.bufferView];
        int pos_byte_stride = pos_accessor.ByteStride(pos_view);
        const tinygltf::Buffer& pos_buffer = model.buffers[pos_view.buffer];
        const float* pos_data = reinterpret_cast<const float*>(&pos_buffer.data[pos_view.byteOffset + pos_accessor.byteOffset]);

        // Access texcoord attribute if it exists
        const float* tex_data = nullptr;
        if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
            const tinygltf::Accessor& tex_accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
            const tinygltf::BufferView& tex_view = model.bufferViews[tex_accessor.bufferView];
            const tinygltf::Buffer& tex_buffer = model.buffers[tex_view.buffer];
            tex_data = reinterpret_cast<const float*>(&tex_buffer.data[tex_view.byteOffset + tex_accessor.byteOffset]);
        }

        // Access joint (bone) attribute if it exists
        const unsigned short* bone_data = nullptr;
        if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) {
            const tinygltf::Accessor& bone_accessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
            const tinygltf::BufferView& bone_view = model.bufferViews[bone_accessor.bufferView];
            const tinygltf::Buffer& bone_buffer = model.buffers[bone_view.buffer];
            bone_data = reinterpret_cast<const unsigned short*>(&bone_buffer.data[bone_view.byteOffset + bone_accessor.byteOffset]);
        }

        // Access weight attribute if it exists
        const float* weight_data = nullptr;
        if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end()) {
            const tinygltf::Accessor& weight_accessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
            const tinygltf::BufferView& weight_view = model.bufferViews[weight_accessor.bufferView];
            const tinygltf::Buffer& weight_buffer = model.buffers[weight_view.buffer];
            weight_data = reinterpret_cast<const float*>(&weight_buffer.data[weight_view.byteOffset + weight_accessor.byteOffset]);
        }

        // Access index data if it exists
        const unsigned short* index_data = nullptr;
        if (primitive.indices > -1) {
            const tinygltf::Accessor& index_accessor = model.accessors[primitive.indices];
            const tinygltf::BufferView& index_view = model.bufferViews[index_accessor.bufferView];
            const tinygltf::Buffer& index_buffer = model.buffers[index_view.buffer];
            index_data = reinterpret_cast<const unsigned short*>(&index_buffer.data[index_view.byteOffset + index_accessor.byteOffset]);
        }

        // Loop through the vertices
        for (size_t i = 0; i < pos_accessor.count; i++) {
            size_t index = index_data ? index_data[i] : i;

            // Positions
            _data.positions.push_back(pos_data[3 * index + 0]);
            _data.positions.push_back(pos_data[3 * index + 1]);
            _data.positions.push_back(pos_data[3 * index + 2]);

            // Texture coordinates
            if (tex_data) {
                _data.texcoords.push_back(tex_data[2 * index + 0]);
                _data.texcoords.push_back(tex_data[2 * index + 1]); // flip Y coordinate
                // _data.texcoords.push_back(1.0f - tex_data[2 * index + 1]); // flip Y coordinate
            }

            // Bones (joints)
            if (bone_data) {
                for (int j = 0; j < 4; ++j) { // Usually, there are 4 bones per vertex
                    _data.bones.push_back(bone_data[4 * index + j]);
                }
            }

            // Weights
            if (weight_data) {
                for (int j = 0; j < 4; ++j) { // Usually, there are 4 weights per vertex
                    _data.weights.push_back(weight_data[4 * index + j]);
                }
            }

            // Indices
            _data.indices.push_back(static_cast<unsigned int>(_data.indices.size()));

            _data.count++;
        }
    }

    return _data;
}

}

/// @brief Imports a mesh as data
/// @param input the mesh path to load, containing text
/// @return the mesh data as a string
mesh_data import_mesh(const std::filesystem::path& input)
{
    const std::string _extension = input.extension().string();
    if (_extension == ".obj") {
        return detail::import_obj(input);
    } else if (_extension == ".gltf" || _extension == ".glb") {
        return detail::import_gltf(input);
    } else {
        std::cout << "Implementation error : invalid mesh extension. Expected .obj, .gltf, .glb." << std::endl;
        std::terminate();
    }
    
}
