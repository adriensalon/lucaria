
#include <fstream>
#include <filesystem>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>

#include <data/armature.hpp>
#include <data/mesh.hpp>
#include <data/shader.hpp>
#include <data/texture.hpp>

namespace detail {

template <typename resource_data_t>
void compile_binary_or_json(const resource_data_t& data, const std::filesystem::path& output_path)
{
#if LUCARIA_JSON
    std::ofstream _fstream(output_path);
    cereal::JSONOutputArchive _archive(_fstream);
#else
    std::ofstream _fstream(output_path, std::ios::binary);
    cereal::PortableBinaryOutputArchive _archive(_fstream);
#endif
    _archive(data);
}

}

void export_binary(const armature_data& data, const std::filesystem::path& output_path)
{
    detail::compile_binary_or_json(data, output_path);
}

void export_binary(const mesh_data& data, const std::filesystem::path& output_path)
{
    detail::compile_binary_or_json(data, output_path);
}

void export_binary(const shader_data& data, const std::filesystem::path& output_path)
{
    detail::compile_binary_or_json(data, output_path);
}

void export_binary(const texture_data& data, const std::filesystem::path& output_path)
{
    detail::compile_binary_or_json(data, output_path);
}
