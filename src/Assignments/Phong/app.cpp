//
// Created by pbialas on 25.09.2020.
//

#include "app.h"

#include <iostream>
#include <vector>
#include <tuple>
#include <cstring>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp> 

#include "Application/utils.h"
#include "../Engine/Material.h"

#define STB_IMAGE_IMPLEMENTATION 1
#include "3rdParty/stb/stb_image.h"

const GLuint POSITION_ATTR = 0;
const GLuint TEXCOORD_ATTR = 1;

void SimpleShapeApplication::init() {
    xe::ColorMaterial::init();
    
    auto program = xe::utils::create_program(
            {{GL_VERTEX_SHADER,   std::string(PROJECT_DIR) + "/shaders/base_vs.glsl"},
             {GL_FRAGMENT_SHADER, std::string(PROJECT_DIR) + "/shaders/base_fs.glsl"}});

    if (!program) {
        std::cerr << "Invalid program" << std::endl;
        exit(-1);
    }

    std::vector<GLfloat> vertices = {
         0.0f,  0.0f, 1.0f,   0.000f, 0.000f,  // 0
        -0.5f,  0.5f, 0.0f,   0.191f, 0.500f,  // 1
        -0.5f, -0.5f, 0.0f,   0.500f, 0.191f,  // 2

         0.0f,  0.0f, 1.0f,   1.000f, 0.000f,  // 3
        -0.5f, -0.5f, 0.0f,   0.500f, 0.191f,  // 4
         0.5f, -0.5f, 0.0f,   0.809f, 0.500f,  // 5

         0.0f,  0.0f, 1.0f,   1.000f, 1.000f,  // 6
         0.5f, -0.5f, 0.0f,   0.809f, 0.500f,  // 7
         0.5f,  0.5f, 0.0f,   0.500f, 0.809f,  // 8

         0.0f,  0.0f, 1.0f,   0.000f, 1.000f,  // 9
         0.5f,  0.5f, 0.0f,   0.500f, 0.809f,  // 10
        -0.5f,  0.5f, 0.0f,   0.191f, 0.500f,  // 11

        -0.5f,  0.5f, 0.0f,   0.191f, 0.500f,  // 12
        -0.5f, -0.5f, 0.0f,   0.500f, 0.191f,  // 13
         0.5f, -0.5f, 0.0f,   0.809f, 0.500f,  // 14
         0.5f,  0.5f, 0.0f,   0.500f, 0.809f   // 15
    };

    std::vector<GLushort> indices = {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8,
        9, 10, 11,

        14, 13, 12,
        15, 14, 12
    };
        
    stbi_set_flip_vertically_on_load(true);
    GLuint texture_id;
    GLint width, height, channels;
    std::string texture_file = std::string(ROOT_DIR) + "/Models/multicolor.png";
    auto img = stbi_load(texture_file.c_str(), &width, &height, &channels, 0);
    if (!img) {
        std::cerr << "Could not read image from file: " << texture_file << std::endl;
    } else {
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;
        
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, img);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(img);
    }
    
    #pragma region --- Camera setup ---
    
    set_camera(new Camera);
    auto[w, h] = frame_buffer_size();
    GLfloat aspect_ = (float)w / (float)h;
    GLfloat fov_ = glm::radians(45.0f);
    GLfloat near_ = 0.1f;
    GLfloat far_ = 100.0f;
    camera_->perspective(fov_, aspect_, near_, far_);
    
    glm::vec3 camera_pos    = glm::vec3(0.0f, -1.0f, 3.0f); 
    glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up_vector     = glm::vec3(0.0f, 1.0f, 0.0f);
    camera_->look_at(camera_pos, camera_target, up_vector);

    glGenBuffers(1, &u_pvm_buffer_);
    OGL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_));

    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);    
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, u_pvm_buffer_);

    set_controler(new CameraControler(camera()));

    #pragma endregion

    #pragma region --- Pyramid mesh setup ---
    auto pyramid = new xe::Mesh;

    // VERTEX buffer
    pyramid->allocate_vertex_buffer(vertices.size() * sizeof(GLfloat), GL_STATIC_DRAW);
    pyramid->load_vertices(0, vertices.size() * sizeof(GLfloat), vertices.data());
    pyramid->vertex_attrib_pointer(POSITION_ATTR, 3, GL_FLOAT, 5 * sizeof(GLfloat), 0);

    // INDEX buffer
    pyramid->allocate_index_buffer(indices.size() * sizeof(GLushort), GL_STATIC_DRAW);
    pyramid->load_indices(0, indices.size() * sizeof(GLushort), indices.data());

    // Textures and materials
    pyramid->vertex_attrib_pointer(TEXCOORD_ATTR, 2, GL_FLOAT, 5 * sizeof(GLfloat), 3 * sizeof(GLfloat));

    auto mat_tex = new xe::ColorMaterial(glm::vec4(1.0f));
    
    if (texture_id > 0) {
        mat_tex->set_texture(texture_id);
        mat_tex->set_texture_unit(0);
    }

    pyramid->add_submesh(0, indices.size(), mat_tex);
    add_submesh(pyramid);

    #pragma endregion

    glClearColor(0.81f, 0.81f, 0.8f, 1.0f);
    glViewport(0, 0, w, h);
    glUseProgram(program);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

//This functions is called every frame and does the actual rendering.
void SimpleShapeApplication::frame() {

    glm::mat4 M_ = glm::mat4(1.0f); 
    auto P_ = camera_->projection();
    auto V_ = camera_->view();

    auto PVM = P_ * V_ * M_;

    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &PVM[0]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    for (auto m : meshes_)
        m->draw();
}

void SimpleShapeApplication::framebuffer_resize_callback(int w, int h) {
    Application::framebuffer_resize_callback(w, h);
    glViewport(0, 0, w, h); 

   if (camera_) {
        camera_->set_aspect((float) w / h);
    }
}

void SimpleShapeApplication::scroll_callback(double xoffset, double yoffset) {
    Application::scroll_callback(xoffset, yoffset);
    camera()->zoom(yoffset / 30.0f);
}