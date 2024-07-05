#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <core/import.hpp>

armature_data import_armature(const std::filesystem::path& input)
{
    armature_data _data;
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
    aiMatrix4x4 rootTransform = scene->mRootNode->mTransformation;
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
        rotated_vertex = rootTransform * rotated_vertex;
        if (_extension == ".glb" || _extension == ".gltf") {
            rotated_vertex = gltf_rotationMatrix * rotated_vertex;
        }
        _data.positions.push_back(rotated_vertex.x);
        _data.positions.push_back(rotated_vertex.y);
        _data.positions.push_back(rotated_vertex.z);
        _data.bones.insert(_data.bones.end(), vertex_bones[i].begin(), vertex_bones[i].end());
        _data.weights.insert(_data.weights.end(), vertex_weights[i].begin(), vertex_weights[i].end());
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];
        if (face.mNumIndices != 3) {
            std::cout << "Non-triangle face encountered. Only triangles are supported." << std::endl;
            std::terminate();
        }
        _data.count += face.mNumIndices;
    }
    return _data;
}