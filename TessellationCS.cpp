//Vertex Shader
#version 430 core

layout(location = 1) attribute vec2 position;

void main(){
    gl_Position = vec4(position, 0, 1);
}







//Tessellation control Shader
#version 430

layout(vertices = 3) out;

patch out vec3 patchColor;

void main()
{
    if(gl_PrimitiveID == 0){
        patchColor = vec3(1,0,0);
    }else {
        patchColor = vec3(1,1,0);
    }


    //CPU端绘制patch时，指定每个patch由2个顶点构成
    //前2次调用使用原样输出，第3次调用（即，gl_InvocationID为2的那次）指定新增顶点的坐标
    if(gl_InvocationID <= 1){
        gl_out[gl_InvocationID].gl_Position =  gl_in[gl_InvocationID].gl_Position;
    }else {
        gl_out[gl_InvocationID].gl_Position =  vec4(0,0,0,1); //如果TCS执行ID为2，则输出新增cp
    }


    gl_TessLevelInner[0] = 1;
    gl_TessLevelOuter[0] = 1;
    gl_TessLevelOuter[1] = 1;
    gl_TessLevelOuter[2] = 1;
}







//Tessellation Evaluation Shader
#version 430

layout (triangles, equal_spacing, ccw) in;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


patch in vec3 patchColor; //使用patch关键字声明每patch输出变量
out vec4 tessColor; //输出颜色给FS


void main()
{
    vec3 p0 = gl_TessCoord.x * gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_TessCoord.y * gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_TessCoord.z * gl_in[2].gl_Position.xyz;
    vec3 pos = (p0 + p1 + p2);

    tessColor = vec4(patchColor, 1); //输出颜色给FS

    gl_Position = projection * view * model *  vec4(pos,1);

}







//Fragment Shader
#version 430 core

in vec4 tessColor; //接受来自TES输入的颜色
out vec4 FragColor;
void main(){
    FragColor = tessColor;
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


    void createShader() {

        //1. CREATE SHADER PROGRAM
        sID = glCreateProgram();
        GLuint vID = glCreateShader(GL_VERTEX_SHADER);
        GLuint tcsID = glCreateShader(GL_TESS_CONTROL_SHADER); //增加TCS
        GLuint tesID = glCreateShader(GL_TESS_EVALUATION_SHADER); //增加TES
        GLuint fID = glCreateShader(GL_FRAGMENT_SHADER);


        //2. LOAD SHADER SOURCE CODE
        readShader(vID, "../GLSL/tessellationVS.glsl");
        readShader(tcsID, "../GLSL/tessellationTCS.glsl");//增加TCS
        readShader(tesID, "../GLSL/tessellationTES.glsl");//增加TES
        readShader(fID, "../GLSL/tessellationFS.glsl");


        //3. COMPILE
        compileShader(vID, "vertex");
        compileShader(tcsID, "tessellationCS");
        compileShader(tesID, "tessellationES");
        compileShader(fID, "fragment");


        //5. ATTACH SHADERS TO PROGRAM
        glAttachShader(sID, vID);
        glAttachShader(sID, tcsID);
        glAttachShader(sID, tesID);
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
        glDeleteShader(tcsID);
        glDeleteShader(tesID);
        glDeleteShader(fID);
    }


    void assignData() {

        glUseProgram(sID);
        glGenVertexArrays(1, &arrayID);
        glBindVertexArray(arrayID);

        float triangle[5][2] = {
                {-0.5, -0.5},
                {0.5,  -0.5},
                {0.5,  0.5},
                {-0.5,  0.5}

        };

        GLubyte index[4] = {0, 1, 2, 3};

        GLuint elementID;
        glGenBuffers(1, &elementID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);


        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);


        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float[2]), 0);


        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);

    }


    void draw(int width, int height, float f) {

        //project变换
        glm::mat4 projection = glm::perspective(45.0f, (float) width / height, 0.1f, 100.f);
        GLuint projectionID = glGetUniformLocation(sID, "projection");
        glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projection));

        //view变换
        glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 3.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        GLuint viewID = glGetUniformLocation(sID, "view");
        glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));

        //model变换
        glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1, 1, 0) * f);
        glm::mat4 rotate = glm::rotate(glm::mat4(1), glm::radians(360 * f), glm::vec3(0, 1, 0));
        glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(2 * f, 0, 0));

        GLuint modelID = glGetUniformLocation(sID, "model");
        glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(rotate));



        glPatchParameteri(GL_PATCH_VERTICES, 2); //指定一个patch由多个顶点构成
        glDrawElements(GL_PATCHES, 4, GL_UNSIGNED_BYTE, 0); //绘制patch


    }
};


int main() {

    if (!glfwInit()) exit(EXIT_FAILURE);

    int width = 640, height = 480;
    GLFWwindow *window = glfwCreateWindow(width, height, "Tessellation", NULL, NULL);
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

    glUseProgram(shader.sID);
    glBindVertexArray(shader.arrayID);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glViewport(0, 0, width, height);


    cout << glGetError() << endl;


    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;
        shader.draw(width, height, f);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //Unbind Vertex Array Object and Shader
    glBindVertexArray(0);
    glUseProgram(0);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
