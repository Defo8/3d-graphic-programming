#include "glm/gtx/string_cast.hpp"

#include "Material.h"
#include "PhongMaterial.h"

#include "spdlog/spdlog.h"

namespace xe {
    GLuint PhongMaterial::color_uniform_buffer_ = 0u;
    GLuint PhongMaterial::shader_ = 0u;
    GLint  PhongMaterial::uniform_map_Kd_location_ = 0;

    void PhongMaterial::bind() {
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, color_uniform_buffer_);
        glUseProgram(program());
        glBindBuffer(GL_UNIFORM_BUFFER, color_uniform_buffer_);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4), &color_[0]);
        GLint use_map_Kd = (texture_ > 0) ? 1 : 0;
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec4), sizeof(GLint), &use_map_Kd);
        glBindBuffer(GL_UNIFORM_BUFFER, 0u);

        if (texture_ > 0) {
            glUniform1i(uniform_map_Kd_location_, texture_unit_);
            glActiveTexture(GL_TEXTURE0 + texture_unit_);
            glBindTexture(GL_TEXTURE_2D, texture_);
        }

        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec4), sizeof(GLint), &use_map_Kd);
    }

    void PhongMaterial::init() {
        auto program = xe::utils::create_program(
                {{GL_VERTEX_SHADER,   std::string(PROJECT_DIR) + "/shaders/phong_vs.glsl"},
                 {GL_FRAGMENT_SHADER, std::string(PROJECT_DIR) + "/shaders/phong_fs.glsl"}});
        if (!program) {
            std::cerr << "Invalid program" << std::endl;
            exit(-1);
        }

        shader_ = program;
        
        glGenBuffers(1, &color_uniform_buffer_);

        glBindBuffer(GL_UNIFORM_BUFFER, color_uniform_buffer_);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) + sizeof(GLint), nullptr, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0u);
    }
}
