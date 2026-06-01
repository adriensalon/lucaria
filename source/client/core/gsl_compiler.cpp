
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <lucaria/core/gsl_compiler.hpp>

namespace lucaria {
namespace detail {

    gsl_compiler::gsl_compiler(const std::vector<gsl_system_info>& reflected)
    {
        const char* source = R"GLSL(
#version 450

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

void main()
{
}
)GLSL";

        glslang::TShader shader { EShLangCompute };
        shader.setStrings(&source, 1);

        shader.setEnvInput(
            glslang::EShSourceGlsl,
            EShLangCompute,
            glslang::EShClientVulkan,
            100);

        shader.setEnvClient(
            glslang::EShClientVulkan,
            glslang::EShTargetVulkan_1_2);

        shader.setEnvTarget(
            glslang::EShTargetSpv,
            glslang::EShTargetSpv_1_5);

        const TBuiltInResource* resources = GetDefaultResources();

        const EShMessages messages = static_cast<EShMessages>(
            EShMsgSpvRules | EShMsgVulkanRules);

        if (!shader.parse(resources, 450, false, messages)) {
            std::string error = "glslang parse failed:\n";
            error += shader.getInfoLog();
            error += "\n";
            error += shader.getInfoDebugLog();
            throw std::runtime_error(error);
        }

        glslang::TProgram program;
        program.addShader(&shader);

        if (!program.link(messages)) {
            std::string error = "glslang link failed:\n";
            error += program.getInfoLog();
            error += "\n";
            error += program.getInfoDebugLog();
            throw std::runtime_error(error);
        }

        std::vector<std::uint32_t> spirv = {};

        glslang::SpvOptions options = {};
        options.generateDebugInfo = false;
        options.disableOptimizer = true;
        options.optimizeSize = false;
        options.validate = false;

        glslang::GlslangToSpv(
            *program.getIntermediate(EShLangCompute),
            spirv,
            &options);
    }

}
}
