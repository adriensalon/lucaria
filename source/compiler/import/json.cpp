#include "json.hpp"

#include <fstream>
#include <sstream>

#include <cereal/external/rapidjson/document.h>
#include <cereal/external/rapidjson/rapidjson.h>


static std::string read_file(const std::filesystem::path& p)
{
    std::ifstream ifs(p, std::ios::binary);
    if (!ifs)
        throw std::runtime_error("import_event_track: cannot open file: " + p.string());
    std::ostringstream oss;
    oss << ifs.rdbuf();
    return oss.str();
}

lucaria::event_track_data import_event_track(const std::filesystem::path& json_path)
{
    using namespace rapidjson;

    const std::string text = read_file(json_path);

    Document d;
    d.Parse(text.c_str());
    if (d.HasParseError() || !d.IsObject()) {
        std::ostringstream err;
        err << "import_event_track: JSON parse error at offset " << d.GetErrorOffset();
        throw std::runtime_error(err.str());
    }

    const Value& root = d;

    auto require_number = [&](const char* key) -> float {
        if (!root.HasMember(key) || !root[key].IsNumber())
            throw std::runtime_error(std::string("import_event_track: missing or non-numeric '") + key + "'");
        return root[key].GetFloat();
    };

    auto require_array = [&](const char* key) -> const Value& {
        if (!root.HasMember(key) || !root[key].IsArray())
            throw std::runtime_error(std::string("import_event_track: missing or non-array '") + key + "'");
        return root[key];
    };

    lucaria::event_track_data out {};
    out.frames_per_second = require_number("fps");
    out.frame_start = require_number("animation_frame_start");
    out.frame_end = require_number("animation_frame_end");
    out.duration_seconds = require_number("animation_duration_seconds");

    const Value& events = require_array("animation_events");
    out.events.reserve(events.Size());

    for (const Value& e : events.GetArray()) {
        if (!e.IsObject())
            throw std::runtime_error("import_event_track: event entry is not an object");

        auto req_num = [&](const Value& obj, const char* key) -> float {
            if (!obj.HasMember(key) || !obj[key].IsNumber())
                throw std::runtime_error(std::string("import_event_track: event missing or non-numeric '") + key + "'");
            return obj[key].GetFloat();
        };
        auto req_str = [&](const Value& obj, const char* key) -> std::string {
            if (!obj.HasMember(key) || !obj[key].IsString())
                throw std::runtime_error(std::string("import_event_track: event missing or non-string '") + key + "'");
            return obj[key].GetString();
        };

        lucaria::event_data ev {};
        ev.name = req_str(e, "name");
        ev.frame = req_num(e, "frame");
        ev.time = req_num(e, "time");
        ev.time_normalized = req_num(e, "t_normalized");

        out.events.push_back(std::move(ev));
    }

    return out;
}