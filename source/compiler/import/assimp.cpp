#include <fstream>
#include <unordered_map>

#include <assimp/importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/gtc/type_ptr.hpp>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/io/archive.h>
#include <ozz/base/io/stream.h>

#include "assimp.hpp"

namespace {
    
class ozz_raw_input_stream : public ozz::io::Stream {
public:
    ozz_raw_input_stream(const std::vector<char>& data)
        : data_(data), position_(0) {}

    ~ozz_raw_input_stream() override = default;

    bool opened() const override {
        return true;  // The stream is always "opened" when constructed
    }

    size_t Read(void* buffer, size_t size) override {
        size_t remaining = data_.size() - position_;
        size_t to_read = std::min(size, remaining);
        std::memcpy(buffer, data_.data() + position_, to_read);
        position_ += to_read;
        return to_read;
    }

    size_t Write(const void* buffer, size_t size) override {
        // Not implemented since this is a read-only stream
        return 0;
    }

    int Seek(int offset, Origin origin) override {
        int new_position = 0;
        switch (origin) {
            case kSet:
                new_position = offset;
                break;
            case kCurrent:
                new_position = static_cast<int>(position_) + offset;
                break;
            case kEnd:
                new_position = static_cast<int>(data_.size()) + offset;
                break;
            default:
                return -1;
        }
        if (new_position < 0 || static_cast<size_t>(new_position) > data_.size()) {
            return -1;
        }
        position_ = new_position;
        return 0;
    }

    int Tell() const override {
        return static_cast<int>(position_);
    }
    
    size_t Size() const override {
        return data_.size();
    }

private:
    const std::vector<char>& data_;
    size_t position_;
};

void print_matrix(const glm::mat4& matrix)
{
    const float* p = glm::value_ptr(matrix); // Get a pointer to the matrix data

    std::cout << std::fixed << std::setprecision(2); // Format for better readability

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            std::cout << std::setw(8) << p[row + col * 4] << " "; // Print each element with padding
        }
        std::cout << std::endl; // New line after each row
    }

}

bool load_binary_file(const std::filesystem::path& filename, std::vector<char>& buffer) {
    // Open the file in binary mode at the end to determine the file size
    std::ifstream file(filename.string(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }

    // Get the file size
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Resize the vector to hold the file content
    buffer.resize(size);

    // Read the file content into the buffer
    if (!file.read(buffer.data(), size)) {
        std::cerr << "Error: Could not read file " << filename << std::endl;
        return false;
    }

    return true;
}

void print_bone_names(const ozz::animation::Skeleton& skeleton) {
    const int num_joints = skeleton.num_joints();
    for (int i = 0; i < num_joints; ++i) {
        const char* joint_name = skeleton.joint_names()[i];
        printf("Bone %d: %s\n", i, joint_name);
    }
    std::cout << std::endl;
}

void print_bone_names(const aiMesh* mesh) {
    for (unsigned int j = 0; j < mesh->mNumBones; ++j) {
        aiBone* bone = mesh->mBones[j];
        std::cout << "Bone " << j << ": " << bone->mName.C_Str() << std::endl;
    }
    std::cout << std::endl;
}
}

bool assimp_has_skeleton(const std::filesystem::path& assimp_path)
{
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
    return _mesh->mNumBones > 0;
}

geometry_data import_assimp(const std::filesystem::path& assimp_path, const std::optional<std::filesystem::path>& skeleton_path)
{
    geometry_data _data;
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

    std::unordered_map<std::string, glm::int32> _skeleton_reindex = {};
    if (skeleton_path.has_value()) {
        std::vector<char> _skeleton_bytes;
        load_binary_file(skeleton_path.value(), _skeleton_bytes);
        ozz::animation::Skeleton _skeleton;
        ozz_raw_input_stream _ozz_stream(_skeleton_bytes);
        {
            ozz::io::IArchive _ozz_archive(&_ozz_stream);
            _ozz_archive >> _skeleton;
        }
        for (int i = 0; i < _skeleton.num_joints(); ++i) {
            _skeleton_reindex[_skeleton.joint_names()[i]] = i;
        }
        // print_bone_names(_skeleton);
    }
    // print_bone_names(_mesh);

    const aiMatrix4x4 _root_transform = _scene->mRootNode->mTransformation;
    _data.count = _mesh->mNumVertices;
    _data.bones.resize(_data.count);
    _data.weights.resize(_data.count, glm::vec4(0.f));
    _data.invposes.resize(_skeleton_reindex.size(), glm::mat4(1.f));
    for (glm::uint _b = 0; _b < _mesh->mNumBones; ++_b) {
        const aiBone* _bone = _mesh->mBones[_b];
        const glm::int32 _bone_reindex = _skeleton_reindex.at(std::string(_bone->mName.C_Str()));
        _data.invposes[_bone_reindex] = glm::transpose(*(reinterpret_cast<const glm::mat4*>(&(_bone->mOffsetMatrix))));
        for (glm::uint w = 0; w < _bone->mNumWeights; ++w) {
            const glm::uint _vertex_id = _bone->mWeights[w].mVertexId;
            const glm::float32 _weight = _bone->mWeights[w].mWeight;
            for (int l = 0; l < 4; ++l) {
                if (_data.weights[_vertex_id][l] == 0.0f) {
                    _data.bones[_vertex_id][l] = _bone_reindex;
                    _data.weights[_vertex_id][l] = _weight;
                    break;
                }
            }
        }
    }

    for (glm::uint _i = 0; _i < _mesh->mNumVertices; ++_i) {
        if (_mesh->HasPositions()) {
            // const aiVector3D _position = _root_transform * _mesh->mVertices[_i];
            const aiVector3D _position = _mesh->mVertices[_i];
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
            std::cout << "Non-triangle face encountered in gltf '" << assimp_path << "'. Only triangles are supported." << std::endl;
            std::terminate();
        }
        _data.indices.push_back(glm::uvec3(_face.mIndices[0], _face.mIndices[1], _face.mIndices[2]));
    }
    return _data;
}