#pragma once

#include <filesystem>
#include <string>

#include <cereal/types/string.hpp>

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