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

#include "../Engine/mesh_loader.h"

#include "../Engine/ColorMaterial.h"
#include "../Engine/PhongMaterial.h"

void SimpleShapeApplication::init() {
    xe::ColorMaterial::init();
    xe::PhongMaterial::init();
    
    auto program = xe::utils::create_program(
            {{GL_VERTEX_SHADER,   std::string(PROJECT_DIR) + "/shaders/base_vs.glsl"},
             {GL_FRAGMENT_SHADER, std::string(PROJECT_DIR) + "/shaders/base_fs.glsl"}});

    if (!program) {
        std::cerr << "Invalid program" << std::endl;
        exit(-1);
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
    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);

    glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);    
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, u_pvm_buffer_);

    set_controler(new CameraControler(camera()));

    #pragma endregion

    #pragma region --- Lights setup ---
    add_light(xe::PointLight(glm::vec3(0.0f, 0.0f, 0.3f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 1.0f));
    add_ambient(glm::vec3(0.5f, 0.5f, 0.5f));

    glGenBuffers(1, &u_light_buffer_);
    glBindBuffer(GL_UNIFORM_BUFFER, u_light_buffer_);
    
    size_t light_buffer_size = sizeof(glm::vec3) + sizeof(unsigned int) + 24 * sizeof(xe::PointLight);
    glBufferData(GL_UNIFORM_BUFFER, light_buffer_size, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, u_light_buffer_);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    #pragma endregion

    #pragma region --- Mesh setup ---
    auto square = xe::load_mesh_from_obj(std::string(ROOT_DIR) + "/Models/square.obj",
                                          std::string(ROOT_DIR) + "/Models");

    if (square) {
        add_submesh(square);
    } else {
        std::cerr << "Cannot load the mesh" << std::endl;
    }

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
    auto VM = V_ * M_;
    
    auto R = glm::mat3(VM);
    auto N = glm::mat3(glm::cross(R[1], R[2]), glm::cross(R[2], R[0]), glm::cross(R[0], R[1]));

    glBindBuffer(GL_UNIFORM_BUFFER, u_pvm_buffer_);
  
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &PVM[0]);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &VM[0]);
    
    size_t offset_N = 2 * sizeof(glm::mat4);
    glBufferSubData(GL_UNIFORM_BUFFER, offset_N, 3 * sizeof(float), &N[0]); 
    glBufferSubData(GL_UNIFORM_BUFFER, offset_N + 4 * sizeof(float), 3 * sizeof(float), &N[1]);
    glBufferSubData(GL_UNIFORM_BUFFER, offset_N + 8 * sizeof(float), 3 * sizeof(float), &N[2]);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    auto V = camera_->view();
    
    for (auto &light : p_lights_) {
        glm::vec4 pos_ws = glm::vec4(light.position_in_ws, 1.0f);
        glm::vec4 pos_vs = V * pos_ws;
        light.position_in_vs = glm::vec3(pos_vs);
    }

    glBindBuffer(GL_UNIFORM_BUFFER, u_light_buffer_);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4), &ambient_[0]); // + 16B (12 + 4 padding w formie ustawienia ambbientu jako vec4)
    
    unsigned int n_lights = p_lights_.size();
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec4), sizeof(unsigned int), &n_lights); // + 16B (w sumie 32B)

    // ambient   -> offset 0
    // n_lights  -> offset 16 (bo ambient wyrównany do 16)
    // p_light[] -> offset 32 (bo n_lights też ma 16 a więc 32 w sumie)
    
    size_t lights_offset = 2 * sizeof(glm::vec4);
    for (const auto &light : p_lights_) {
        glBufferSubData(GL_UNIFORM_BUFFER, lights_offset, sizeof(glm::vec4), &light.position_in_vs); // +16B
        glBufferSubData(GL_UNIFORM_BUFFER, lights_offset + 16, sizeof(glm::vec4), &light.color); // +16B (32B w sumie)
        glBufferSubData(GL_UNIFORM_BUFFER, lights_offset + 32, sizeof(float), &light.intensity); // +4B (36B w sumie)
        glBufferSubData(GL_UNIFORM_BUFFER, lights_offset + 36, sizeof(float), &light.radius); // +4B (40B w sumie)

        // W standardzie std140 struktura w tablicy musi mieć rozmiar będący wielokrotnością 16 (vec4).
        // Nasze dane zajmują 40 bajtów (16+16+4+4), więc najbliższa wielokrotność 16 to 48.
        lights_offset += 48; 
    }
    
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