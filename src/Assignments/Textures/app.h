//
// Created by pbialas on 05.08.2020.
//

#pragma once

#include <vector>

#include "Application/application.h"
#include "Application/utils.h"
#include "../Engine/Mesh.h"
#include "camera.h"
#include "camera_controler.h"

#include "glad/gl.h"
#include <glm/glm.hpp>



class SimpleShapeApplication : public xe::Application
{
public:
    SimpleShapeApplication(int width, int height, std::string title, bool debug) : Application(width, height, title, debug) {}

    void init() override;

    void frame() override;

    void framebuffer_resize_callback(int w, int h) override;

    void set_camera(Camera *camera) { camera_ = camera; }

    void set_controler(CameraControler *controler) { controler_ = controler; }

    void scroll_callback(double xoffset, double yoffset) override;

    Camera *camera() { return camera_; }

    ~SimpleShapeApplication() 
    {
        if (camera_) {
            delete camera_;
        }
    }

    void mouse_button_callback(int button, int action, int mods) override {
        Application::mouse_button_callback(button, action, mods);
        if (controler_) {
            double x, y;
            glfwGetCursorPos(window_, &x, &y);
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
                controler_->LMB_pressed(x, y);
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
                controler_->LMB_released(x, y);
        }
    }

    void cursor_position_callback(double x, double y) override {
        Application::cursor_position_callback(x, y);
        if (controler_) {
            controler_->mouse_moved(x, y);
        }
    }

    void add_submesh(xe::Mesh *mesh) {
        meshes_.push_back(mesh);
    }

private:
    GLuint vao_;
    
    GLuint u_pvm_buffer_;

    Camera *camera_;

    CameraControler *controler_;

    std::vector<xe::Mesh*> meshes_; 
};