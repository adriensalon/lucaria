#pragma once

#define CEREAL_FUTURE_EXPERIMENTAL
#include <cereal/archives/adapters.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>


namespace lucaria {
namespace detail {

    struct mappings_manager_game_save;
    struct mappings_manager_game_load;

    using archive_json_output_type_base = cereal::JSONOutputArchive;
    using archive_json_output = cereal::UserDataAdapter<mappings_manager_game_save, archive_json_output_type_base>;

    using archive_json_input_type_base = cereal::JSONInputArchive;
    using archive_json_input = cereal::UserDataAdapter<mappings_manager_game_load, archive_json_input_type_base>;

    using archive_binary_output_type_base = cereal::PortableBinaryOutputArchive;
    using archive_binary_output = cereal::UserDataAdapter<mappings_manager_game_save, archive_binary_output_type_base>;

    using archive_binary_input_type_base = cereal::PortableBinaryInputArchive;
    using archive_binary_input = cereal::UserDataAdapter<mappings_manager_game_load, archive_binary_input_type_base>;

}
}
