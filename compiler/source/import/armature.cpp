#include <filesystem>

#include <assimp/importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cereal/archives/portable_binary.hpp>

#include <data/armature.hpp>

armature_data import_armature(const std::filesystem::path& gltf_path)
{
    armature_data _data;
    _data.count = 0;
    Assimp::Importer _importer;
    const aiScene* _scene = _importer.ReadFile(gltf_path.string(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
    if (!_scene) {
        std::cout << "Error importing armature file '" << gltf_path << "': " << _importer.GetErrorString() << std::endl;
        std::terminate();
    }
    if (_scene->mNumMeshes == 0) {
        std::cout << "No mesh found in armature file '" << gltf_path << "'. " << std::endl;
        std::terminate();
    }
    const aiMesh* _mesh = _scene->mMeshes[0];
    const aiMatrix4x4 _root_transform = _scene->mRootNode->mTransformation;
    std::vector<std::vector<glm::uint>> _vertex_bones(_mesh->mNumVertices);
    std::vector<std::vector<glm::float32>> _vertex_weights(_mesh->mNumVertices);
    for (glm::uint _b = 0; _b < _mesh->mNumBones; ++_b) {
        const aiBone* _bone = _mesh->mBones[_b];
        for (glm::uint w = 0; w < _bone->mNumWeights; ++w) {
            const glm::uint _vertex_id = _bone->mWeights[w].mVertexId;
            const glm::float32 _weight = _bone->mWeights[w].mWeight;
            _vertex_bones[_vertex_id].push_back(_b);
            _vertex_weights[_vertex_id].push_back(_weight);
        }
    }
    for (glm::uint _i = 0; _i < _mesh->mNumVertices; ++_i) {
        const aiVector3D _position = _root_transform * _mesh->mVertices[_i];
        _data.positions.push_back(glm::vec3(_position.x, _position.y, _position.z));
        _data.weights.insert(_data.weights.end(), _vertex_weights[_i].begin(), _vertex_weights[_i].end());
        _data.bones.insert(_data.bones.end(), _vertex_bones[_i].begin(), _vertex_bones[_i].end());
    }
    for (glm::uint _i = 0; _i < _mesh->mNumFaces; ++_i) {
        const aiFace& _face = _mesh->mFaces[_i];
        if (_face.mNumIndices != 3) {
            std::cout << "Non-triangle face encountered in gltf '" << gltf_path << "'. Only triangles are supported." << std::endl;
            std::terminate();
        }
        _data.count +=3;
    }
    std::cout << "   Exporting armature data binary..." << std::endl;
    return _data;
}
