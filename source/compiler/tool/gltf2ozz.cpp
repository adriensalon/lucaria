#include <fstream>
#include <iostream>
#include <string>

#include "gltf2ozz.hpp"

namespace detail {

static const std::string gltf2ozz_config = R"({
  "skeleton": {
    "import": {
      "enable": true,
      "raw": false,
      "types": {
        "skeleton": true,
        "marker": false,
        "camera": false,
        "geometry": false,
        "light": false,
        "null": false,
        "any": false
      }
    }
  },
  "animations": [
    {
      "clip": "*",
      "raw": false,
      "additive": false,
      "additive_reference": "animation",
      "sampling_rate": 0,
      "iframe_interval": 10,
      "optimize": true,
      "optimization_settings": {
        "tolerance": 0.001,
        "distance": 0.1,
        "override": [
          {
            "name": "*",
            "tolerance": 0.001,
            "distance": 0.1
          }
        ]
      },
      "tracks": {
        "properties": [
          {
            "joint_name": "*",
            "property_name": "*",
            "type": "float1",
            "raw": false,
            "optimize": true,
            "optimization_tolerance": 0.001
          }
        ],
        "motion": {
          "enable": true,
          "joint_name": "ROOT",
          "position": {
            "components": "xz",
            "reference": "skeleton",
            "bake": true,
            "loop": false,
            "raw": false,
            "optimize": true,
            "optimization_tolerance": 0.001
          },
          "rotation": {
            "components": "",
            "reference": "skeleton",
            "bake": true,
            "loop": false,
            "raw": false,
            "optimize": true,
            "optimization_tolerance": 0.001
          }
        }
      }
    }
  ]
})";

}

void execute_gltf2ozz(const std::filesystem::path& gltf2ozz_path, const std::filesystem::path& input_path, const std::filesystem::path& output_directory)
{
    const std::filesystem::path _config_path = gltf2ozz_path.parent_path() / "gltf2ozz_config.json";
    if (!std::filesystem::exists(_config_path)) {
        std::ofstream _config_stream(_config_path);
        _config_stream << detail::gltf2ozz_config;
        _config_stream.close();
    }
    std::string _command = gltf2ozz_path.string() + " --file=" + input_path.string() + " --config_file=\"" + _config_path.string() + "\"";
#if _WIN32
    _command += " > NUL 2>&1";
#elif __LINUX__
    _command += " > /dev/null 2>&1";
#endif
    // std::cout << _command << std::endl;
    const int _result = std::system(_command.c_str());
    if (_result != 0) {
        std::cout << "Error: gltf2ozz command failed with exit code " << _result << std::endl;
        std::terminate();
    }
    const std::filesystem::path _current_path = std::filesystem::current_path();
    for (const std::filesystem::directory_entry& _entry : std::filesystem::directory_iterator(_current_path)) {
        if (std::filesystem::is_regular_file(_entry.path()) && _entry.path().extension().string() == ".ozz") {
            std::filesystem::path _destination_file;
            if (_entry.path().filename().string() == "skeleton.ozz") {
                _destination_file = output_directory / (input_path.stem().string() + "_skeleton.bin");
                std::cout << "   Exporting ozz skeleton " << _destination_file.filename() << std::endl;
            } else {
                _destination_file = output_directory / (input_path.stem().string() + "_animation_" + _entry.path().stem().string() + ".bin");
                std::cout << "   Exporting ozz animation " << _destination_file.filename() << std::endl;
            }
            std::filesystem::rename(_entry.path(), _destination_file);
        }
    }
}
