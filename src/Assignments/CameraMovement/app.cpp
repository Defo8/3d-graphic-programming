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

const GLuint POSITION_ATTR = 0;
const GLuint COLOR_ATTR = 1;

void SimpleShapeApplication::init() {
    auto program = xe::utils::create_program(
            {{GL_VERTEX_SHADER,   std::string(PROJECT_DIR) + "/shaders/base_vs.glsl"},
             {GL_FRAGMENT_SHADER, std::string(PROJECT_DIR) + "/shaders/base_fs.glsl"}});

    if (!program) {
        std::cerr << "Invalid program" << std::endl;
        exit(-1);
    }

    std::vector<GLfloat> vertices = {
            -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,

            0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        }; 

    std::vector<GLushort> indices = {
            0, 1, 2,  
            0, 2, 3,  
            
            0, 4, 1,
            
            0, 3, 4,

            2, 4, 3,

            1, 4, 2
        };
    
    #pragma region --- Transformations uniform block setup (std140 layout, binding=1) ---
    
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

    #pragma region --- Modifier uniform block setup (std140 layout, binding=0) ---
    
    float strength = 0.8f;
    float color[3] = {1.0f, 1.0f, 1.0f};
    GLuint uniform_buffer_Modifier_handle;
    glGenBuffers(1, &uniform_buffer_Modifier_handle);
    OGL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_Modifier_handle));
    glBufferData(GL_UNIFORM_BUFFER, 8 * sizeof(GLfloat), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GLfloat), &strength); 
    glBufferSubData(GL_UNIFORM_BUFFER, 4 * sizeof(GLfloat), 3 * sizeof(GLfloat), &color); 
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer_Modifier_handle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    #pragma endregion

    #pragma region --- Index buffer setup ---

    GLuint index_buffer_handle;    
    glGenBuffers(1, &index_buffer_handle); 
    OGL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_handle));
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    #pragma endregion
    
    #pragma region --- Vertex Array Object (VAO) setup ---

    GLuint v_buffer_handle;
    glGenBuffers(1, &v_buffer_handle);
    OGL_CALL(glBindBuffer(GL_ARRAY_BUFFER, v_buffer_handle));
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    #pragma endregion

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    OGL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_handle));
    glBindBuffer(GL_ARRAY_BUFFER, v_buffer_handle);
    glEnableVertexAttribArray(POSITION_ATTR);
    glVertexAttribPointer(POSITION_ATTR, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<GLvoid *>(0));
    glEnableVertexAttribArray(COLOR_ATTR);
    glVertexAttribPointer(COLOR_ATTR, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<GLvoid *>(3 * sizeof(GLfloat)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

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

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
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