#pragma once

#include <type_traits>

#include <cereal/cereal.hpp>
#include <pfr.hpp>

template <typename ArchiveType, typename T>
std::enable_if_t<std::is_aggregate_v<T>
    && !cereal::traits::has_member_serialize<T, ArchiveType>::value
    && !cereal::traits::has_member_save<T, ArchiveType>::value
    && !cereal::traits::has_member_load<T, ArchiveType>::value>
serialize(ArchiveType& archive, T& object)
{
    pfr::for_each_field_with_name(object, [&](std::string_view name, auto& field) {
        const std::string _key(name);
        archive(cereal::make_nvp(_key, field));
    });
}
