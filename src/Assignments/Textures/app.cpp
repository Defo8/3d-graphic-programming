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

const GLuint POSITION_ATTR = 0;
const GLuint COLOR_ATTR = 1;

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
        -0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    std::vector<GLushort> indices = {
            0, 1, 2,  
            0, 2, 3,  
            
            0, 4, 1,
            
            0, 3, 4,

            2, 4, 3,

            1, 4, 2
        };

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
    pyramid->vertex_attrib_pointer(POSITION_ATTR, 3, GL_FLOAT, 3 * sizeof(GLfloat), 0);

    // INDEX buffer
    pyramid->allocate_index_buffer(indices.size() * sizeof(GLushort), GL_STATIC_DRAW);
    pyramid->load_indices(0, indices.size() * sizeof(GLushort), indices.data());

    auto mat_red    = new xe::ColorMaterial(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    auto mat_green  = new xe::ColorMaterial(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    auto mat_blue   = new xe::ColorMaterial(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    auto mat_yellow = new xe::ColorMaterial(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
    auto mat_cyan   = new xe::ColorMaterial(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
    auto mat_magenta= new xe::ColorMaterial(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));    

    pyramid->add_submesh(0, 3, mat_red);      
    pyramid->add_submesh(3, 6, mat_green);   
    pyramid->add_submesh(6, 9, mat_blue);     
    pyramid->add_submesh(9, 12, mat_yellow);  
    pyramid->add_submesh(12, 15, mat_cyan); 
    pyramid->add_submesh(15, 18, mat_magenta); 

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