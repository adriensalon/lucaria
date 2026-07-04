#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>

#include <cereal/external/rapidjson/document.h>
#include <cereal/external/rapidjson/stringbuffer.h>
#include <cereal/external/rapidjson/writer.h>

#include <binc/compile_geometry.hpp>
#include <binc/import_manifest.hpp>

namespace detail {

std::string read_text_file(const std::filesystem::path& path)
{
    std::ifstream _stream(path, std::ios::binary);
    if (!_stream) {
        std::cout << "Could not open file " << path << std::endl;
        std::terminate();
    }
    std::ostringstream _buffer;
    _buffer << _stream.rdbuf();
    return _buffer.str();
}

void write_text_file_if_changed(const std::filesystem::path& path, const std::string& text)
{
    if (std::filesystem::exists(path) && read_text_file(path) == text) {
        return;
    }
    const std::filesystem::path _parent_path = path.parent_path();
    if (!_parent_path.empty()) {
        std::filesystem::create_directories(_parent_path);
    }
    std::ofstream _stream(path, std::ios::binary);
    if (!_stream) {
        std::cout << "Could not open file " << path << " for writing" << std::endl;
        std::terminate();
    }
    _stream << text;
}

std::string manifest_path(const std::filesystem::path& path)
{
    return std::filesystem::absolute(path).lexically_normal().generic_string();
}

std::int64_t file_timestamp(const std::filesystem::path& path)
{
    return static_cast<std::int64_t>(std::filesystem::last_write_time(path).time_since_epoch().count());
}

std::string profile_key(const asset_profiles profiles)
{
    std::string _key;
    _key += "texture";
    _key += std::to_string(profiles.texture_size);
    _key += "+geometry";
    _key += std::to_string(profiles.geometry_max_vertices);
    if (profiles.raw) {
        if (!_key.empty()) {
            _key += "+";
        }
        _key += "raw";
    }
    if (profiles.etc) {
        if (!_key.empty()) {
            _key += "+";
        }
        _key += "etc";
    }
    if (profiles.s3tc) {
        if (!_key.empty()) {
            _key += "+";
        }
        _key += "s3tc";
        if (profiles.s3tc_psp_layout) {
            _key += "-psp";
        }
    }
    return _key;
}

std::filesystem::path texture_output_path(const std::filesystem::path& output_file, const std::string_view suffix)
{
    return output_file.parent_path() / (output_file.stem().string() + ".lod0" + std::string(suffix) + ".bin");
}

std::vector<std::filesystem::path> expected_outputs(const asset_profiles profiles, const std::filesystem::path& input_file, const std::filesystem::path& output_file)
{
    const std::string _extension = input_file.extension().generic_string();
    std::vector<std::filesystem::path> _outputs;
    if (_extension == ".ttf"
        || _extension == ".evtt") {
        _outputs.emplace_back(output_file);
    } else if (_extension == ".glb" || _extension == ".gltf") {
        _outputs.emplace_back(geometry_output_path(output_file));
    } else if (_extension == ".wav" || _extension == ".aiff") {
        _outputs.emplace_back(output_file.parent_path() / (output_file.stem().string() + ".ogg.bin"));
    } else if (_extension == ".jpg" || _extension == ".png" || _extension == ".bmp") {
        if (profiles.raw) {
            _outputs.emplace_back(texture_output_path(output_file, ""));
        }
        if (_extension == ".png") {
            if (profiles.etc) {
                _outputs.emplace_back(texture_output_path(output_file, ".etc"));
            }
            if (profiles.s3tc) {
                _outputs.emplace_back(texture_output_path(output_file, ".s3tc"));
            }
        }
    }
    return _outputs;
}

std::vector<std::string> manifest_outputs(const std::vector<std::filesystem::path>& outputs)
{
    std::vector<std::string> _outputs;
    _outputs.reserve(outputs.size());
    for (const std::filesystem::path& _output : outputs) {
        _outputs.emplace_back(manifest_path(_output));
    }
    return _outputs;
}

bool outputs_exist(const std::vector<std::filesystem::path>& outputs)
{
    for (const std::filesystem::path& _output : outputs) {
        if (!std::filesystem::exists(_output)) {
            return false;
        }
    }
    return true;
}

bool is_up_to_date(
    const asset_manifest& manifest,
    const std::string& input,
    const std::int64_t timestamp,
    const std::string& profile,
    const std::vector<std::filesystem::path>& outputs)
{
    const auto _entry = manifest.find(input);
    if (_entry == manifest.end()) {
        return false;
    }
    const std::vector<std::string> _outputs = manifest_outputs(outputs);
    return _entry->second.timestamp == timestamp
        && _entry->second.profile == profile
        && _entry->second.outputs == _outputs
        && outputs_exist(outputs);
}

asset_manifest load_manifest(const std::optional<std::filesystem::path>& path)
{
    asset_manifest _manifest;
    if (!path || !std::filesystem::exists(*path)) {
        return _manifest;
    }

    rapidjson::Document _document;
    const std::string _text = read_text_file(*path);
    _document.Parse(_text.c_str());
    if (_document.HasParseError() || !_document.IsObject()) {
        std::cout << "Ignoring invalid asset manifest " << *path << std::endl;
        return _manifest;
    }
    if (!_document.HasMember("entries") || !_document["entries"].IsObject()) {
        return _manifest;
    }

    const rapidjson::Value& _entries = _document["entries"];
    for (auto _entry = _entries.MemberBegin(); _entry != _entries.MemberEnd(); ++_entry) {
        if (!_entry->name.IsString() || !_entry->value.IsObject()) {
            continue;
        }
        const rapidjson::Value& _value = _entry->value;
        if (!_value.HasMember("timestamp") || !_value["timestamp"].IsInt64()) {
            continue;
        }
        if (!_value.HasMember("profile") || !_value["profile"].IsString()) {
            continue;
        }
        if (!_value.HasMember("outputs") || !_value["outputs"].IsArray()) {
            continue;
        }
        asset_manifest_entry _manifest_entry;
        _manifest_entry.timestamp = _value["timestamp"].GetInt64();
        _manifest_entry.profile = _value["profile"].GetString();
        for (const rapidjson::Value& _output : _value["outputs"].GetArray()) {
            if (_output.IsString()) {
                _manifest_entry.outputs.emplace_back(_output.GetString());
            }
        }
        _manifest.emplace(_entry->name.GetString(), std::move(_manifest_entry));
    }
    return _manifest;
}

void write_manifest(const std::optional<std::filesystem::path>& path, const asset_manifest& manifest)
{
    if (!path) {
        return;
    }

    rapidjson::StringBuffer _buffer;
    rapidjson::Writer<rapidjson::StringBuffer> _writer(_buffer);
    _writer.StartObject();
    _writer.Key("version");
    _writer.Int(1);
    _writer.Key("entries");
    _writer.StartObject();

    std::vector<std::string> _inputs;
    _inputs.reserve(manifest.size());
    for (const std::pair<const std::string, asset_manifest_entry>& _pair : manifest) {
        _inputs.emplace_back(_pair.first);
    }
    std::sort(_inputs.begin(), _inputs.end());

    for (const std::string& _input : _inputs) {
        const asset_manifest_entry& _entry = manifest.at(_input);
        _writer.Key(_input.c_str());
        _writer.StartObject();
        _writer.Key("timestamp");
        _writer.Int64(_entry.timestamp);
        _writer.Key("profile");
        _writer.String(_entry.profile.c_str());
        _writer.Key("outputs");
        _writer.StartArray();
        for (const std::string& _output : _entry.outputs) {
            _writer.String(_output.c_str());
        }
        _writer.EndArray();
        _writer.EndObject();
    }

    _writer.EndObject();
    _writer.EndObject();
    write_text_file_if_changed(*path, _buffer.GetString());
}

}
