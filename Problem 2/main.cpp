#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include "include/utility.hpp"
#include "include/shader.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "include/cube.hpp"
#include "include/skybox.hpp"
#include "include/camera.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
bool DEBUG=false;

glm::vec3 initialCameraPos = glm::vec3(0.0f, 0.0f, 6.0f); // +Z
glm::vec3 initialUpperVec = glm::vec3(0.0f, 1.0f, 0.0f); //+Y
glm::vec3 initialLookAt = glm::vec3(0.0f, 0.0f, -1.0f);
Camera camera(initialCameraPos, initialUpperVec, initialLookAt);

void reshape_viewport(GLFWwindow *w, int width, int height);
void cursor_callback(GLFWwindow* window, double x, double y);
void scroll_callback(GLFWwindow* w, double x, double y);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
GLuint load_texture(char const* path);
GLuint load_cube_texture(std::vector< std::string > paths, std::string base);
std::vector<std::string> skybox = {"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"};
std::string skybox_base_path = "textures/skybox/";

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* w = glfwCreateWindow(800, 600, "Interactive Camera Model", NULL, NULL);
    glfwMakeContextCurrent(w);
    glewExperimental = GL_TRUE;
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 800, 600);
    glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(w, cursor_callback);
    glfwSetFramebufferSizeCallback(w, reshape_viewport);
    glfwSetScrollCallback(w, scroll_callback);
    glfwSetKeyCallback(w, key_callback);

    GLuint texture = load_texture("textures/mural.jpg");
    GLuint skyTexture = load_cube_texture(skybox, skybox_base_path);
    Shader *shdr = new Shader("shaders/vertexshad.glsl", "shaders/fragment.glsl");
    Shader *skyShdr = new Shader("shaders/skyboxvertex.glsl", "shaders/skyboxfragment.glsl");

    Cube cube(true);
    SkyBox skybox;
    
    while (!glfwWindowShouldClose(w)){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        
        camera.updateTimeCounter();
        glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 800.0f/600.0f, 0.1f, 100.0f);

        //for camera we will exclusively modify only the view matrix
        camera.updateLookAt();
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        shdr->use();
        shdr->setMatrix4f("view", view);
        shdr->setMatrix4f("model", model);
        glm::mat4 normalMatrix = glm::transpose(glm::inverse(model));
        shdr->setMatrix4f("normalMatrix", normalMatrix);
        shdr->setVec3f("viewLoc", camera.cameraPos);
        shdr->setMatrix4f("view", view);
        shdr->setMatrix4f("projection", projection);
        
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(cube.VAO);
        glDrawArrays(GL_TRIANGLES, 0, cube.vertex_count);

        glm::mat4 skyMapView = glm::mat4(glm::mat3(view));  

        glDepthFunc(GL_LEQUAL);
        skyShdr->use();
        skyShdr->setMatrix4f("view", skyMapView);
        skyShdr->setMatrix4f("projection", projection);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyTexture);
        glBindVertexArray(skybox.VAO);
        glDrawArrays(GL_TRIANGLES, 0, skybox.vertex_count);
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(w);
        glfwPollEvents();
    } // 
    glfwTerminate();
    return 0;
}

void reshape_viewport(GLFWwindow *w, int width, int height){
    glViewport(0, 0, width, height);
}
void cursor_callback(GLFWwindow* window, double x, double y){
    float delta_x = x - camera.x_glob;
    float delta_y = camera.y_glob - y;
    camera.x_glob = x;
    camera.y_glob = y;
    camera.modify_pitch_yaw(delta_x, delta_y);
}
void scroll_callback(GLFWwindow* w, double x, double y){
    camera.modify_fov(y);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    else 
        camera.update_camera_position(key, action);
    
    if (key != GLFW_KEY_ESCAPE && DEBUG){
        std::cout << "Current Position is " << camera.cameraPos.x << ", " << camera.cameraPos.y << ", " << camera.cameraPos.z << std::endl;
    }
    
}

GLuint load_texture(char const * path)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

GLuint load_cube_texture(std::vector< std::string > paths, std::string base=""){
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    int w, h, nrChannels;
    for (int i=0; i<paths.size(); ++i){
        unsigned char* data = stbi_load((base+paths[i]).c_str(), &w, &h, &nrChannels, 0);
        if (data){
            GLenum format;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else 
                format = GL_RGBA;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else {
            std::cerr << "FAILED TO LOAD CUBE TEXTURE AT LOCATION " << paths[i] << std::endl;
        }
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return texture;
}