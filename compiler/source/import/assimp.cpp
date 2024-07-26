#include <assimp/importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <import/assimp.hpp>

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
    std::vector<std::vector<glm::uint>> _vertex_bones(_mesh->mNumVertices);
    std::vector<std::vector<glm::float32>> _vertex_weights(_mesh->mNumVertices);
    _data.mesh_geometry.count = _mesh->mNumVertices;
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
        _data.mesh_geometry.weights.insert(_data.mesh_geometry.weights.end(), _vertex_weights[_i].begin(), _vertex_weights[_i].end());
        _data.mesh_geometry.bones.insert(_data.mesh_geometry.bones.end(), _vertex_bones[_i].begin(), _vertex_bones[_i].end());
        if (_mesh->HasPositions()) {
            const aiVector3D _position = _root_transform * _mesh->mVertices[_i];
            _data.mesh_geometry.positions.push_back(glm::vec3(_position.x, _position.y, _position.z));
        }
        if (_mesh->HasVertexColors(0)) {
            const aiColor4D _color = _mesh->mColors[0][_i];
            _data.mesh_geometry.colors.push_back(glm::vec4(_color.r, _color.g, _color.b, _color.a));
        }
        if (_mesh->HasNormals()) {
            const aiVector3D _normal = _mesh->mNormals[_i];
            _data.mesh_geometry.normals.push_back(glm::vec3(_normal.x, _normal.y, _normal.z));
        }
        if (_mesh->HasTangentsAndBitangents()) {
            const aiVector3D _tangent = _mesh->mTangents[_i];
            const aiVector3D _bitangent = _mesh->mBitangents[_i];
            _data.mesh_geometry.tangents.push_back(glm::vec3(_tangent.x, _tangent.y, _tangent.z));
            _data.mesh_geometry.bitangents.push_back(glm::vec3(_bitangent.x, _bitangent.y, _bitangent.z));
        }
        if (_mesh->mTextureCoords[0]) {
            _data.mesh_geometry.texcoords.push_back(glm::vec2(_mesh->mTextureCoords[0][_i].x, 1.f - _mesh->mTextureCoords[0][_i].y));
        }
    }
    for (glm::uint _i = 0; _i < _mesh->mNumFaces; ++_i) {
        const aiFace& _face = _mesh->mFaces[_i];
        if (_face.mNumIndices != 3) {
            std::cout << "Non-triangle face encountered in gltf '" << assimp_path << "'. Only triangles are supported." << std::endl;
            std::terminate();
        }
        _data.mesh_geometry.indices.push_back(glm::uvec3(_face.mIndices[0], _face.mIndices[1], _face.mIndices[2]));
    }
    _data.has_skeleton = _data.mesh_geometry.bones.size() > 0;
    return _data;
}