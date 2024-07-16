#include <fstream>
#include <iostream>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>

#include <core/armature.hpp>
#include <glue/fetch.hpp>







armature_ref::armature_ref(const armature_data& data)
{
    _count = data.count;
    // todo copy
}
const std::vector<glm::vec3>& armature_ref::get_positions() const
{
    return _positions;
}

const std::vector<glm::uvec4>& armature_ref::get_bones() const
{
    return _bones;
}

const std::vector<glm::vec4>& armature_ref::get_weights() const
{
    return _weights;
}

glm::uint armature_ref::get_count() const
{
    return _count;
}

armature_data load_armature_data(std::istringstream& armature_stream)
{
    armature_data _data;
    {
#if LUCARIA_JSON        
        cereal::JSONInputArchive _archive(armature_stream);
#else
        cereal::PortableBinaryInputArchive _archive(armature_stream);
#endif
        _archive(_data);
    }
    return _data;
}

std::shared_future<std::shared_ptr<armature_ref>> fetch_armature(const std::filesystem::path& armature_path)
{
    
}

