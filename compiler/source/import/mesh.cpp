#include <filesystem>

#include <assimp/importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cereal/archives/portable_binary.hpp>

#include <data/mesh.hpp>

mesh_data import_mesh(const std::filesystem::path& gltf_path, bool& has_armature)
{
    mesh_data _data;
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
    has_armature = _mesh->mNumBones > 0;
    for (glm::uint _i = 0; _i < _mesh->mNumVertices; ++_i) {
        if (!has_armature) {
            const aiVector3D _position = _root_transform * _mesh->mVertices[_i];
            _data.positions.push_back(glm::vec3(_position.x, _position.y, _position.z));
        }
        if (_mesh->HasVertexColors(0)) {
            const aiColor4D _color = _mesh->mColors[0][_i];
            _data.colors.push_back(glm::vec4(_color.r, _color.g, _color.b, _color.a));
        }
        if (_mesh->HasNormals()) {
            const aiVector3D _normal = _mesh->mNormals[_i];
            _data.normals.push_back(glm::vec3(_normal.x, _normal.y, _normal.z));
        }
        if (_mesh->HasTangentsAndBitangents()) {
            const aiVector3D _tangent = _mesh->mTangents[_i];
            const aiVector3D _bitangent = _mesh->mBitangents[_i];
            _data.tangents.push_back(glm::vec3(_tangent.x, _tangent.y, _tangent.z));
            _data.bitangents.push_back(glm::vec3(_bitangent.x, _bitangent.y, _bitangent.z));
        }
        if (_mesh->mTextureCoords[0]) {
            _data.texcoords.push_back(glm::vec2(_mesh->mTextureCoords[0][_i].x, 1.f - _mesh->mTextureCoords[0][_i].y));
        }
    }
    for (glm::uint _i = 0; _i < _mesh->mNumFaces; ++_i) {
        const aiFace& _face = _mesh->mFaces[_i];
        if (_face.mNumIndices != 3) {
            std::cout << "Non-triangle face encountered in gltf '" << gltf_path << "'. Only triangles are supported." << std::endl;
            std::terminate();
        }
        _data.indices.push_back(glm::uvec3(_face.mIndices[0], _face.mIndices[1], _face.mIndices[2]));
        _data.count +=3;
    }
    std::cout << "   Exporting mesh data binary..." << std::endl;
    return _data;
}
