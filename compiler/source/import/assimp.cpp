#include <assimp/importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <import/assimp.hpp>

namespace detail {

geometry_data import_armature(const std::filesystem::path& assimp_path, const aiMesh* mesh, const aiMatrix4x4& root_transform)
{
    geometry_data _data;
    std::vector<std::vector<glm::uint>> _vertex_bones(mesh->mNumVertices);
    std::vector<std::vector<glm::float32>> _vertex_weights(mesh->mNumVertices);
    for (glm::uint _b = 0; _b < mesh->mNumBones; ++_b) {
        const aiBone* _bone = mesh->mBones[_b];
        for (glm::uint w = 0; w < _bone->mNumWeights; ++w) {
            const glm::uint _vertex_id = _bone->mWeights[w].mVertexId;
            const glm::float32 _weight = _bone->mWeights[w].mWeight;
            _vertex_bones[_vertex_id].push_back(_b);
            _vertex_weights[_vertex_id].push_back(_weight);
        }
    }
    _data.count = mesh->mNumVertices;
    for (glm::uint _i = 0; _i < mesh->mNumVertices; ++_i) {
        const aiVector3D _position = root_transform * mesh->mVertices[_i];
        _data.positions.push_back(glm::vec3(_position.x, _position.y, _position.z));
        _data.weights.insert(_data.weights.end(), _vertex_weights[_i].begin(), _vertex_weights[_i].end());
        _data.bones.insert(_data.bones.end(), _vertex_bones[_i].begin(), _vertex_bones[_i].end());
    }
    for (glm::uint _i = 0; _i < mesh->mNumFaces; ++_i) {
        const aiFace& _face = mesh->mFaces[_i];
        if (_face.mNumIndices != 3) {
            std::cout << "Non-triangle face encountered in gltf '" << assimp_path << "'. Only triangles are supported." << std::endl;
            std::terminate();
        }
    }
    return _data;
}

geometry_data import_mesh(const std::filesystem::path& assimp_path, const aiMesh* mesh, const aiMatrix4x4& root_transform, const bool has_armature)
{
    geometry_data _data;
    _data.count = mesh->mNumVertices;
    for (glm::uint _i = 0; _i < mesh->mNumVertices; ++_i) {
        if (!has_armature) {
            const aiVector3D _position = root_transform * mesh->mVertices[_i];
            _data.positions.push_back(glm::vec3(_position.x, _position.y, _position.z));
        }
        if (mesh->HasVertexColors(0)) {
            const aiColor4D _color = mesh->mColors[0][_i];
            _data.colors.push_back(glm::vec4(_color.r, _color.g, _color.b, _color.a));
        }
        if (mesh->HasNormals()) {
            const aiVector3D _normal = mesh->mNormals[_i];
            _data.normals.push_back(glm::vec3(_normal.x, _normal.y, _normal.z));
        }
        if (mesh->HasTangentsAndBitangents()) {
            const aiVector3D _tangent = mesh->mTangents[_i];
            const aiVector3D _bitangent = mesh->mBitangents[_i];
            _data.tangents.push_back(glm::vec3(_tangent.x, _tangent.y, _tangent.z));
            _data.bitangents.push_back(glm::vec3(_bitangent.x, _bitangent.y, _bitangent.z));
        }
        if (mesh->mTextureCoords[0]) {
            _data.texcoords.push_back(glm::vec2(mesh->mTextureCoords[0][_i].x, 1.f - mesh->mTextureCoords[0][_i].y));
        }
    }
    for (glm::uint _i = 0; _i < mesh->mNumFaces; ++_i) {
        const aiFace& _face = mesh->mFaces[_i];
        if (_face.mNumIndices != 3) {
            std::cout << "Non-triangle face encountered in gltf '" << assimp_path << "'. Only triangles are supported." << std::endl;
            std::terminate();
        }
        _data.indices.push_back(glm::uvec3(_face.mIndices[0], _face.mIndices[1], _face.mIndices[2]));
    }
    return _data;
}

}

imported_assimp_data import_assimp(const std::filesystem::path& assimp_path)
{
    imported_assimp_data _data;
    Assimp::Importer _importer;
    const aiScene* _scene = _importer.ReadFile(assimp_path.string(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
    if (!_scene) {
        std::cout << "Error importing armature file '" << assimp_path << "': " << _importer.GetErrorString() << std::endl;
        std::terminate();
    }
    if (_scene->mNumMeshes == 0) {
        std::cout << "No mesh found in armature file '" << assimp_path << "'. " << std::endl;
        std::terminate();
    }
    const aiMesh* _mesh = _scene->mMeshes[0];
    const aiMatrix4x4 _root_transform = _scene->mRootNode->mTransformation;
    const bool _has_armature = _mesh->mNumBones > 0;
    _data.mesh_geometry = detail::import_mesh(assimp_path, _mesh, _root_transform, _has_armature);
    if (_has_armature) {
        _data.armature_geometry = detail::import_armature(assimp_path, _mesh, _root_transform);
    }
    return _data;
}