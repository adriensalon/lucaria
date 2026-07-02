#if defined(LUCARIA_BACKEND_VULKAN)

#include <algorithm>
#include <array>
#include <cstring>

#include <backends/imgui_impl_vulkan.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/rendering_program.hpp>
#include <lucaria/core/rendering_vulkan.hpp>

namespace lucaria {
namespace detail {
    namespace {

        struct _program_push_constants {
            float32x4x4 matrix = float32x4x4(1.f);
            float32x4 color = { 1.f, 1.f, 1.f, 1.f };
            float32x4 uv_rect = { 0.f, 0.f, 1.f, 1.f };
            float32x4 texel_size_fxaa = { 0.f, 0.f, 0.f, 0.f };
            float32x4 fxaa_params = { 0.f, 0.f, 0.f, 0.f };
        };

        constexpr uint32 _max_bones = 100;
        constexpr VkDeviceSize _bones_transforms_offset = 0;
        constexpr VkDeviceSize _bones_invposes_offset = sizeof(float32x4x4) * _max_bones;
        constexpr VkDeviceSize _bones_buffer_size = sizeof(float32x4x4) * _max_bones * 2;

        [[nodiscard]] bool _imgui_vulkan_renderer_ready()
        {
            return ImGui::GetCurrentContext() != nullptr
                && ImGui::GetIO().BackendRendererUserData != nullptr;
        }

        [[nodiscard]] VkDescriptorSet _add_imgui_texture(const VkSampler sampler, const VkImageView image_view, const VkImageLayout image_layout)
        {
            if (sampler == VK_NULL_HANDLE || image_view == VK_NULL_HANDLE || !_imgui_vulkan_renderer_ready()) {
                return VK_NULL_HANDLE;
            }
            return ImGui_ImplVulkan_AddTexture(sampler, image_view, image_layout);
        }

        void _create_program_buffer(const VkDeviceSize size, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();

            VkBufferCreateInfo _create = {};
            _create.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            _create.size = size;
            _create.usage = usage;
            _create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            if (vkCreateBuffer(_vulkan.device, &_create, nullptr, &buffer) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan program buffer")
                return;
            }

            VkMemoryRequirements _requirements = {};
            vkGetBufferMemoryRequirements(_vulkan.device, buffer, &_requirements);

            VkMemoryAllocateInfo _allocate = {};
            _allocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            _allocate.allocationSize = _requirements.size;
            _allocate.memoryTypeIndex = rendering_vulkan_find_memory_type(_requirements.memoryTypeBits, properties);
            if (vkAllocateMemory(_vulkan.device, &_allocate, nullptr, &memory) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to allocate Vulkan program buffer memory")
                return;
            }
            vkBindBufferMemory(_vulkan.device, buffer, memory, 0);
        }

        void _destroy_program_buffer(VkBuffer& buffer, VkDeviceMemory& memory)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            if (_vulkan.device == VK_NULL_HANDLE) {
                buffer = VK_NULL_HANDLE;
                memory = VK_NULL_HANDLE;
                return;
            }
            if (buffer != VK_NULL_HANDLE) {
                vkDestroyBuffer(_vulkan.device, buffer, nullptr);
                buffer = VK_NULL_HANDLE;
            }
            if (memory != VK_NULL_HANDLE) {
                vkFreeMemory(_vulkan.device, memory, nullptr);
                memory = VK_NULL_HANDLE;
            }
        }

        [[nodiscard]] std::vector<uint32> _compile_spirv(const EShLanguage stage, const char* source)
        {
            static bool _glslang_initialized = false;
            if (!_glslang_initialized) {
                glslang::InitializeProcess();
                _glslang_initialized = true;
            }

            glslang::TShader _shader(stage);
            _shader.setStrings(&source, 1);
            _shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, 100);
            _shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
            _shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);

            const EShMessages _messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);
            if (!_shader.parse(GetDefaultResources(), 450, false, _messages)) {
                LUCARIA_DEBUG_ERROR(std::string("Failed to compile Vulkan shader: ") + _shader.getInfoLog())
                return {};
            }

            glslang::TProgram _program;
            _program.addShader(&_shader);
            if (!_program.link(_messages)) {
                LUCARIA_DEBUG_ERROR(std::string("Failed to link Vulkan shader: ") + _program.getInfoLog())
                return {};
            }

            std::vector<uint32> _spirv = {};
            glslang::GlslangToSpv(*_program.getIntermediate(stage), _spirv);
            return _spirv;
        }

        [[nodiscard]] VkShaderModule _create_shader_module(const std::vector<uint32>& spirv)
        {
            if (spirv.empty()) {
                return VK_NULL_HANDLE;
            }
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            VkShaderModuleCreateInfo _create = {};
            _create.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            _create.codeSize = spirv.size() * sizeof(uint32);
            _create.pCode = spirv.data();
            VkShaderModule _module = VK_NULL_HANDLE;
            if (vkCreateShaderModule(_vulkan.device, &_create, nullptr, &_module) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan shader module")
            }
            return _module;
        }

        [[nodiscard]] bool _contains(const std::string& text, const std::string& pattern)
        {
            return text.find(pattern) != std::string::npos;
        }

        void _create_descriptor_resources(rendering_program& program)
        {
            if (!program.shader_uses_texture && !program.shader_uses_cubemap) {
                return;
            }
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            VkDescriptorSetLayoutBinding _binding = {};
            _binding.binding = 0;
            _binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            _binding.descriptorCount = 1;
            _binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutCreateInfo _layout = {};
            _layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            _layout.bindingCount = 1;
            _layout.pBindings = &_binding;
            if (vkCreateDescriptorSetLayout(_vulkan.device, &_layout, nullptr, &program.descriptor_set_layout) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan program descriptor set layout")
                return;
            }

            VkDescriptorSetAllocateInfo _allocate = {};
            _allocate.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            _allocate.descriptorPool = _vulkan.descriptor_pool;
            _allocate.descriptorSetCount = 1;
            _allocate.pSetLayouts = &program.descriptor_set_layout;
            if (vkAllocateDescriptorSets(_vulkan.device, &_allocate, &program.descriptor_set) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to allocate Vulkan program descriptor set")
            }
        }

        void _create_bones_resources(rendering_program& program)
        {
            if (!program.shader_uses_bones) {
                return;
            }
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            _create_program_buffer(
                _bones_buffer_size,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                program.bones_buffer,
                program.bones_memory);
            if (program.bones_buffer == VK_NULL_HANDLE || program.bones_memory == VK_NULL_HANDLE) {
                _destroy_program_buffer(program.bones_buffer, program.bones_memory);
                return;
            }

            VkDescriptorSetLayoutBinding _binding = {};
            _binding.binding = 0;
            _binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            _binding.descriptorCount = 1;
            _binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            VkDescriptorSetLayoutCreateInfo _layout = {};
            _layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            _layout.bindingCount = 1;
            _layout.pBindings = &_binding;
            if (vkCreateDescriptorSetLayout(_vulkan.device, &_layout, nullptr, &program.bones_descriptor_set_layout) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan bones descriptor set layout")
                return;
            }

            VkDescriptorSetAllocateInfo _allocate = {};
            _allocate.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            _allocate.descriptorPool = _vulkan.descriptor_pool;
            _allocate.descriptorSetCount = 1;
            _allocate.pSetLayouts = &program.bones_descriptor_set_layout;
            if (vkAllocateDescriptorSets(_vulkan.device, &_allocate, &program.bones_descriptor_set) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to allocate Vulkan bones descriptor set")
                return;
            }

            VkDescriptorBufferInfo _buffer = {};
            _buffer.buffer = program.bones_buffer;
            _buffer.offset = 0;
            _buffer.range = _bones_buffer_size;

            VkWriteDescriptorSet _write = {};
            _write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            _write.dstSet = program.bones_descriptor_set;
            _write.dstBinding = 0;
            _write.descriptorCount = 1;
            _write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            _write.pBufferInfo = &_buffer;
            vkUpdateDescriptorSets(_vulkan.device, 1, &_write, 0, nullptr);
        }

        void _upload_bones_data(rendering_program& program, const VkDeviceSize offset, const void* data, const std::size_t matrix_size, const std::size_t count)
        {
            if (!program.shader_uses_bones || program.bones_memory == VK_NULL_HANDLE || data == nullptr || count == 0) {
                return;
            }
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            const std::size_t _count = std::min<std::size_t>(count, _max_bones);
            if (matrix_size == sizeof(float32x4x4)) {
                void* _mapped = nullptr;
                vkMapMemory(_vulkan.device, program.bones_memory, offset, sizeof(float32x4x4) * _count, 0, &_mapped);
                std::memcpy(_mapped, data, sizeof(float32x4x4) * _count);
                vkUnmapMemory(_vulkan.device, program.bones_memory);
                return;
            }

            void* _mapped = nullptr;
            vkMapMemory(_vulkan.device, program.bones_memory, offset, sizeof(float32x4x4) * _count, 0, &_mapped);
            const std::size_t _copy_size = std::min<std::size_t>(matrix_size, sizeof(float32x4x4));
            const uint8* _source = static_cast<const uint8*>(data);
            uint8* _destination = static_cast<uint8*>(_mapped);
            for (std::size_t i = 0; i < _count; ++i) {
                std::memcpy(_destination + sizeof(float32x4x4) * i, _source + matrix_size * i, _copy_size);
            }
            vkUnmapMemory(_vulkan.device, program.bones_memory);
        }

        void _update_descriptor(rendering_program& program, const VkSampler sampler, const VkImageView view, const VkImageLayout layout)
        {
            if (program.descriptor_set == VK_NULL_HANDLE || sampler == VK_NULL_HANDLE || view == VK_NULL_HANDLE) {
                return;
            }
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            VkDescriptorImageInfo _image = {};
            _image.sampler = sampler;
            _image.imageView = view;
            _image.imageLayout = layout;

            VkWriteDescriptorSet _write = {};
            _write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            _write.dstSet = program.descriptor_set;
            _write.dstBinding = 0;
            _write.descriptorCount = 1;
            _write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            _write.pImageInfo = &_image;
            vkUpdateDescriptorSets(_vulkan.device, 1, &_write, 0, nullptr);
        }

        void _destroy_pipeline(rendering_program& program)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            if (_vulkan.device == VK_NULL_HANDLE) {
                return;
            }
            if (program.pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(_vulkan.device, program.pipeline, nullptr);
                program.pipeline = VK_NULL_HANDLE;
            }
            if (program.pipeline_layout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(_vulkan.device, program.pipeline_layout, nullptr);
                program.pipeline_layout = VK_NULL_HANDLE;
            }
            program.pipeline_color_format = VK_FORMAT_UNDEFINED;
            program.pipeline_depth_format = VK_FORMAT_UNDEFINED;
        }

        void _destroy_program_resources(rendering_program& program)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            if (_vulkan.device == VK_NULL_HANDLE) {
                return;
            }
            _destroy_pipeline(program);
            program.descriptor_set = VK_NULL_HANDLE;
            if (program.descriptor_set_layout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(_vulkan.device, program.descriptor_set_layout, nullptr);
                program.descriptor_set_layout = VK_NULL_HANDLE;
            }
            program.bones_descriptor_set = VK_NULL_HANDLE;
            _destroy_program_buffer(program.bones_buffer, program.bones_memory);
            if (program.bones_descriptor_set_layout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(_vulkan.device, program.bones_descriptor_set_layout, nullptr);
                program.bones_descriptor_set_layout = VK_NULL_HANDLE;
            }
            if (program.vertex_module != VK_NULL_HANDLE) {
                vkDestroyShaderModule(_vulkan.device, program.vertex_module, nullptr);
                program.vertex_module = VK_NULL_HANDLE;
            }
            if (program.fragment_module != VK_NULL_HANDLE) {
                vkDestroyShaderModule(_vulkan.device, program.fragment_module, nullptr);
                program.fragment_module = VK_NULL_HANDLE;
            }
        }

        [[nodiscard]] uint32 _attribute_offset(const rendering_mesh& mesh, const data_vertex_attribute attribute)
        {
            const auto _found = mesh.attribute_offsets.find(attribute);
            if (_found == mesh.attribute_offsets.end()) {
                return 0;
            }
            return _found->second;
        }

        void _ensure_pipeline(rendering_program& program, const VkPrimitiveTopology topology, const bool use_texture, const bool use_depth)
        {
            const VkFormat _color_format = rendering_vulkan_current_color_format();
            const VkFormat _depth_format = (use_depth || program.shader_uses_cubemap) ? rendering_vulkan_current_depth_format() : VK_FORMAT_UNDEFINED;
            const uint32 _vertex_stride = program.bound_mesh != nullptr ? program.bound_mesh->vertex_stride : static_cast<uint32>(sizeof(float32x3));
            const uint32 _position_offset = program.bound_mesh != nullptr ? _attribute_offset(*program.bound_mesh, data_vertex_attribute::position) : 0;
            const uint32 _texcoord_offset = program.bound_mesh != nullptr ? _attribute_offset(*program.bound_mesh, data_vertex_attribute::texcoord) : 0;
            const uint32 _normal_offset = program.bound_mesh != nullptr ? _attribute_offset(*program.bound_mesh, data_vertex_attribute::normal) : 0;
            const uint32 _bones_offset = program.bound_mesh != nullptr ? _attribute_offset(*program.bound_mesh, data_vertex_attribute::bones) : 0;
            const uint32 _weights_offset = program.bound_mesh != nullptr ? _attribute_offset(*program.bound_mesh, data_vertex_attribute::weights) : 0;
            if (program.pipeline != VK_NULL_HANDLE
                && program.pipeline_color_format == _color_format
                && program.pipeline_depth_format == _depth_format
                && program.pipeline_topology == topology
                && program.pipeline_uses_texture == use_texture
                && program.pipeline_uses_depth == use_depth
                && program.pipeline_vertex_stride == _vertex_stride
                && program.pipeline_position_offset == _position_offset
                && program.pipeline_texcoord_offset == _texcoord_offset
                && program.pipeline_normal_offset == _normal_offset
                && program.pipeline_bones_offset == _bones_offset
                && program.pipeline_weights_offset == _weights_offset) {
                return;
            }

            _destroy_pipeline(program);
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            const bool _has_descriptor_set = program.shader_uses_texture || program.shader_uses_cubemap;
            std::array<VkDescriptorSetLayout, 2> _set_layouts = {};
            uint32 _set_layout_count = 0;
            if (_has_descriptor_set) {
                _set_layouts[0] = program.descriptor_set_layout;
                _set_layout_count = 1;
            }
            if (program.shader_uses_bones) {
                _set_layouts[1] = program.bones_descriptor_set_layout;
                _set_layout_count = 2;
            }

            VkPushConstantRange _push_range = {};
            _push_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            _push_range.offset = 0;
            _push_range.size = sizeof(_program_push_constants);

            VkPipelineLayoutCreateInfo _pipeline_layout = {};
            _pipeline_layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            _pipeline_layout.setLayoutCount = _set_layout_count;
            _pipeline_layout.pSetLayouts = _set_layout_count == 0 ? nullptr : _set_layouts.data();
            _pipeline_layout.pushConstantRangeCount = 1;
            _pipeline_layout.pPushConstantRanges = &_push_range;
            vkCreatePipelineLayout(_vulkan.device, &_pipeline_layout, nullptr, &program.pipeline_layout);

            std::array<VkPipelineShaderStageCreateInfo, 2> _stages = {};
            _stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            _stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            _stages[0].module = program.vertex_module;
            _stages[0].pName = "main";
            _stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            _stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            _stages[1].module = program.fragment_module;
            _stages[1].pName = "main";

            VkVertexInputBindingDescription _binding = {};
            _binding.binding = 0;
            _binding.stride = _vertex_stride;
            _binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            std::array<VkVertexInputAttributeDescription, 5> _attributes = {};
            _attributes[0] = { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, _position_offset };
            _attributes[1] = { 1, 0, VK_FORMAT_R32G32_SFLOAT, _texcoord_offset };
            _attributes[2] = { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, _normal_offset };
            _attributes[3] = { 3, 0, VK_FORMAT_R32G32B32A32_SINT, _bones_offset };
            _attributes[4] = { 4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, _weights_offset };

            VkPipelineVertexInputStateCreateInfo _vertex_input = {};
            _vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            _vertex_input.vertexBindingDescriptionCount = 1;
            _vertex_input.pVertexBindingDescriptions = &_binding;
            _vertex_input.vertexAttributeDescriptionCount = static_cast<uint32>(_attributes.size());
            _vertex_input.pVertexAttributeDescriptions = _attributes.data();

            VkPipelineInputAssemblyStateCreateInfo _input_assembly = {};
            _input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            _input_assembly.topology = topology;

            VkPipelineViewportStateCreateInfo _viewport_state = {};
            _viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            _viewport_state.viewportCount = 1;
            _viewport_state.scissorCount = 1;

            VkPipelineRasterizationStateCreateInfo _rasterization = {};
            _rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            _rasterization.polygonMode = VK_POLYGON_MODE_FILL;
            _rasterization.cullMode = topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST || program.shader_uses_cubemap ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;
            _rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            _rasterization.lineWidth = 1.0f;

            VkPipelineMultisampleStateCreateInfo _multisample = {};
            _multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            _multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineDepthStencilStateCreateInfo _depth = {};
            _depth.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            _depth.depthTestEnable = use_depth && _depth_format != VK_FORMAT_UNDEFINED;
            _depth.depthWriteEnable = use_depth && _depth_format != VK_FORMAT_UNDEFINED;
            _depth.depthCompareOp = VK_COMPARE_OP_GREATER;

            VkPipelineColorBlendAttachmentState _blend_attachment = {};
            _blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            _blend_attachment.blendEnable = VK_TRUE;
            _blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            _blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            _blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
            _blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            _blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            _blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

            VkPipelineColorBlendStateCreateInfo _blend = {};
            _blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            _blend.attachmentCount = 1;
            _blend.pAttachments = &_blend_attachment;

            std::array<VkDynamicState, 2> _dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            VkPipelineDynamicStateCreateInfo _dynamic = {};
            _dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            _dynamic.dynamicStateCount = static_cast<uint32>(_dynamic_states.size());
            _dynamic.pDynamicStates = _dynamic_states.data();

            VkPipelineRenderingCreateInfo _rendering = {};
            _rendering.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            _rendering.colorAttachmentCount = 1;
            _rendering.pColorAttachmentFormats = &_color_format;
            _rendering.depthAttachmentFormat = _depth_format;

            VkGraphicsPipelineCreateInfo _create = {};
            _create.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            _create.pNext = &_rendering;
            _create.stageCount = static_cast<uint32>(_stages.size());
            _create.pStages = _stages.data();
            _create.pVertexInputState = &_vertex_input;
            _create.pInputAssemblyState = &_input_assembly;
            _create.pViewportState = &_viewport_state;
            _create.pRasterizationState = &_rasterization;
            _create.pMultisampleState = &_multisample;
            _create.pDepthStencilState = &_depth;
            _create.pColorBlendState = &_blend;
            _create.pDynamicState = &_dynamic;
            _create.layout = program.pipeline_layout;
            _create.renderPass = VK_NULL_HANDLE;
            if (vkCreateGraphicsPipelines(_vulkan.device, VK_NULL_HANDLE, 1, &_create, nullptr, &program.pipeline) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan graphics pipeline")
            }

            program.pipeline_color_format = _color_format;
            program.pipeline_depth_format = _depth_format;
            program.pipeline_topology = topology;
            program.pipeline_uses_texture = use_texture;
            program.pipeline_uses_depth = use_depth;
            program.pipeline_vertex_stride = _vertex_stride;
            program.pipeline_position_offset = _position_offset;
            program.pipeline_texcoord_offset = _texcoord_offset;
            program.pipeline_normal_offset = _normal_offset;
            program.pipeline_bones_offset = _bones_offset;
            program.pipeline_weights_offset = _weights_offset;
        }

        void _bind_common_state(const rendering_program& program, const VkPrimitiveTopology topology, const bool use_texture, const bool use_depth)
        {
            rendering_program& _program = const_cast<rendering_program&>(program);
            _ensure_pipeline(_program, topology, use_texture, use_depth);
            VkCommandBuffer _commands = rendering_vulkan_command_buffer();
            vkCmdBindPipeline(_commands, VK_PIPELINE_BIND_POINT_GRAPHICS, program.pipeline);

            const uint32x2 _size = rendering_vulkan_current_target_size();
            VkViewport _viewport = {};
            _viewport.x = 0.f;
            _viewport.y = static_cast<float32>(_size.y);
            _viewport.width = static_cast<float32>(_size.x);
            _viewport.height = -static_cast<float32>(_size.y);
            _viewport.minDepth = 0.f;
            _viewport.maxDepth = 1.f;
            vkCmdSetViewport(_commands, 0, 1, &_viewport);

            VkRect2D _scissor = {};
            _scissor.extent = { _size.x, _size.y };
            vkCmdSetScissor(_commands, 0, 1, &_scissor);

            _program_push_constants _push = {};
            _push.matrix = program.push_matrix;
            _push.color = program.push_color;
            _push.uv_rect = program.push_uv_rect;
            _push.texel_size_fxaa = program.push_texel_size_fxaa;
            _push.fxaa_params = program.push_fxaa_params;
            vkCmdPushConstants(_commands, program.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(_push), &_push);
        }

    }

    rendering_program::~rendering_program()
    {
        if (!ownership.owns()) {
            return;
        }
        rendering_vulkan_context& _vulkan = rendering_vulkan();
        if (_vulkan.device == VK_NULL_HANDLE) {
            return;
        }
        _destroy_program_resources(*this);
    }

    rendering_program::rendering_program(const object_shader& vertex, const object_shader& fragment)
    {
        shader_uses_texture = _contains(fragment.data.text, "sampler2D");
        shader_uses_cubemap = _contains(fragment.data.text, "samplerCube");
        shader_uses_bones = _contains(vertex.data.text, "uniform_bones_transforms") || _contains(vertex.data.text, "uniform_bones_invposes");
        vertex_module = _create_shader_module(_compile_spirv(EShLangVertex, vertex.data.text.c_str()));
        fragment_module = _create_shader_module(_compile_spirv(EShLangFragment, fragment.data.text.c_str()));
        _create_descriptor_resources(*this);
        _create_bones_resources(*this);
        ownership.emplace();
    }

    void rendering_program::use() const
    {
    }

    void rendering_program::bind_attribute(const std::string&, const rendering_mesh& mesh, const data_vertex_attribute)
    {
        bound_mesh = &mesh;
        bound_line_mesh = nullptr;
        bound_indices_count = mesh.size;
        bound_index_offset = mesh.allocation.elements.offset;
        push_has_texcoord = mesh.attribute_offsets.find(data_vertex_attribute::texcoord) != mesh.attribute_offsets.end();
    }

    void rendering_program::bind_uniform(const std::string&, const rendering_cubemap& cubemap, const uint32) const
    {
        bound_cubemap = &cubemap;
        bound_texture = nullptr;
        push_use_texture = cubemap.descriptor != VK_NULL_HANDLE || cubemap.sampler != VK_NULL_HANDLE;
        push_color = { 1.f, 1.f, 1.f, 1.f };
        _update_descriptor(*const_cast<rendering_program*>(this), cubemap.sampler, cubemap.image_view, cubemap.layout);
    }

    void rendering_program::bind_uniform(const std::string&, const rendering_texture& texture, const uint32) const
    {
        rendering_texture& _texture = const_cast<rendering_texture&>(texture);
        if (_texture.imgui_descriptor == VK_NULL_HANDLE && _texture.sampler != VK_NULL_HANDLE && _texture.image_view != VK_NULL_HANDLE && _texture.layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            _texture.imgui_descriptor = _add_imgui_texture(_texture.sampler, _texture.image_view, _texture.layout);
        }
        bound_texture = &texture;
        bound_cubemap = nullptr;
        push_use_texture = _texture.sampler != VK_NULL_HANDLE && _texture.image_view != VK_NULL_HANDLE;
        push_uv_rect = texture.uv_rect;
        push_color = { 1.f, 1.f, 1.f, 1.f };
        _update_descriptor(*const_cast<rendering_program*>(this), _texture.sampler, _texture.image_view, _texture.layout);
    }

    template <>
    void rendering_program::bind_uniform<int32>(const std::string& name, const int32& value)
    {
        if (name.find("color") != std::string::npos) {
            push_color = { static_cast<float32>(value), 0.f, 0.f, 1.f };
        }
    }

    template <>
    void rendering_program::bind_uniform<float32>(const std::string& name, const float32& value)
    {
        if (name.find("color") != std::string::npos) {
            push_color = { value, value, value, 1.f };
        } else if (name == "uniform_fxaa_enable") {
            push_texel_size_fxaa.z = value;
        } else if (name == "uniform_fxaa_contrast_threshold") {
            push_fxaa_params.x = value;
        } else if (name == "uniform_fxaa_relative_threshold") {
            push_fxaa_params.y = value;
        } else if (name == "uniform_fxaa_edge_sharpness") {
            push_fxaa_params.z = value;
        }
    }

    template <>
    void rendering_program::bind_uniform<std::vector<float32>>(const std::string&, const std::vector<float32>&)
    {
    }

    template <>
    void rendering_program::bind_uniform<float32x2>(const std::string& name, const float32x2& value)
    {
        if (name == "uniform_texel_size") {
            push_texel_size_fxaa.x = value.x;
            push_texel_size_fxaa.y = value.y;
        }
    }

    template <>
    void rendering_program::bind_uniform<std::vector<float32x2>>(const std::string&, const std::vector<float32x2>&)
    {
    }

    template <>
    void rendering_program::bind_uniform<float32x3>(const std::string& name, const float32x3& value)
    {
        if (name.find("color") != std::string::npos) {
            push_color = { value.x, value.y, value.z, 1.f };
        }
    }

    template <>
    void rendering_program::bind_uniform<std::vector<float32x3>>(const std::string&, const std::vector<float32x3>&)
    {
    }

    template <>
    void rendering_program::bind_uniform<float32x4>(const std::string& name, const float32x4& value)
    {
        if (name.find("color") != std::string::npos) {
            push_color = value;
        }
    }

    template <>
    void rendering_program::bind_uniform<std::vector<float32x4>>(const std::string&, const std::vector<float32x4>&)
    {
    }

    template <>
    void rendering_program::bind_uniform<float32x4x4>(const std::string& name, const float32x4x4& value)
    {
        push_matrix = value;
    }

    template <>
    void rendering_program::bind_uniform<std::vector<float32x4x4>>(const std::string& name, const std::vector<float32x4x4>& value)
    {
        if (name.find("uniform_bones_transforms") != std::string::npos) {
            _upload_bones_data(*this, _bones_transforms_offset, value.data(), sizeof(float32x4x4), value.size());
        } else if (name.find("uniform_bones_invposes") != std::string::npos) {
            _upload_bones_data(*this, _bones_invposes_offset, value.data(), sizeof(float32x4x4), value.size());
        }
    }

    template <>
    void rendering_program::bind_uniform<ozz::vector<ozz::math::Float4x4>>(const std::string& name, const ozz::vector<ozz::math::Float4x4>& value)
    {
        if (name.find("uniform_bones_transforms") != std::string::npos) {
            _upload_bones_data(*this, _bones_transforms_offset, value.data(), sizeof(ozz::math::Float4x4), value.size());
        } else if (name.find("uniform_bones_invposes") != std::string::npos) {
            _upload_bones_data(*this, _bones_invposes_offset, value.data(), sizeof(ozz::math::Float4x4), value.size());
        }
    }

    void rendering_program::draw(const bool use_depth) const
    {
        if (bound_mesh == nullptr || bound_indices_count == 0 || bound_mesh->vertices_buffer == VK_NULL_HANDLE || bound_mesh->elements_buffer == VK_NULL_HANDLE) {
            return;
        }
        rendering_vulkan_begin_rendering(false, false);
        const bool _use_texture = push_use_texture && descriptor_set != VK_NULL_HANDLE;
        _bind_common_state(*this, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, _use_texture, use_depth);

        VkCommandBuffer _commands = rendering_vulkan_command_buffer();
        VkDeviceSize _offset = bound_mesh->allocation.vertices.offset;
        vkCmdBindVertexBuffers(_commands, 0, 1, &bound_mesh->vertices_buffer, &_offset);
        vkCmdBindIndexBuffer(_commands, bound_mesh->elements_buffer, bound_index_offset, VK_INDEX_TYPE_UINT32);
        if (_use_texture) {
            vkCmdBindDescriptorSets(_commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);
        }
        if (shader_uses_bones && bones_descriptor_set != VK_NULL_HANDLE) {
            vkCmdBindDescriptorSets(_commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 1, 1, &bones_descriptor_set, 0, nullptr);
        }
        vkCmdDrawIndexed(_commands, bound_indices_count, 1, 0, 0, 0);
    }

#if defined(LUCARIA_DEBUG)
    void rendering_program::bind_guizmo(const std::string&, const rendering_mesh_line& from)
    {
        bound_mesh = nullptr;
        bound_line_mesh = &from;
        bound_indices_count = from.size;
        bound_index_offset = 0;
        push_has_texcoord = false;
        push_use_texture = false;
    }

    void rendering_program::draw_guizmo() const
    {
        if (bound_line_mesh == nullptr || bound_indices_count == 0 || bound_line_mesh->positions_buffer == VK_NULL_HANDLE || bound_line_mesh->elements_buffer == VK_NULL_HANDLE) {
            return;
        }
        rendering_vulkan_begin_rendering(false, false);
        _bind_common_state(*this, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, false, false);

        VkCommandBuffer _commands = rendering_vulkan_command_buffer();
        VkDeviceSize _offset = 0;
        vkCmdBindVertexBuffers(_commands, 0, 1, &bound_line_mesh->positions_buffer, &_offset);
        vkCmdBindIndexBuffer(_commands, bound_line_mesh->elements_buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(_commands, bound_indices_count, 1, 0, 0, 0);
    }
#endif

    void rendering_program::viewport(const uint32x2 size)
    {
        if (!rendering_vulkan().frame_active || !rendering_vulkan().rendering_active) {
            return;
        }
        VkViewport _viewport = {};
        _viewport.x = 0.f;
        _viewport.y = static_cast<float32>(size.y);
        _viewport.width = static_cast<float32>(size.x);
        _viewport.height = -static_cast<float32>(size.y);
        _viewport.minDepth = 0.f;
        _viewport.maxDepth = 1.f;
        vkCmdSetViewport(rendering_vulkan_command_buffer(), 0, 1, &_viewport);
    }

    void rendering_program::clear(const bool clear_depth)
    {
        rendering_vulkan_begin_rendering(true, clear_depth);
    }

}
}

#endif
