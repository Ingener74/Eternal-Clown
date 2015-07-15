
#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <cstdint>
#include <climits>

#include <GL/glew.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

extern "C"
{
	#include <libavformat/avformat.h>
}

//#include "VideoSprite.h"

#include "highgui.hpp"

using namespace std;
using namespace glm;

void mouse(int button, int action, int x, int y) {
}
void mouseMove(int x, int y) {
}
void key(unsigned char key, int x, int y) {
}
void specKey(int key, int x, int y) {
}


void reshape(int w, int h);
void init();
void display(void);
void timer(int time);
void deinit();

string vertex_texture = R"(
#version 330
uniform mat4 projection, view, model;
layout (location = 0) in vec4 position;
layout (location = 1) in vec2 texcoord;
out vec2 texcoord_;
void main(){
    gl_Position = projection * view * model * position;
    texcoord_ = texcoord;
}
)", fragment_texture = R"(
#version 330
uniform sampler2D texture;
in vec2 texcoord_;
out vec4 fragColor;
void main(){
    fragColor = texture2D(texture, texcoord_);
}
)";

GLuint createShader(GLenum shaderType, const string& source) {
    GLuint shader = glCreateShader(shaderType);
    if (!shader)
        throw runtime_error("can't create shader");

    const char* sourceBuffer = source.c_str();

    glShaderSource(shader, 1, &sourceBuffer, 0);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled)
        return shader;

    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    if (!infoLen)
        throw runtime_error("error in create shader");

    vector<char> buf(infoLen);
    glGetShaderInfoLog(shader, infoLen, 0, buf.data());

    static array<string, 2> shaderTypeString = { "GL_FRAGMENT_SHADER", "GL_VERTEX_SHADER" };

    throw runtime_error(""
            "can't create shader " + shaderTypeString[shaderType - GL_FRAGMENT_SHADER] + "\n" + &buf.front());
}

GLuint createProgram(const string& vertexShaderSource, const string& fragmentShaderSource) {
    if (vertexShaderSource.empty())
        throw runtime_error("vertex shader is empty");
    if (fragmentShaderSource.empty())
        throw runtime_error("fragment shader is empty");

    GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint program = glCreateProgram();
    if (!program)
        throw runtime_error("can't create program");

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus != GL_TRUE) {
        GLint bufLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLen);
        if (bufLen) {
            vector<char> buf(bufLen);
            glGetProgramInfoLog(program, bufLen, NULL, buf.data());
            throw runtime_error(string("can't link shader program\n") + buf.data());
        }
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

float w = 1280.f, h = 536.f, m = .3f;
float width = w * m, height = h * m;

GLfloat
//bw       = 136.f,
//bh       = 40.f,
bw       = width,
bh       = height,

b0_off   = 0.f,
b1_off   = 41.f,
b2_off   = 82.f,

ts       = 256.f;

vector<GLfloat> pos012 = {
    -bw/2,  bh/2, 0.f,
     bw/2,  bh/2, 0.f,
    -bw/2, -bh/2, 0.f,
     bw/2, -bh/2, 0.f,
};

vector<GLfloat> texcoord0 = {
//    0.f, b0_off/ts,           bw/ts, b0_off/ts,
//    0.f, b0_off/ts + bh/ts,   bw/ts, b0_off/ts + bh/ts,
    0.f, 0.f,                  w / 2048.f, 0.f,
    0.f, h / 2048.f,           w / 2048.f, h / 2048.f,
}, texcoord1 = {
    0.f, b1_off/ts,           bw/ts, b1_off/ts,
    0.f, b1_off/ts + bh/ts,   bw/ts, b1_off/ts + bh/ts,
}, texcoord2 = {
    0.f, b2_off/ts,           bw/ts, b2_off/ts,
    0.f, b2_off/ts + bh/ts,   bw/ts, b2_off/ts + bh/ts,
};

vector<uint16_t> indeces = {
    0, 2, 1, 3
};

GLuint shader;
GLint uProj, uView, uModel, uTexture;
GLint aTexCoord;
GLint aPosition;

GLuint vaos[3];
GLuint pos;
GLuint texcoords[3];
GLuint indeces_;
GLuint indecesCount[] = {4, 4, 4};
GLenum drawModes[] = {GL_TRIANGLE_STRIP, GL_TRIANGLE_STRIP, GL_TRIANGLE_STRIP};

mat4 proj, view, model;

VideoCapture* videoSprite = nullptr;

void init();
void display(void);
void deinit() {}

void reshape(int w, int h);
void timer(int time);

int main(int argc, char **argv) {
    try {
//        if(argc < 2)
//            throw invalid_argument("usage: ./Test <path-to-resource>");

        av_register_all();

//        base = argv[1];

        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
        glutInitWindowSize(width, height);

        // Для OpenGL 3.3
        glutInitContextVersion(3, 3);
        glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
        glutInitContextProfile(GLUT_CORE_PROFILE);

        glutCreateWindow("Test");

        glewExperimental = GL_TRUE;
        auto glewInitError = glewInit();
        if (glewInitError != GLEW_OK)
            throw runtime_error("glew init error " + string(reinterpret_cast<const char*>(glewGetErrorString(glewInitError))));

        {
            cout << "OpenGL vendor:                     " << glGetString(GL_VENDOR) << endl;
            cout << "OpenGL renderer:                   " << glGetString(GL_RENDERER) << endl;
            cout << "OpenGL version:                    " << glGetString(GL_VERSION) << endl;
            cout << "OpenGL shading language version:   " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
//            cout << "OpenGL extensions:                 " << glGetString(GL_EXTENSIONS) << endl;
        }

        glutReshapeFunc(reshape);
        glutDisplayFunc(display);
        glutMouseFunc(mouse);
        glutMotionFunc(mouseMove);
        glutKeyboardFunc(key);
        glutSpecialFunc(specKey);

//        glutFullScreen();

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        reshape(width, height);
        init();

        glutTimerFunc(0, timer, 0);
        glutMainLoop();

        deinit();

        return EXIT_SUCCESS;
    } catch (exception const & e) {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }
}

void init() {

    shader = createProgram(vertex_texture, fragment_texture);

    aPosition = glGetAttribLocation(shader, "position");
    aTexCoord = glGetAttribLocation(shader, "texcoord");

    uProj     = glGetUniformLocation(shader, "projection");
    uView     = glGetUniformLocation(shader, "view");
    uModel    = glGetUniformLocation(shader, "model");
    uTexture  = glGetUniformLocation(shader, "texture");

    cout << "aPosition " << aPosition << ", aTexCoord " << aTexCoord << ", " << endl <<
        "uProj " << uProj << ", uView " << uView << ", uModel " << uModel << ", uTexture " << uTexture << endl;

    videoSprite = new VideoCapture("/home/pavel/trailer.h.mp4");

    glGenBuffers(1, &pos);
    glBindBuffer(GL_ARRAY_BUFFER, pos);
    glBufferData(GL_ARRAY_BUFFER, pos012.size() * sizeof(float), pos012.data(), GL_STATIC_DRAW);

    glGenBuffers(3, texcoords);
    {
        glBindBuffer(GL_ARRAY_BUFFER, texcoords[0]);
        glBufferData(GL_ARRAY_BUFFER, texcoord0.size() * sizeof(float), texcoord0.data(), GL_STATIC_DRAW);

//        glBindBuffer(GL_ARRAY_BUFFER, texcoords[1]);
//        glBufferData(GL_ARRAY_BUFFER, texcoord1.size() * sizeof(float), texcoord1.data(), GL_STATIC_DRAW);
//
//        glBindBuffer(GL_ARRAY_BUFFER, texcoords[2]);
//        glBufferData(GL_ARRAY_BUFFER, texcoord2.size() * sizeof(float), texcoord2.data(), GL_STATIC_DRAW);
    }

    glGenBuffers(1, &indeces_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indeces_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indeces.size() * sizeof(uint16_t), indeces.data(), GL_STATIC_DRAW);

    glGenVertexArrays(3, vaos);
    {
        glBindVertexArray(vaos[0]);

        glBindBuffer(GL_ARRAY_BUFFER, pos);
        glEnableVertexAttribArray(aPosition);
        glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, texcoords[0]);
        glEnableVertexAttribArray(aTexCoord);
        glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indeces_);
    }
//    {
//        glBindVertexArray(vaos[1]);
//
//        glBindBuffer(GL_ARRAY_BUFFER, pos);
//        glEnableVertexAttribArray(aPosition);
//        glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
//
//        glBindBuffer(GL_ARRAY_BUFFER, texcoords[1]);
//        glEnableVertexAttribArray(aTexCoord);
//        glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
//
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indeces_);
//    }
//    {
//        glBindVertexArray(vaos[2]);
//
//        glBindBuffer(GL_ARRAY_BUFFER, pos);
//        glEnableVertexAttribArray(aPosition);
//        glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
//
//        glBindBuffer(GL_ARRAY_BUFFER, texcoords[2]);
//        glEnableVertexAttribArray(aTexCoord);
//        glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
//
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indeces_);
//    }
}

void display(void) {
//    if (!videoSprite->isPlay()) {
//        videoSprite->start();
//    }

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.2f, 0.2f, 0.1f, 1.f);

    glUseProgram(shader);

    glUniformMatrix4fv(uProj, 1, GL_FALSE, &proj[0][0]);
    glUniformMatrix4fv(uView, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(uModel, 1, GL_FALSE, &model[0][0]);

    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, tex.textureId);
//    videoSprite->draw();
    glUniform1i(uTexture, 0);

    glBindVertexArray(vaos[0]);
    glDrawElements(drawModes[0], indecesCount[0], GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);

    glFlush();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);

//	proj = glm::perspective(45.f, w / float(h), 10.f, 10000.f);
    proj = glm::ortho<float>(-width / 2, width / 2, -height / 2, height / 2, -height / 2, height / 2);
//	view = glm::lookAt<float>(vec3(40.f, 40.f, 100.f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
}

void timer(int time) {
    glutPostRedisplay();

    int delay = 1000 / 60;
    glutTimerFunc(delay, timer, 0);
}

