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
    aiMatrix4x4 gltf_rotationMatrix(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    const std::string _extension = input.extension().string();
    
    std::vector<std::vector<unsigned int>> vertex_bones(mesh->mNumVertices);
    std::vector<std::vector<float>> vertex_weights(mesh->mNumVertices);    
    for (unsigned int b = 0; b < mesh->mNumBones; ++b) {
        aiBone* bone = mesh->mBones[b];
        for (unsigned int w = 0; w < bone->mNumWeights; ++w) {
            unsigned int vertex_id = bone->mWeights[w].mVertexId;
            float weight = bone->mWeights[w].mWeight;
            vertex_bones[vertex_id].push_back(b);
            vertex_weights[vertex_id].push_back(weight);
        }
    }
    
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        aiVector3D rotated_vertex = mesh->mVertices[i];
        if (_extension == ".glb" || _extension == ".gltf") {
            rotated_vertex = gltf_rotationMatrix * rotated_vertex;
        }
        _data.positions.push_back(rotated_vertex.x);
        _data.positions.push_back(rotated_vertex.y);
        _data.positions.push_back(rotated_vertex.z);     

        if (mesh->HasVertexColors(0)) {
            aiColor4D color = mesh->mColors[0][i];
            _data.colors.push_back(color.r);
            _data.colors.push_back(color.g);
            _data.colors.push_back(color.b);
            _data.colors.push_back(color.a);
        }

        if (mesh->HasNormals()) {
            aiVector3D normal = mesh->mNormals[i];
            _data.normals.push_back(normal.x);
            _data.normals.push_back(normal.y);
            _data.normals.push_back(normal.z);
        }

        if (mesh->HasTangentsAndBitangents()) {
            aiVector3D tangent = mesh->mTangents[i];
            aiVector3D bitangent = mesh->mBitangents[i];
            _data.tangents.push_back(tangent.x);
            _data.tangents.push_back(tangent.y);
            _data.tangents.push_back(tangent.z);
            _data.bitangents.push_back(bitangent.x);
            _data.bitangents.push_back(bitangent.y);
            _data.bitangents.push_back(bitangent.z);
        }   

        if (mesh->mTextureCoords[0]) {
            _data.texcoords.push_back(mesh->mTextureCoords[0][i].x);
            _data.texcoords.push_back(1.f - mesh->mTextureCoords[0][i].y);
        }

        // Store bone indices and weights for the current vertex
        _data.bones.insert(_data.bones.end(), vertex_bones[i].begin(), vertex_bones[i].end());
        _data.weights.insert(_data.weights.end(), vertex_weights[i].begin(), vertex_weights[i].end());
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
