#pragma once

#include <filesystem>

#define CEREAL_FUTURE_EXPERIMENTAL
#include <cereal/archives/adapters.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>

namespace cereal {

template <typename ArchiveType>
void save(ArchiveType& archive, const std::filesystem::path& value)
{
    archive(cereal::make_nvp("value", value.string()));
}

template <typename ArchiveType>
void load(ArchiveType& archive, std::filesystem::path& value)
{
    std::string _temp;
    archive(cereal::make_nvp("value", _temp));
    value = std::filesystem::path(_temp);
}

}
