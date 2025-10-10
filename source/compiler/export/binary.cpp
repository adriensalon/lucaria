
#include <fstream>

#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include "binary.hpp"

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

void export_binary(const lucaria::geometry_data& data, const std::filesystem::path& output_path)
{
    detail::compile_binary_or_json(data, output_path);
    std::cout << "   Exporting binary geometry " << output_path.filename() << std::endl;
}

void export_binary(const lucaria::shader_data& data, const std::filesystem::path& output_path)
{
    detail::compile_binary_or_json(data, output_path);
    std::cout << "   Exporting binary shader " << output_path.filename() << std::endl;
}

void export_binary(const lucaria::image_data& data, const std::filesystem::path& output_path)
{
    detail::compile_binary_or_json(data, output_path);
    std::cout << "   Exporting binary image " << output_path.filename() << std::endl;
}

void export_binary(const lucaria::event_track_data& data, const std::filesystem::path& output_path)
{
    detail::compile_binary_or_json(data, output_path);
    std::cout << "   Exporting binary event track " << output_path.filename() << std::endl;
}
