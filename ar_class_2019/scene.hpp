#pragma once

#include "gl_program_wrapper.hpp"
#include "tinyobjloader.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/transform.hpp>

#include <opencv2/opencv.hpp>

struct Object {
	tinyobj::attrib_t attr;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	GLuint vao;       // Vertex Array Object
	GLuint vaaos[3];  // Vertex Attribute Array Object
	GLuint eao;       // Element Array Object
	GLuint tbo;       // Texture Buffer Object
	int elemCount;
};

class Scene {
	GLSLProgramWrapper* backPro, * objPro;
	Object backObj, unityChan;
	cv::VideoCapture cap;
	cv::Mat frame;

	void drawBackground();
	void drawUnityChan();
	
public:
	Scene();
	~Scene();
	void setupProgram();
	void preProcess();
	void draw(GLFWwindow*);
	void keycallback(GLFWwindow* window, int key, int scancode, int action,
		int mods);
	int getwidth();
	int getheight();
};
