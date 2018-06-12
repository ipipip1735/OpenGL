//Vertex Shader
#version 430 core

layout(location = 0) attribute vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 texCoord;
void main(){
    texCoord = position;
    gl_Position = projection * view * model * vec4(position, 1);
}





//Fragment Shader
#version 430 core

layout(binding=0) uniform samplerCube sampler;
in vec3 texCoord;

void main(){
    vec4 texColor = texture(sampler, texCoord);
    gl_FragColor = texColor;

}




//CPP source code
#include "inc/GLEW/glew.h"
#include "inc/GLFW/glfw3.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>

#define GLM_FORCE_RADIANS

#include "lib/GLM/gtc/type_ptr.hpp"
#include "lib/GLM/gtc/matrix_transform.hpp"
#include "inc/lodepng.h"

using namespace std;


class Shader {

    void readShader(GLuint id, string file) {
        ifstream ifs(file);
        stringstream ss;
        ifs >> ss.rdbuf();
        file = ss.str();
        const char *shader = file.data();
        glShaderSource(id, 1, &shader, NULL);
    }

    void compileShader(GLuint id, string type) {
        glCompileShader(id);

        GLint comp;
        glGetShaderiv(id, GL_COMPILE_STATUS, &comp);
        if (comp == GL_FALSE) {
            cout << type << " Shader Compilation FAILED" << endl;
            GLchar messages[256];
            glGetShaderInfoLog(id, sizeof(messages), 0, &messages[0]);
            cout << messages;
        }
    }


public:
    GLuint sID;
    GLuint arrayID;
    GLuint fboID;
    GLuint tID1, tID2;


    void createShader() {

        //1. CREATE SHADER PROGRAM
        sID = glCreateProgram();
        GLuint vID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fID = glCreateShader(GL_FRAGMENT_SHADER);


        //2. LOAD SHADER SOURCE CODE
        readShader(vID, "../GLSL/cubeTextureVS.glsl");
        readShader(fID, "../GLSL/cubeTextureFS.glsl");


        //3. COMPILE
        compileShader(vID, "vertex");
        compileShader(fID, "fragment");


        //5. ATTACH SHADERS TO PROGRAM
        glAttachShader(sID, vID);
        glAttachShader(sID, fID);


        //6. LINK PROGRAM
        glLinkProgram(sID);



        //7. CHECK FOR LINKING ERRORS
        GLint linkStatus;
        glGetProgramiv(sID, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE) {
            cout << "Shader Linking FAILED" << endl;
            GLchar messages[256];
            glGetProgramInfoLog(sID, sizeof(messages), 0, &messages[0]);
            cout << messages;
        }

        //8. CHECK FOR LINKING STATUS
        glValidateProgram(sID);
        GLint validateStatus;
        glGetProgramiv(sID, GL_VALIDATE_STATUS, &validateStatus);
        cout << "Link: " << linkStatus << "  Validate: " << validateStatus << endl;
        if (linkStatus == GL_FALSE) {
            cout << "Shader Validation FAILED" << endl;
            GLchar messages[256];
            glGetProgramInfoLog(sID, sizeof(messages), 0, &messages[0]);
            cout << messages;
        }


        //9. DELETE SHADER
        glDeleteShader(vID);
        glDeleteShader(fID);
    }


    void assignData() {

        float cube[8][3] = {
                {1,  -1, 1},
                {1,  1,  1},
                {-1, 1,  1},
                {-1, -1, 1},
                {1,  -1, -1},
                {1,  1,  -1},
                {-1, 1,  -1},
                {-1, -1, -1},
        };


//        GLubyte index[24] = {0, 1, 2, 0, 2, 3}; //bottom

        GLubyte index[24] = {
                0, 1, 2, 3, //front
                7, 6, 5, 4, //back
                3, 2, 6, 7, //left
                4, 5, 1, 0, //right
                1, 5, 6, 2, //top
                4, 0, 3, 7}; //bottom


//            //6-------------/5
//          //  .           // |
//        //2--------------1   |
//        //    .          |   |
//        //    .          |   |
//        //    .          |   |
//        //    .          |   |
//        //    7.......   |   /4
//        //               | //
//        //3--------------/0


        glUseProgram(sID);

        glGenVertexArrays(1, &arrayID);
        glBindVertexArray(arrayID);

        GLuint elementID;
        glGenBuffers(1, &elementID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float[3]), 0);

        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);

    }


    void assignTexture() {

        //准备3张图片，6个面每2个使用同一张图片
        unsigned width, height;
        vector<GLubyte> image1, image2, image3;
        lodepng::decode(image1, width, height, "../resources/a.png");
        lodepng::decode(image2, width, height, "../resources/b.png");
        lodepng::decode(image3, width, height, "../resources/c.png");


        //创建纹理对象
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &tID1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tID1);
        //配置纹理参数
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 2);


        if (width == height) {//判断是否图片的宽高是相等的


            //加载图片，有2种方法
            //方法一，手动加载
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image1.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image1.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image2.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image2.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image3.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image3.data());



            //方法二，使用for循环
//        for (int i = 0; i < 6; ++i) {
//            if (i < 2) {
//                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image1.data());
//            } else if(i>=2 && i <=3) {
//                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image2.data());
//            } else{
//                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image3.data());
//            }
//
//        }







//        //下面的代码是让不同的纹理使用不同的贴图（这里测试失败了）
//        //纹理级别0
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image1.data());
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image1.data());
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image1.data());
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image1.data());
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image1.data());
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image1.data());
//        //纹理级别1
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 1, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image2.data());
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 1, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image2.data());
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 1, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image2.data());
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 1, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image2.data());
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 1, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image2.data());
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 1, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image2.data());
//        //纹理级别2
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 2, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image3.data());
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 2, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image3.data());
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 2, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image3.data());
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 2, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image3.data());
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 2, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image3.data());
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 2, GL_RGBA, width, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, image3.data());
        }else {
            cout << "failure! image's width don't equal height" << endl;
        }
    }


    void draw(int width, int height) {

        //view变换
        glm::mat4 view = glm::lookAt(glm::vec3(3, 3, 5.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        GLuint viewID = glGetUniformLocation(sID, "view");
        glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));

        //投影变换
        glm::mat4 projection = glm::perspective(45.0f, (float) width / height, 0.1f, 100.f);
        GLuint projectionID = glGetUniformLocation(sID, "projection");
        glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projection));

        //model变换
        float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;
        glm::mat4 model(1);
        model = glm::scale(model, glm::vec3(2, 2, 2) * f);
        model = glm::rotate(model, glm::radians(360 * f), glm::vec3(0, 1, 0));
//        model = glm::translate(model, glm::vec3(2 * f, 0, 0));

        GLuint modelID = glGetUniformLocation(sID, "model");
        glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(model));



        //Draw Triangle
//        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
        glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, 0);

    }
};

int main() {

    if (!glfwInit()) exit(EXIT_FAILURE);

    int width = 640, height = 480;
    GLFWwindow *window = glfwCreateWindow(width, height, "triangle", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetWindowPos(window, 350, 170);
    glfwMakeContextCurrent(window);
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        cout << glewGetErrorString(err) << endl;
    }


    Shader shader;
    shader.createShader();
    shader.assignData();
    shader.assignTexture();



    //Bind Shader and Vertex Array Object
    glUseProgram(shader.sID);
    glBindVertexArray(shader.arrayID);


    glViewport(0, 0, width, height);
    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);


    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.draw(width, height);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //Unbind Vertex Array Object and Shader
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);


    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
