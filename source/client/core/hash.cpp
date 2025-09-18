#include <lucaria/core/hash.hpp>

#include <functional>
#include <numeric>
#include <unordered_set>

namespace lucaria {

std::size_t uvec2_hash::operator()(const glm::uvec2& vec) const
{
    std::size_t _h1 = std::hash<glm::uint> {}(vec.x);
    std::size_t _h2 = std::hash<glm::uint> {}(vec.y);
    return _h1 ^ (_h2 << 1);
}

std::size_t vec3_hash::operator()(const glm::vec3& vec) const
{
    std::size_t _h1 = std::hash<glm::float32> {}(vec.x);
    std::size_t _h2 = std::hash<glm::float32> {}(vec.y);
    std::size_t _h3 = std::hash<glm::float32> {}(vec.z);
    return _h1 ^ (_h2 << 1) ^ (_h3 << 2);
}

std::size_t path_vector_hash::operator()(const std::vector<std::filesystem::path>& paths) const
{
    std::size_t _seed = 0;
    for (const std::filesystem::path& _path : paths) {
        std::size_t _path_hash = std::filesystem::hash_value(_path);
        _seed ^= _path_hash + 0x9e3779b9 + (_seed << 6) + (_seed >> 2);
    }
    return _seed;
}

}
