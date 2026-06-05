#pragma once

#include <unordered_map>

#include <lucaria/core/gsl_reflection.hpp>
#include <lucaria/core/scenes_compute.hpp>

namespace lucaria {
namespace detail {

    struct gsl_compiler {
        gsl_compiler() = delete;
        gsl_compiler(const gsl_compiler& other) = delete;
        gsl_compiler& operator=(const gsl_compiler& other) = delete;
        gsl_compiler(gsl_compiler&& other) = delete;
        gsl_compiler& operator=(gsl_compiler&& other) = delete;

        gsl_compiler(const std::vector<gsl_system_info>& reflected);
		
    private:
       	const std::vector<gsl_system_info>* _infos = {};
    };

}
}
