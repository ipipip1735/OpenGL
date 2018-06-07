//Vertex Shader
layout(location = 1) attribute vec2 position;
layout(location = 2) attribute vec2 coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texCoord;
void main(){
    gl_Position = projection * view * model * vec4(position, 0, 1);
    texCoord = coord;
}





//Fragment Shader
layout(binding=0) uniform sampler2D sampler;

in vec2 texCoord;
out vec4 FragColor;

void main(){
    FragColor = texture(sampler, texCoord);
    if(FragColor.a < 0.5)discard;
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
#include "inc/GLSW/glsw.h"
#include "inc/GLSW/bstrlib.h"


using namespace std;

struct Bitmap {

    int width;
    int height;
    short BitsPerPixel;
    vector<unsigned char> pixels;

    Bitmap(const char *FilePath) : width(0), height(0), BitsPerPixel(0) {
        load(FilePath);
    }

    //adapted from http://stackoverflow.com/questions/20595340/loading-a-tga-bmp-file-in-c-opengl
    void load(const char *FilePath) {
        fstream hFile;

        //search for file by going up file directory tree up to 5 times
        int attempts = 0;
        string nfilepath = FilePath;
        while (!hFile.is_open() && attempts < 5) {
            hFile.open(nfilepath.c_str(), ios::in | ios::binary);
            nfilepath = "../" + nfilepath;
            attempts += 1;
        }
        if (!hFile.is_open()) throw invalid_argument("Error: File Not Found.");

        hFile.seekg(0, ios::end);
        int Length = hFile.tellg();
        hFile.seekg(0, ios::beg);


        vector<uint8_t> FileInfo(Length);
        hFile.read(reinterpret_cast<char *>(FileInfo.data()), 54);

        if (FileInfo[0] != 'B' && FileInfo[1] != 'M') {
            hFile.close();
            throw invalid_argument("Error: Invalid File Format. Bitmap Required.");
        }

        if (FileInfo[28] != 24 && FileInfo[28] != 32) {
            hFile.close();
            throw invalid_argument("Error: Invalid File Format. 24 or 32 bit Image Required.");
        }

        BitsPerPixel = FileInfo[28];
        width = FileInfo[18] + (FileInfo[19] << 8);
        height = FileInfo[22] + (FileInfo[23] << 8);
        uint32_t pixelsOffset = FileInfo[10] + (FileInfo[11] << 8);
        uint32_t size = ((width * BitsPerPixel + 31) / 32) * 4 * height;
        pixels.resize(size);

        hFile.seekg(pixelsOffset, ios::beg);
        hFile.read(reinterpret_cast<char *>(pixels.data()), size);
        hFile.close();
    }

};

class Shader {
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
    GLuint tID1, tID2;


    void createShader() {

        //1. CREATE SHADER PROGRAM
        sID = glCreateProgram();
        GLuint vID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fID = glCreateShader(GL_FRAGMENT_SHADER);


        //2. LOAD SHADER SOURCE CODE
        glswInit();
        glswSetPath("../GLSL/", ".glsl");
        glswAddDirectiveToken("", "#version 430 core");
        const char *vs = glswGetShader("texture.Vertex");
        const char *fs = glswGetShader("texture.Fragment");
        glswShutdown();
        glShaderSource(vID, 1, &vs, NULL);
        glShaderSource(fID, 1, &fs, NULL);


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

        float triangle[3][4] = {
                {-0.8, -0.5, 0,   0},
                {0.8,  -0.5, 0,   1},
                {0,    0.8,  0.5, 1}
        };

        glGenVertexArrays(1, &arrayID);
        glBindVertexArray(arrayID);

        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float[3][4]), triangle, GL_STATIC_DRAW);


        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float[4]), 0);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float[4]), (void *) sizeof(float[2]));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

    }

    void assignTexture() {

        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &tID1);
        glBindTexture(GL_TEXTURE_2D, tID1);

        //设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        float borderColor[] = {1.0f, 0.0f, 0.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);


        //读取图片数据
        Bitmap bitmap("../resources/flower4.bmp");

        //使用可变存储纹理
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, bitmap.width, bitmap.height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                     bitmap.pixels.data());
        glGenerateMipmap(GL_TEXTURE_2D);  //自动生成多级纹理


    }


    void draw(int width, int height, float f) {


        GLuint samplerID = glGetUniformLocation(sID, "sampler");
        glUniform1i(samplerID, 0);


        //project变换
        glm::mat4 projection = glm::perspective(45.0f, (float) width / height, 0.1f, 100.f);
        GLuint projectionID = glGetUniformLocation(sID, "projection");
        glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projection));

        //view变换
        glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 3.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        GLuint viewID = glGetUniformLocation(sID, "view");
        glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));

        //model变换
        glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1, 1, 1) * f);
        glm::mat4 rotate = glm::rotate(glm::mat4(1), glm::radians(360 * f), glm::vec3(0, 1, 0));
        glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(2 * f, 0, 0));

        GLuint modelID = glGetUniformLocation(sID, "model");
        glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(rotate));

        //Draw Triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);

    }

    void assignCubeData() {

        float triangle[4][4] = {
                {-0.5f, -0.5f, 0, 0},
                {0.5f,  -0.5f, 1, 0},
                {0.5f,  0.5f,  1, 1},
                {-0.5f, 0.5f,  0, 1}
        };

        GLuint index[6] = { 0, 1, 3, 1, 2, 3};
//     3---2
//     |\  |
//     |  \|
//     0---1


        glGenVertexArrays(1, &arrayID);
        glBindVertexArray(arrayID);

        GLuint elementID;
        glGenBuffers(1, &elementID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float[4][4]), triangle, GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float[4]), 0);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float[4]), (void *) sizeof(float[2]));

        glBindVertexArray(0);
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
//        glBindBuffer(GL_ARRAY_BUFFER, 0);




    }



    void assignCubeTexture() {


        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &tID1);
        glBindTexture(GL_TEXTURE_2D, tID1);

        //设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 2);


        //读取图片数据
        vector<unsigned char> image; //the raw pixels
        unsigned width, height;
        unsigned error = lodepng::decode(image, width, height, "../resources/y.png");
        if (error) cout << "decoder error " << error << ": " << lodepng_error_text(error) << endl;


        //使用不变存储纹理
        glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, width, height);

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
        glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, width / 2, height / 2, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
        glTexSubImage2D(GL_TEXTURE_2D, 2, 0, 0, width / 4, height / 4, GL_RGBA, GL_UNSIGNED_BYTE, image.data());

    }


    void drawCube(int width, int height, float f) {


//        GLuint samplerID = glGetUniformLocation(sID, "sampler");
//        glUniform1i(samplerID, 0);


        //project变换
        glm::mat4 projection = glm::perspective(45.0f, (float) width / height, 0.1f, 100.f);
        GLuint projectionID = glGetUniformLocation(sID, "projection");
        glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projection));

        //view变换
        glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 3.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        GLuint viewID = glGetUniformLocation(sID, "view");
        glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));

        //model变换
        glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1, 1, 1) * f);
        glm::mat4 rotate = glm::rotate(glm::mat4(1), glm::radians(360 * f), glm::vec3(0, 1, 0));
        glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(2 * f, 0, 0));

        GLuint modelID = glGetUniformLocation(sID, "model");
        glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(rotate));


        //Draw QUADS
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

};


int main() {

    if (!glfwInit()) exit(EXIT_FAILURE);

    int width = 640, height = 480;
    GLFWwindow *window = glfwCreateWindow(width, height, "texture", NULL, NULL);
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


//    shader.assignData(); //三角形顶点数据
//    shader.assignTexture(); //可变存储

    //绘制立方体
    shader.assignCubeData(); //立方体顶点数据
    shader.assignCubeTexture(); //不变存储


    glViewport(0, 0, width, height);

    glUseProgram(shader.sID);
    glBindVertexArray(shader.arrayID);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;

//        shader.draw(width, height, f);
        shader.drawCube(width, height, f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glUseProgram(0);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}




