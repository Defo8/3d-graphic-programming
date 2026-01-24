#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

#include "Application/utils.h"

#include "Material.h"

namespace xe { 
    class PhongMaterial : public Material {
    public:
        glm::vec4 Kd;
        glm::vec4 Ka;
        glm::vec4 Ks;
        float Ns;

        GLuint map_Kd;
        const GLuint map_Kd_unit;

        PhongMaterial(const glm::vec4 Kd): Kd(Kd), map_Kd(0), map_Kd_unit(0) {}

        void bind();

        static void init();

        static GLuint program() { return shader_; }

        GLuint get_map_Kd() const { return map_Kd; }
        void set_map_Kd(GLuint tex) { map_Kd = tex; }

    private:
        static GLuint shader_;
        static GLuint material_uniform_buffer_;
        static GLint uniform_map_Kd_location_;        
    };
}

