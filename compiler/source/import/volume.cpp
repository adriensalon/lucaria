#include <filesystem>

#include <assimp/importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cereal/archives/portable_binary.hpp>

#include <data/volume.hpp>

volume_data import_volume(const std::filesystem::path& gltf_path)
{
    volume_data _data;
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
    _data.minimum = glm::vec3(std::numeric_limits<glm::float32>::max());
    _data.maximum = glm::vec3(std::numeric_limits<glm::float32>::lowest());
    for (glm::uint _i = 0; _i < _mesh->mNumVertices; ++_i) {
        const aiVector3D _position = _root_transform * _mesh->mVertices[_i];
        _data.minimum.x = glm::min(_data.minimum.x, _position.x);
        _data.minimum.y = glm::min(_data.minimum.y, _position.y);
        _data.minimum.z = glm::min(_data.minimum.z, _position.z);
        _data.maximum.x = glm::max(_data.maximum.x, _position.x);
        _data.maximum.y = glm::max(_data.maximum.y, _position.y);
        _data.maximum.z = glm::max(_data.maximum.z, _position.z);
    }
    std::cout << "   Exporting volume data binary..." << std::endl;
    return _data;
}
