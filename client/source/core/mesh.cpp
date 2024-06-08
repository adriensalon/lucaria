#include <fstream>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <tiny_obj_loader.h>
#include <core/mesh.hpp>

namespace lucaria {
namespace detail {    

    bool obj_parse(const std::filesystem::path& mesh_path, tinyobj::ObjReader& mesh_reader, const tinyobj::ObjReaderConfig& mesh_reader_config)
    {
        return mesh_reader.ParseFromFile(mesh_path.generic_string(), mesh_reader_config);
    }

    void obj_verify(const tinyobj::ObjReader& mesh_reader)
    {
        if (!mesh_reader.Error().empty()) {
            std::cerr << mesh_reader.Error() << " while importing mesh" << std::endl;
            std::terminate();
        }
        if (!mesh_reader.Error().empty()) {
            std::cout << mesh_reader.Warning() << " while importing mesh" << std::endl;
            std::terminate();
        }
    }
}

mesh import_mesh(const std::filesystem::path& input)
{
    mesh _data;
    tinyobj::ObjReaderConfig _mesh_reader_config;
    tinyobj::ObjReader _mesh_reader;
    detail::obj_parse(input, _mesh_reader, _mesh_reader_config);
    detail::obj_verify(_mesh_reader);
    auto& _attributes = _mesh_reader.GetAttrib();
    auto& _shapes = _mesh_reader.GetShapes();
    for (std::size_t _s = 0; _s < _shapes.size(); _s++) {
        std::size_t _index_offset = 0;
        for (std::size_t _f = 0; _f < _shapes[_s].mesh.num_face_vertices.size(); _f++) { // loop over faces (polygon)
            std::size_t _face_vertices_count = std::size_t(_shapes[_s].mesh.num_face_vertices[_f]);
            if (_face_vertices_count != 3) {
                std::cerr << "Error while loading mesh because not triangle face" << std::endl;
                std::terminate();
            }
            for (std::size_t _v = 0; _v < _face_vertices_count; _v++) { // loop over vertices in the face
                tinyobj::index_t _index = _shapes[_s].mesh.indices[_index_offset + _v];
                _data.count++;

                // positions
                float _vx = _attributes.vertices[3 * std::size_t(_index.vertex_index) + 0];
                float _vy = _attributes.vertices[3 * std::size_t(_index.vertex_index) + 1];
                float _vz = _attributes.vertices[3 * std::size_t(_index.vertex_index) + 2];
                _data.positions.emplace_back(_vx);
                _data.positions.emplace_back(_vy);
                _data.positions.emplace_back(_vz);

                // uvs
                if (_index.texcoord_index >= 0) {
                    float _tx = _attributes.texcoords[2 * std::size_t(_index.texcoord_index) + 0];
                    float _ty = _attributes.texcoords[2 * std::size_t(_index.texcoord_index) + 1];
                    _data.texcoords.emplace_back(_tx);
                    _data.texcoords.emplace_back(1.f - _ty);
                }

                // indices
                _data.indices.emplace_back(static_cast<unsigned int>(_data.indices.size()));
            }
            _index_offset += _face_vertices_count;
        }
    }
    
    return _data;
}

void compile_mesh(const mesh& data, const std::filesystem::path& output)
{
    std::ofstream _fstream(output);
    cereal::PortableBinaryOutputArchive _archive(_fstream);
    _archive(data.count);
    _archive(data.positions);
    _archive(data.texcoords);
    _archive(data.indices);
}

}











