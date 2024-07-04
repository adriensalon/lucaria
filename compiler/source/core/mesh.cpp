#include <filesystem>
#include <fstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>

#include <data/mesh.hpp>

/// @brief Imports a mesh as data
/// @param input the mesh path to load, containing text
/// @return the mesh data as a string
mesh_data import_mesh(const std::filesystem::path& input)
{
    mesh_data _data;
    _data.count = 0;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(input.string(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
    if (!scene) {
        std::cout << "Error importing mesh file: " << importer.GetErrorString() << std::endl;
        std::terminate();
    }
    if (scene->mNumMeshes == 0) {
        std::cout << "No meshes found in mesh file." << std::endl;
        std::terminate();
    }
    aiMesh* mesh = scene->mMeshes[0];
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        _data.positions.push_back(mesh->mVertices[i].x);
        _data.positions.push_back(mesh->mVertices[i].y);
        _data.positions.push_back(mesh->mVertices[i].z);
        if (mesh->mTextureCoords[0]) {
            _data.texcoords.push_back(mesh->mTextureCoords[0][i].x);
            _data.texcoords.push_back(mesh->mTextureCoords[0][i].y);
        }
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];
        if (face.mNumIndices != 3) {
            std::cout << "Non-triangle face encountered. Only triangles are supported." << std::endl;
            std::terminate();
        }
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            _data.indices.push_back(face.mIndices[j]);
            _data.count++;
        }
    }
    return _data;
}
