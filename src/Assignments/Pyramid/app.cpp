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
    // A utility function that reads the shader sources, compiles them and creates the program object
    // As everything in OpenGL we reference program by an integer "handle".
    auto program = xe::utils::create_program(
            {{GL_VERTEX_SHADER,   std::string(PROJECT_DIR) + "/shaders/base_vs.glsl"},
             {GL_FRAGMENT_SHADER, std::string(PROJECT_DIR) + "/shaders/base_fs.glsl"}});

    if (!program) {
        std::cerr << "Invalid program" << std::endl;
        exit(-1);
    }

    std::vector<GLfloat> vertices = {
            -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.5f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
            0.0f, 0.5f, 0.0f,  1.0f, 0.0f, 0.0f,

            -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
            0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,

            -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 
        }; 

    std::vector<GLushort> indices = {
            0, 1, 2,  
            3, 4, 5,  
            5, 4, 6   
        };

    #pragma region --- Transformations uniform block setup (std140 layout, binding=1) ---
    
    auto[w, h] = frame_buffer_size();
    GLfloat aspect = (GLfloat)w / (GLfloat)h;
    GLfloat fov = glm::radians(45.0f); // Pole widzenia
    GLfloat near_plane = 0.1f;
    GLfloat far_plane = 100.0f;
    glm::mat4 Projection = glm::perspective(fov, aspect, near_plane, far_plane);

    glm::vec3 camera_pos   = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 camera_target= glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up_vector    = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 View = glm::lookAt(camera_pos, camera_target, up_vector);

    glm::mat4 Model = glm::mat4(1.0f);

    glm::mat4 PVM = Projection * View * Model;

    GLuint uniform_buffer_Transformation_handle;
    glGenBuffers(1, &uniform_buffer_Transformation_handle);
    OGL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_Transformation_handle));
    
    //(mat4 to 16 floatów, czyli 64 bajty)
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), &PVM[0], GL_STATIC_DRAW);    
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, uniform_buffer_Transformation_handle);

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

    // This setups a Vertex Array Object (VAO) that  encapsulates
    // the state of all vertex buffers needed for rendering
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    OGL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_handle));
    glBindBuffer(GL_ARRAY_BUFFER, v_buffer_handle);

    // This indicates that the data for attribute 0 should be read from a vertex buffer.
    glEnableVertexAttribArray(POSITION_ATTR);
    // and this specifies how the VERTEX data is layout in the buffer.
    glVertexAttribPointer(POSITION_ATTR, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<GLvoid *>(0));

    glEnableVertexAttribArray(COLOR_ATTR);
    // and this specifies how the COLOR data is layout in the buffer.
    glVertexAttribPointer(COLOR_ATTR, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<GLvoid *>(3 * sizeof(GLfloat)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //end of vao "recording"

    // Setting the background color of the rendering window,
    // I suggest not to use white or black for better debuging.
    glClearColor(0.81f, 0.81f, 0.8f, 1.0f);

    // Wywołanie frame_buffer_size ponownie, aby ustawić viewport
    // (wcześniej użyliśmy go tylko do obliczenia Aspect Ratio)
    glViewport(0, 0, w, h);

    glUseProgram(program);
}

//This functions is called every frame and does the actual rendering.
void SimpleShapeApplication::frame() {
    // Binding the VAO will setup all the required vertex buffers.
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
}
