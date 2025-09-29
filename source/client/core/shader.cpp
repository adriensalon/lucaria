#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/shader.hpp>

namespace lucaria {
namespace {
    static void load_data_from_bytes(shader_data& data, const std::vector<char>& data_bytes)
    {
        detail::bytes_stream _stream(data_bytes);
#if LUCARIA_JSON
        cereal::JSONInputArchive _archive(_stream);
#else
        cereal::PortableBinaryInputArchive _archive(_stream);
#endif
        _archive(data);
    }
}

shader::shader(shader_data&& data)
{
    this->data = std::move(data);
}

shader::shader(const std::vector<char>& data_bytes)
{
    load_data_from_bytes(data, data_bytes);
}

shader::shader(const std::filesystem::path& data_path)
{
    detail::load_bytes(data_path, [this](const std::vector<char>& _data_bytes) {
        load_data_from_bytes(data, _data_bytes);
    });
}

fetched<shader> fetch_shader(const std::filesystem::path data_path)
{
    std::shared_ptr<std::promise<shader>> _promise = std::make_shared<std::promise<shader>>();
    
    detail::fetch_bytes(data_path, [_promise](const std::vector<char>& _data_bytes) {
        shader _shader(_data_bytes);
        _promise->set_value(std::move(_shader));
    });

    return fetched<shader>(_promise->get_future());
}

}