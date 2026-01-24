#include "glm/gtx/string_cast.hpp"

#include "Material.h"
#include "PhongMaterial.h"

#include "spdlog/spdlog.h"

namespace xe {
    GLuint PhongMaterial::material_uniform_buffer_ = 0u;
    GLuint PhongMaterial::shader_ = 0u;
    GLint  PhongMaterial::uniform_map_Kd_location_ = 0;

    void PhongMaterial::bind() {
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, material_uniform_buffer_);

        glUseProgram(program());

        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4), &Kd[0]); // + 16B (16B)
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec4), sizeof(glm::vec4), &Ka[0]) ;// + 16B (32B);
        glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::vec4), sizeof(glm::vec4), &Ks[0]); // + 16B (48B);
        glBufferSubData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::vec4), sizeof(float), &Ns); // + 4B (52B);

        GLint use_map_Kd = (map_Kd > 0) ? 1 : 0;
        glBufferSubData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::vec4) + sizeof(float), sizeof(GLint), &use_map_Kd);

        glBindBuffer(GL_UNIFORM_BUFFER, 0u);

        if (map_Kd > 0) {
            glUniform1i(uniform_map_Kd_location_, map_Kd_unit);
            glActiveTexture(GL_TEXTURE0 + map_Kd_unit);
            glBindTexture(GL_TEXTURE_2D, map_Kd);
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
        
        glGenBuffers(1, &material_uniform_buffer_);

        glBindBuffer(GL_UNIFORM_BUFFER, material_uniform_buffer_);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) + sizeof(glm::vec4) + sizeof(glm::vec4) + sizeof(float) + sizeof(GLint), nullptr, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0u);
    }
}
