#include <filesystem>
#include <fstream>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
// #include <tiny_gltf.h>
#include <tiny_obj_loader.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


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

mesh_data import_assimp(const std::filesystem::path& input) {
    mesh_data _data;
    _data.count = 0;

    Assimp::Importer importer;

    // Import the GLTF file using Assimp
    const aiScene* scene = importer.ReadFile(input.string(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

    if (!scene) {
        std::cerr << "Error importing GLTF file: " << importer.GetErrorString() << std::endl;
        std::terminate();
    }

    // Assimp can load multiple meshes; we'll handle only the first mesh for simplicity
    if (scene->mNumMeshes == 0) {
        std::cerr << "No meshes found in GLTF file." << std::endl;
        std::terminate();
    }

    aiMesh* mesh = scene->mMeshes[0];

    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        // Positions
        _data.positions.push_back(mesh->mVertices[i].x);
        _data.positions.push_back(mesh->mVertices[i].y);
        _data.positions.push_back(mesh->mVertices[i].z);

        // Texture coordinates (only taking the first set)
        if (mesh->mTextureCoords[0]) {
            _data.texcoords.push_back(mesh->mTextureCoords[0][i].x);
            _data.texcoords.push_back(1.0f - mesh->mTextureCoords[0][i].y); // Optionally flip Y coordinate
        }
    }

    // Process faces (triangles)
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];
        if (face.mNumIndices != 3) {
            std::cerr << "Non-triangle face encountered. Only triangles are supported." << std::endl;
            std::terminate();
        }

        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            _data.indices.push_back(face.mIndices[j]);
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
    } else {
        return detail::import_assimp(input);
    }
    
}
