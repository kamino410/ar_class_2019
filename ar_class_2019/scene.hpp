#pragma once

#include "gl_program_wrapper.hpp"
#include "tinyobjloader.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/transform.hpp>

#include <opencv2/opencv.hpp>

struct Part {
	GLuint vao;       // Vertex Array Object
	GLuint vaaos[2];  // Vertex Attribute Array Object
	GLuint eao;       // Element Array Object
	int elemCount, tex_id;
};

struct Object {
	tinyobj::attrib_t attr;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::vector<GLuint> tbos;
	std::vector<Part> parts;
};

class Scene {
	GLSLProgramWrapper* backPro, * objPro;
	Object cameraBack, imgBack, unityChan, gunbot, spaceship;
	cv::VideoCapture cap;
	cv::Mat frame, img1, img2, img3, img4;

	void drawBackground(Object obj, const cv::Mat& frame);
	void drawObj(Object obj, glm::mat4 modelMat);

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
