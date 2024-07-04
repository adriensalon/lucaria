#include <filesystem>
#include <fstream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>


#include <data/mesh.hpp>

/// @brief Imports a mesh as data
/// @param input the mesh path to load, containing text
/// @return the mesh data as a string
// mesh_data import_mesh(const std::filesystem::path& input)
// {
//     mesh_data _data;
//     _data.count = 0;
//     Assimp::Importer importer;
//     const aiScene* scene = importer.ReadFile(input.string(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
//     if (!scene) {
//         std::cout << "Error importing mesh file: " << importer.GetErrorString() << std::endl;
//         std::terminate();
//     }
//     if (scene->mNumMeshes == 0) {
//         std::cout << "No meshes found in mesh file." << std::endl;
//         std::terminate();
//     }
//     aiMesh* mesh = scene->mMeshes[0];
//     aiMatrix4x4 rootTransform = scene->mRootNode->mTransformation;
//     aiMatrix4x4 gltf_rotationMatrix(
//         1.0f, 0.0f, 0.0f, 0.0f,
//         0.0f, 0.0f, 1.0f, 0.0f,
//         0.0f, -1.0f, 0.0f, 0.0f,
//         0.0f, 0.0f, 0.0f, 1.0f
//     );
//     const std::string _extension = input.extension().string();

//     std::vector<std::vector<unsigned int>> vertex_bones(mesh->mNumVertices);
//     std::vector<std::vector<float>> vertex_weights(mesh->mNumVertices);
//     for (unsigned int b = 0; b < mesh->mNumBones; ++b) {
//         aiBone* bone = mesh->mBones[b];
//         for (unsigned int w = 0; w < bone->mNumWeights; ++w) {
//             unsigned int vertex_id = bone->mWeights[w].mVertexId;
//             float weight = bone->mWeights[w].mWeight;
//             vertex_bones[vertex_id].push_back(b);
//             vertex_weights[vertex_id].push_back(weight);
//         }
//     }

//     for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
//         aiVector3D rotated_vertex = mesh->mVertices[i];
//         rotated_vertex = rootTransform * rotated_vertex;
//         if (_extension == ".glb" || _extension == ".gltf") {
//             rotated_vertex = gltf_rotationMatrix * rotated_vertex;
//         }
//         _data.positions.push_back(rotated_vertex.x);
//         _data.positions.push_back(rotated_vertex.y);
//         _data.positions.push_back(rotated_vertex.z);

//         if (mesh->HasVertexColors(0)) {
//             aiColor4D color = mesh->mColors[0][i];
//             _data.colors.push_back(color.r);
//             _data.colors.push_back(color.g);
//             _data.colors.push_back(color.b);
//             _data.colors.push_back(color.a);
//         }

//         if (mesh->HasNormals()) {
//             aiVector3D normal = mesh->mNormals[i];
//             _data.normals.push_back(normal.x);
//             _data.normals.push_back(normal.y);
//             _data.normals.push_back(normal.z);
//         }

//         if (mesh->HasTangentsAndBitangents()) {
//             aiVector3D tangent = mesh->mTangents[i];
//             aiVector3D bitangent = mesh->mBitangents[i];
//             _data.tangents.push_back(tangent.x);
//             _data.tangents.push_back(tangent.y);
//             _data.tangents.push_back(tangent.z);
//             _data.bitangents.push_back(bitangent.x);
//             _data.bitangents.push_back(bitangent.y);
//             _data.bitangents.push_back(bitangent.z);
//         }

//         if (mesh->mTextureCoords[0]) {
//             _data.texcoords.push_back(mesh->mTextureCoords[0][i].x);
//             _data.texcoords.push_back(1.f - mesh->mTextureCoords[0][i].y);
//         }

//         // Store bone indices and weights for the current vertex
//         _data.bones.insert(_data.bones.end(), vertex_bones[i].begin(), vertex_bones[i].end());
//         _data.weights.insert(_data.weights.end(), vertex_weights[i].begin(), vertex_weights[i].end());
//     }
//     for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
//         const aiFace& face = mesh->mFaces[i];
//         if (face.mNumIndices != 3) {
//             std::cout << "Non-triangle face encountered. Only triangles are supported." << std::endl;
//             std::terminate();
//         }
//         for (unsigned int j = 0; j < face.mNumIndices; ++j) {
//             _data.indices.push_back(face.mIndices[j]);
//             _data.count++;
//         }
//     }
//     return _data;
// }

void copy_ozz_files(const std::filesystem::path& input, const std::filesystem::path& output_directory)
{
    std::filesystem::path _current_path = std::filesystem::current_path();
    for (const std::filesystem::directory_entry& _entry : std::filesystem::directory_iterator(_current_path)) {
        if (std::filesystem::is_regular_file(_entry.path()) && _entry.path().extension().string() == ".ozz") {
            std::filesystem::path _destination_file;
            if (_entry.path().filename().string() == "skeleton.ozz") {
                _destination_file = output_directory / (input.stem().string() + "_skeleton.bin");
            } else {
                _destination_file = output_directory / (input.stem().string() + "_animation_" + _entry.path().stem().string() + ".bin");
            }
            std::filesystem::rename(_entry.path(), _destination_file);
        }
    }
}

void compute_ozz_files(const std::filesystem::path& input, const std::filesystem::path& output_directory)
{
    const std::string _command = std::filesystem::current_path().string() + "/compiler/gltf2ozz.exe --file=" + input.string();
    const int _result = std::system(_command.c_str());
    if (_result != 0) {
        std::cout << "Error: gltf2ozz command failed with exit code " << _result << std::endl;
        std::terminate();
    }
    copy_ozz_files(input, output_directory);
}

void save_armature_file(const armature_data& armature, const std::filesystem::path& input, const std::filesystem::path& output_directory)
{
    std::ofstream _fstream(output_directory / (input.stem().string() + "_armature.bin"), std::ios::binary);
    cereal::PortableBinaryOutputArchive _archive(_fstream);
    _archive(armature);
}

mesh_data import_mesh(const std::filesystem::path& input, const std::filesystem::path& output_directory)
{
    mesh_data _data;
    armature_data _armature_data;

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

    // Define a GLTF-specific rotation matrix
    aiMatrix4x4 gltf_rotationMatrix(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    const std::string _extension = input.extension().string();

    // Function to accumulate node transformations
    std::function<aiMatrix4x4(const aiNode*)> getTransform = [&](const aiNode* node) -> aiMatrix4x4 {
        aiMatrix4x4 transform = node->mTransformation;
        const aiNode* currentNode = node->mParent;
        while (currentNode) {
            transform = currentNode->mTransformation * transform;
            currentNode = currentNode->mParent;
        }
        return transform;
    };

    // Function to apply transformations recursively
    std::function<void(const aiNode*, const aiMatrix4x4&)> applyTransforms = [&](const aiNode* node, const aiMatrix4x4& parentTransform) {
        aiMatrix4x4 nodeTransform = node->mTransformation;
        aiMatrix4x4 combinedTransform = parentTransform * nodeTransform;

        // Apply GLTF-specific rotation if necessary
        if (_extension == ".glb" || _extension == ".gltf") {
            combinedTransform = combinedTransform * gltf_rotationMatrix;
        }

        for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

            // Initialize bone and weight storage
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

            for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                aiVector3D vertex = mesh->mVertices[v];
                vertex = combinedTransform * vertex;

                _data.positions.push_back(vertex.x);
                _data.positions.push_back(vertex.y);
                _data.positions.push_back(vertex.z);

                if (mesh->HasVertexColors(0)) {
                    aiColor4D color = mesh->mColors[0][v];
                    _data.colors.push_back(color.r);
                    _data.colors.push_back(color.g);
                    _data.colors.push_back(color.b);
                    _data.colors.push_back(color.a);
                }

                if (mesh->HasNormals()) {
                    aiVector3D normal = mesh->mNormals[v];
                    normal = combinedTransform * normal;
                    _data.normals.push_back(normal.x);
                    _data.normals.push_back(normal.y);
                    _data.normals.push_back(normal.z);
                }

                if (mesh->HasTangentsAndBitangents()) {
                    aiVector3D tangent = mesh->mTangents[v];
                    aiVector3D bitangent = mesh->mBitangents[v];
                    tangent = combinedTransform * tangent;
                    bitangent = combinedTransform * bitangent;
                    _data.tangents.push_back(tangent.x);
                    _data.tangents.push_back(tangent.y);
                    _data.tangents.push_back(tangent.z);
                    _data.bitangents.push_back(bitangent.x);
                    _data.bitangents.push_back(bitangent.y);
                    _data.bitangents.push_back(bitangent.z);
                }

                if (mesh->mTextureCoords[0]) {
                    _data.texcoords.push_back(mesh->mTextureCoords[0][v].x);
                    _data.texcoords.push_back(1.f - mesh->mTextureCoords[0][v].y);
                }

                // Store bone indices and weights for the current vertex
                _armature_data.bones.insert(_armature_data.bones.end(), vertex_bones[v].begin(), vertex_bones[v].end());
                _armature_data.weights.insert(_armature_data.weights.end(), vertex_weights[v].begin(), vertex_weights[v].end());
            }

            for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
                const aiFace& face = mesh->mFaces[f];
                if (face.mNumIndices != 3) {
                    std::cout << "Non-triangle face encountered. Only triangles are supported." << std::endl;
                    std::terminate();
                }
                for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                    _data.indices.push_back(face.mIndices[j]);
                    _data.count++;
                    _armature_data.count++;
                }
            }
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            applyTransforms(node->mChildren[i], combinedTransform);
        }
    };

    // Start applying transformations from the root node
    applyTransforms(scene->mRootNode, aiMatrix4x4());

    // Debug: Log some vertices to check their final transformed positions
    // std::cout << "Sample Transformed Vertex Positions:" << std::endl;
    // for (unsigned int i = 0; i < std::min(_data.positions.size() / 3, 5u); ++i) {
    //     std::cout << "Vertex " << i << ": ("
    //               << _data.positions[i * 3] << ", "
    //               << _data.positions[i * 3 + 1] << ", "
    //               << _data.positions[i * 3 + 2] << ")" << std::endl;
    // }

    compute_ozz_files(input, output_directory);
    save_armature_file(_armature_data, input, output_directory);

    return _data;
}
