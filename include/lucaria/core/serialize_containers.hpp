#pragma once

#include <type_traits>

#include <cereal/cereal.hpp>
#include <cereal/types/atomic.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/deque.hpp>
#include <cereal/types/forward_list.hpp>
#include <cereal/types/functional.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/queue.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/stack.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/valarray.hpp>
#include <pfr.hpp>

#include <lucaria/bin/types_containers.hpp>

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
