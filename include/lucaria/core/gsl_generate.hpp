#pragma once

#include <unordered_map>

#include <lucaria/core/gsl_reflection.hpp>

namespace lucaria {
namespace detail {
	

	struct lgsl_generator {
		lgsl_generator() = delete;
		lgsl_generator(const lgsl_generator& other) = delete;
		lgsl_generator& operator=(const lgsl_generator& other) = delete;
		lgsl_generator(lgsl_generator&& other) = delete;
		lgsl_generator& operator=(lgsl_generator&& other) = delete;
		
		lgsl_generator(
			std::unordered_map<std::string, execution_system_info>& reflected,
			std::unordered_map<std::string, std::string>& generated);

	private:
		std::unordered_map<std::string, execution_system_info>& _reflected;
		std::unordered_map<std::string, std::string>& _generated;
	};

}
}
