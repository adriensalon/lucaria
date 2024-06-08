#include <GLES3/gl3.h>
#include <backends/imgui_impl_opengl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glue/graphics.hpp>
#include <glue/window.hpp>
#include <imgui.h>
#include <iostream>
#include <optional>
#include <tiny_gltf.h>

namespace lucaria {
namespace detail {

    extern void emscripten_assert(EMSCRIPTEN_RESULT result);

    static float perspective_fov = 60.f;
    static float perspective_near = 0.1f;
    static float perspective_far = 100.f;
    static glm::mat4x4 camera_projection = {};

    static bool setup_opengl();
    static bool static_setup = setup_opengl();

    static bool is_webgl2;

    static bool setup_opengl()
    {
        EmscriptenWebGLContextAttributes _webgl_attributes;
        emscripten_webgl_init_context_attributes(&_webgl_attributes);
        _webgl_attributes.explicitSwapControl = 0;
        _webgl_attributes.depth = 1;
        _webgl_attributes.stencil = 1;
        _webgl_attributes.antialias = 1;
        _webgl_attributes.majorVersion = 2;
        _webgl_attributes.minorVersion = 0;
        EMSCRIPTEN_WEBGL_CONTEXT_HANDLE _webgl_context = emscripten_webgl_create_context("#canvas", &_webgl_attributes);
        if (_webgl_context < 0) {
            detail::is_webgl2 = false;
            _webgl_attributes.majorVersion = 1;
            _webgl_context = emscripten_webgl_create_context("#canvas", &_webgl_attributes);
            if (_webgl_context < 0) {
                std::cerr << "WebGL context cannot be created at all" << std::endl;
                std::terminate();
            }
        }
        emscripten_assert(emscripten_webgl_make_context_current(_webgl_context));
        detail::is_webgl2 = true;
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().IniFilename = NULL;
        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL3_Init("#version 300 es");
        return true;
    }

    constexpr std::string_view vertex_shader = "#version 300 es \n"
                                               "in vec3 vert_position; \n"
                                               "in vec2 vert_texcoord; \n"
                                               "uniform mat4 uniform_projection; \n"
                                               "out vec2 frag_texcoord; \n"
                                               "void main() { \n"
                                               "    frag_texcoord = vert_texcoord; \n"
                                               "    gl_Position = uniform_projection * vec4(vert_position, 1); \n"
                                               "}";

    constexpr std::string_view fragment_shader = "#version 300 es \n"
                                                 "precision mediump float; \n"
                                                 "in vec2 frag_texcoord; \n"
                                                 "uniform sampler2D uniform_color; \n"
                                                 "out vec4 output_color; \n"
                                                 "void main() { \n"
                                                 "    output_color = texture(uniform_color, frag_texcoord); \n"
                                                 "}";

    void load_gltf_positions(const tinygltf::Model& model, const tinygltf::Primitive& primitive, std::vector<GLfloat>& positions)
    {
#if DEBUG
        if (primitive.attributes.find("POSITION") == primitive.attributes.end()) {
            std::cerr << "Impossible to locate position attribute in glTF mesh" << std::endl;
            std::terminate();
        }
#endif
        const tinygltf::Accessor& _accessor = model.accessors[primitive.attributes.find("POSITION")->second];
        const tinygltf::BufferView& _buffer_view = model.bufferViews[_accessor.bufferView];
        const tinygltf::Buffer& _buffer = model.buffers[_buffer_view.buffer];
        const float* _ptr = reinterpret_cast<const float*>(&(_buffer.data[_accessor.byteOffset + _buffer_view.byteOffset]));
        positions.insert(positions.end(), _ptr, _ptr + _accessor.count * 3);
    }

    void load_gltf_texcoords(const tinygltf::Model& model, const tinygltf::Primitive& primitive, std::vector<GLfloat>& texcoords)
    {
#if DEBUG
        if (primitive.attributes.find("TEXCOORD_0") == primitive.attributes.end()) {
            std::cerr << "Impossible to locate texcoord 0 attribute in glTF mesh" << std::endl;
            std::terminate();
        }
#endif
        const tinygltf::Accessor& _accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
        const tinygltf::BufferView& _buffer_view = model.bufferViews[_accessor.bufferView];
        const tinygltf::Buffer& _buffer = model.buffers[_buffer_view.buffer];
        const float* _ptr = reinterpret_cast<const float*>(&(_buffer.data[_accessor.byteOffset + _buffer_view.byteOffset]));
        texcoords.insert(texcoords.end(), _ptr, _ptr + _accessor.count * 2);
    }

    void load_gltf_indices(const tinygltf::Model& model, const tinygltf::Primitive& primitive, std::vector<GLuint>& indices)
    {        
#if DEBUG
        if (primitive.indices < 0) {
            std::cerr << "Impossible to locate indices in glTF mesh" << std::endl;
            std::terminate();
        }
#endif
        const tinygltf::Accessor& _accessor = model.accessors[primitive.indices];
        const tinygltf::BufferView& _buffer_view = model.bufferViews[_accessor.bufferView];
        const tinygltf::Buffer& _buffer = model.buffers[_buffer_view.buffer];        
        if (_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
            const unsigned short* indicesBuffer = reinterpret_cast<const unsigned short*>(&(_buffer.data[_accessor.byteOffset + _buffer_view.byteOffset]));
            indices.insert(indices.end(), indicesBuffer, indicesBuffer + _accessor.count);
        } else if (_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
            const unsigned int* indicesBuffer = reinterpret_cast<const unsigned int*>(&(_buffer.data[_accessor.byteOffset + _buffer_view.byteOffset]));
            indices.insert(indices.end(), indicesBuffer, indicesBuffer + _accessor.count);
        } else if (_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
            const unsigned char* indicesBuffer = reinterpret_cast<const unsigned char*>(&(_buffer.data[_accessor.byteOffset + _buffer_view.byteOffset]));
            indices.insert(indices.end(), indicesBuffer, indicesBuffer + _accessor.count);
        } else {
            std::cerr << "Unsupported index component type in glTF mesh" << std::endl;
            std::terminate();
        }
    }

    void load_gltf_mesh(const tinygltf::Model& model, std::vector<GLuint>& indices, std::vector<GLfloat>& positions, std::vector<GLfloat>& texcoords)
    {
#if DEBUG
        if (model.meshes.size() != 1) {
            std::cerr << "Model must contain only one mesh" << std::endl;
            std::terminate();
        }
#endif
        const tinygltf::Mesh& _mesh = model.meshes[0];        
#if DEBUG
        if (_mesh.primitives.size() != 1) {
            std::cerr << "Mesh must contain only one primitive" << std::endl;
            std::terminate();
        }
#endif
        const tinygltf::Primitive& _primitive = _mesh.primitives[0];
        load_gltf_positions(model, _primitive, positions);
        load_gltf_texcoords(model, _primitive, texcoords);
        load_gltf_indices(model, _primitive, indices);
        

        
        
        
        
    }

    void load_gltf_texture(const tinygltf::Model& model, std::vector<GLubyte>& pixels, GLuint& channels, GLuint& width, GLuint& height)
    {
#if DEBUG
        if (model.textures.size() != 1) {
            std::cerr << "Only one texture must be provided inside glTF file" << std::endl;
            std::terminate();
        }
#endif
        const tinygltf::Texture& _texture = model.textures[0];
#if DEBUG
        if (_texture.source <= -1) {
            std::cerr << "Invalid texture source inside glTF file" << std::endl;
            std::terminate();
        }
#endif
        const tinygltf::Image& _image = model.images[_texture.source];
#if DEBUG
        if (image.bits != 8) {
            std::cerr << "Invalid texture bits different than unsigned char in glTF file" << std::endl;
            std::terminate();
        }
#endif
        width = _image.width;
        height = _image.height;
        channels = _image.component;
        pixels = _image.image;
    }

    void load_gltf(const std::filesystem::path& path, std::vector<GLuint>& indices, std::vector<GLfloat>& positions, std::vector<GLfloat>& texcoords, std::vector<GLubyte>& pixels, GLuint& channels, GLuint& width, GLuint& height)
    {
        tinygltf::Model _model;
        tinygltf::TinyGLTF loader;
        std::string _error_str;
        std::string _warn_str;
        bool _result = loader.LoadBinaryFromFile(&_model, &_error_str, &_warn_str, path.generic_string());
#if DEBUG
        if (!_warn_str.empty()) {
            std::cout << "While loading glTF : '" << _warn_str << "'" << std::endl;
        }
        if (!_error_str.empty()) {
            std::cerr << "While loading glTF : '" << _error_str << "'" << std::endl;
        }
        if (!_result) {
            std::cout << "Failed to load glTF '" << path.generic_string() << "'" << std::endl;
            std::terminate();
        }
#endif
        std::cout << "loading gltf" << std::endl;
        load_gltf_mesh(_model, indices, positions, texcoords);
        std::cout << "loading texture gltf" << std::endl;
        load_gltf_texture(_model, pixels, channels, width, height);
        std::cout << "loading gltf 2" << std::endl;
    }

    GLuint create_vertex_shader()
    {
        GLint _log_length;
        GLint _result = GL_FALSE;
        GLuint _vertex_id = glCreateShader(GL_VERTEX_SHADER);
        const GLchar* _vertex_source_ptr = vertex_shader.data();
        glShaderSource(_vertex_id, 1, &_vertex_source_ptr, NULL);
        glCompileShader(_vertex_id);
        glGetShaderiv(_vertex_id, GL_COMPILE_STATUS, &_result);
        glGetShaderiv(_vertex_id, GL_INFO_LOG_LENGTH, &_log_length);
#if DEBUG
        if (!_result && _log_length > 0) {
            std::vector<GLchar> _result_error_msg(_log_length + 1);
            glGetShaderInfoLog(_vertex_id, _log_length, NULL, &_result_error_msg[0]);
            std::cerr << "Invalid vertex shader '" << std::string(&_result_error_msg[0]) << "'" << std::endl;
            std::terminate();
        }
#endif
        return _vertex_id;
    }

    GLuint create_fragment_shader()
    {
        GLint _log_length;
        GLint _result = GL_FALSE;
        GLuint _fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
        const GLchar* _fragment_source_ptr = fragment_shader.data();
        glShaderSource(_fragment_id, 1, &_fragment_source_ptr, NULL);
        glCompileShader(_fragment_id);
        glGetShaderiv(_fragment_id, GL_COMPILE_STATUS, &_result);
        glGetShaderiv(_fragment_id, GL_INFO_LOG_LENGTH, &_log_length);
#if DEBUG
        if (!_result && _log_length > 0) {
            std::vector<GLchar> _result_error_msg(_log_length + 1);
            glGetShaderInfoLog(_fragment_id, _log_length, NULL, &_result_error_msg[0]);
            std::cerr << "Invalid fragment shader '" << std::string(&_result_error_msg[0]) << "'" << std::endl;
            std::terminate();
        }
#endif
        return _fragment_id;
    }

    GLuint create_program(const GLuint vertex_id, const GLuint fragment_id)
    {
        GLint _log_length;
        GLint _result = GL_FALSE;
        GLuint _program_id = glCreateProgram();
        glAttachShader(_program_id, vertex_id);
        glAttachShader(_program_id, fragment_id);
        glLinkProgram(_program_id);
        glGetProgramiv(_program_id, GL_LINK_STATUS, &_result);
        glGetProgramiv(_program_id, GL_INFO_LOG_LENGTH, &_log_length);
#if DEBUG
        if (!_result && _log_length > 0) {
            std::vector<char> _result_error_msg(_log_length + 1);
            glGetProgramInfoLog(_program_id, _log_length, NULL, &_result_error_msg[0]);
            std::cerr << "Invalid program '" << std::string(&_result_error_msg[0]) << "'" << std::endl;
            std::terminate();
        }
#endif
        glDetachShader(_program_id, vertex_id);
        glDetachShader(_program_id, fragment_id);
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);
        return _program_id;
    }

    GLuint create_vertex_array()
    {
        GLuint _array_id;
        glGenVertexArrays(1, &_array_id);
        glBindVertexArray(_array_id);
        return _array_id;
    }

    GLuint create_positions_buffer(const std::vector<GLfloat>& positions)
    {
        GLuint _positions_id;
        GLfloat* _positions_ptr = const_cast<GLfloat*>(positions.data());
        glGenBuffers(1, &_positions_id);
        glBindBuffer(GL_ARRAY_BUFFER, _positions_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * positions.size(), _positions_ptr, GL_STATIC_DRAW);
        return _positions_id;
    }

    GLuint create_texcoords_buffer(const std::vector<GLfloat>& texcoords)
    {
        GLuint _texcoords_id;
        GLfloat* _texcoords_ptr = const_cast<GLfloat*>(texcoords.data());
        glGenBuffers(1, &_texcoords_id);
        glBindBuffer(GL_ARRAY_BUFFER, _texcoords_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * texcoords.size(), _texcoords_ptr, GL_STATIC_DRAW);
        return _texcoords_id;
    }

    GLuint create_elements_buffer(const std::vector<GLuint>& indices)
    {
        GLuint _elements_id;
        glGenBuffers(1, &_elements_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elements_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &(indices[0]), GL_STATIC_DRAW);
        return _elements_id;
    }

    GLuint create_texture(const std::vector<GLubyte>& pixels, const int channels, const int width, const int height)
    {
        GLuint _texture_id;
        glGenTextures(1, &_texture_id);
        glBindTexture(GL_TEXTURE_2D, _texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        const GLubyte* _pixels_ptr = &(pixels[0]);
        switch (channels) {
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, &(pixels[0]));
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(pixels[0]));
            break;
        default:
#if DEBUG
            std::cerr << "Invalid channels count, must be 3 or 4" << std::endl;
            std::terminate();
#else
            break;
#endif
        }
        return _texture_id;
    }

    void bind_positions_buffer(const GLuint program_id, const GLuint array_id, const GLuint positions_id)
    {
        GLint _location = glGetAttribLocation(program_id, "vert_position");
#if DEBUG
        if (_location < 0) {
            std::cerr << "Invalid attribute location 'vert_position'" << std::endl;
            std::terminate();
        }
#endif
        glBindVertexArray(array_id);
        glBindBuffer(GL_ARRAY_BUFFER, positions_id);
        glVertexAttribPointer(_location, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(_location);
    }

    void bind_texcoords_buffer(const GLuint program_id, const GLuint array_id, const GLuint texcoords_id)
    {
        GLint _location = glGetAttribLocation(program_id, "vert_texcoord");
#if DEBUG
        if (_location < 0) {
            std::cerr << "Invalid attribute location 'vert_texcoord'" << std::endl;
            std::terminate();
        }
#endif
        glBindVertexArray(array_id);
        glBindBuffer(GL_ARRAY_BUFFER, texcoords_id);
        glVertexAttribPointer(_location, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(_location);
    }

    void bind_texture(const GLuint program_id, const GLuint texture_id)
    {
        constexpr GLuint _sampler = 0;
        GLint _location = glGetUniformLocation(program_id, "uniform_color");
#if DEBUG
        if (_location < 0) {
            std::cerr << "Invalid attribute location 'uniform_color'" << std::endl;
            std::terminate();
        }
#endif
        glUniform1i(_location, _sampler);
        glActiveTexture(GL_TEXTURE0 + _sampler);
        glBindTexture(GL_TEXTURE_2D, texture_id);
    }

    void draw_elements(const GLuint program_id, const GLuint array_id, const GLuint count)
    {
        glUseProgram(program_id);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glBindVertexArray(array_id);
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
    }

    void graphics_assert()
    {
#if DEBUG
        GLenum _gl_err;
        while ((_gl_err = glGetError()) != GL_NO_ERROR) {
            std::string _brief, _description;
            switch (_gl_err) {
            case GL_INVALID_ENUM:
                _brief = "Invalid OpenGL enum";
                _description = "This is given when an enumeration parameter is not a legal enumeration for that function. this is given only for local problems; if the spec allows the enumeration in certain circumstances, where other parameters or state dictate those circumstances, then 'Invalid operation' is the _compilation_result instead";
                break;
            case GL_INVALID_VALUE:
                _brief = "Invalid OpenGL value";
                _description = "This is given when a value parameter is not a legal value for that function. this is only given for local problems; if the spec allows the value in certain circumstances, where other parameters or state dictate those circumstances, then 'Invalid operation' is the _compilation_result instead";
                break;
            case GL_INVALID_OPERATION:
                _brief = "Invalid OpenGL operation";
                _description = "This is given when the set of state for a command is not legal for the parameters given to that command. it is also given for commands where combinations of parameters define what the legal parameters are";
                break;
            case GL_OUT_OF_MEMORY:
                _brief = "OpenGL out of memory";
                _description = "This is given when performing an operation that can allocate memory, and the memory cannot be allocated. the _compilation_results of OpenGL functions that return this void are undefined; it is allowable for partial execution of an operation to happen in this circumstance";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                _brief = "Invalid OpenGL framebuffer operation";
                _description = "This is given when doing anything that would attempt to read from or write/render to a framebuffer that is not complete";
                break;
            default:
                _brief = "Unknown OpenGL error";
                _description = "This error hasn't been coded into glite yet, please sumbit an issue report on github.com/127Soft/glite :)";
                break;
            }
            std::cerr << "Invalid OpenGL _compilation_result '" << _brief << "' (" << _description << ")" << std::endl;
            std::terminate();
        }
#endif
    }

}

static std::optional<GLuint> program_id = std::nullopt;
static std::optional<GLuint> array_id = std::nullopt;
static std::optional<GLuint> positions_id = std::nullopt;
static std::optional<GLuint> texcoords_id = std::nullopt;
static std::optional<GLuint> elements_id = std::nullopt;
static std::optional<GLuint> texture_id = std::nullopt;
static std::optional<GLuint> vertices_count = std::nullopt;

void load_unlit_shader()
{
    if (!program_id.has_value()) {
        GLuint _vertex_id = detail::create_vertex_shader();
        GLuint _fragment_id = detail::create_fragment_shader();
        program_id = detail::create_program(_vertex_id, _fragment_id);
        detail::graphics_assert();
    }
}

void load_model_gltf(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) {
        std::cerr << "The file '" << path.generic_string() << "' does not exist" << std::endl;
        std::terminate();
    }
    if (!array_id.has_value() || !positions_id.has_value() || !texcoords_id.has_value() || !elements_id.has_value() || !texture_id.has_value() || !vertices_count.has_value()) {
        std::vector<GLuint> _indices;
        std::vector<GLfloat> _positions;
        std::vector<GLfloat> _texcoords;
        std::vector<GLubyte> _pixels;
        GLuint _channels, _width, _height;
        detail::load_gltf(path, _indices, _positions, _texcoords, _pixels, _channels, _width, _height);
        vertices_count = _positions.size() / 3;
        array_id = detail::create_vertex_array();
        positions_id = detail::create_positions_buffer(_positions);
        texcoords_id = detail::create_texcoords_buffer(_texcoords);
        elements_id = detail::create_elements_buffer(_indices);
        texture_id = detail::create_texture(_pixels, _channels, _width, _height);
        detail::graphics_assert();
    }
}

void clear(const glm::vec4 color, const bool depth)
{
    // BIND FRAMEBUFFER ! viewport avant ou apres ??
    glm::vec2 _screen_size = get_screen_size();
    float _fov_rad = glm::radians(detail::perspective_fov);
    float _aspect_ratio = _screen_size.x / _screen_size.y;
    detail::camera_projection = glm::perspective(_fov_rad, _aspect_ratio, detail::perspective_near, detail::perspective_far);
    GLbitfield _bits = depth ? GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT : GL_COLOR_BUFFER_BIT;
    glViewport(0, 0, _screen_size.x, _screen_size.y);
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(_bits);
    detail::graphics_assert();
}

void set_perspective(const float fov, const float near, const float far)
{
}

void rotate_camera(const glm::vec2 rotation)
{
}

void draw()
{
    if (program_id.has_value() && array_id.has_value() && positions_id.has_value() && texcoords_id.has_value()) {
        detail::bind_positions_buffer(program_id.value(), array_id.value(), positions_id.value());
        detail::bind_texcoords_buffer(program_id.value(), array_id.value(), texcoords_id.value());
        detail::bind_texture(program_id.value(), texture_id.value());
        detail::draw_elements(program_id.value(), array_id.value(), vertices_count.value());
        detail::graphics_assert();
    }
}
}