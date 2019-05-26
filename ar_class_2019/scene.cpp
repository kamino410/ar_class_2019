#include "scene.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <map>

#define WIDTH 960
#define HEIGHT WIDTH*9/16
#define HEIGHT_CROP_RATIO 2.0/16

void setupBackground(GLSLProgramWrapper*& pro, Object& obj, int width, int height) {
	float verts[3 * 4];
	verts[0] = -1.0f, verts[1] = -1.0f, verts[2] = 0.0f;
	verts[3] = 1.0f, verts[4] = -1.0f, verts[5] = 0.0f;
	verts[6] = 1.0f, verts[7] = 1.0f, verts[8] = 0.0f;
	verts[9] = -1.0f, verts[10] = 1.0f, verts[11] = 0.0f;

	float texs[2 * 4];
	texs[0] = 1.0f, texs[1] = 1.0f - HEIGHT_CROP_RATIO;
	texs[2] = 0.0f, texs[3] = 1.0f - HEIGHT_CROP_RATIO;
	texs[4] = 0.0f, texs[5] = HEIGHT_CROP_RATIO;
	texs[6] = 1.0f, texs[7] = HEIGHT_CROP_RATIO;

	unsigned int indexes[3 * 2];
	indexes[0] = 0, indexes[1] = 1, indexes[2] = 2;
	indexes[3] = 0, indexes[4] = 3, indexes[5] = 2;

	{
		glGenBuffers(2, obj.vaaos);
		glBindBuffer(GL_ARRAY_BUFFER, obj.vaaos[0]);
		glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(GLfloat), verts, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, obj.vaaos[1]);
		glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(GLfloat), texs, GL_STATIC_DRAW);

		glGenBuffers(1, &obj.eao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.eao);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * 2 * sizeof(GLuint), indexes, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		obj.elemCount = 6;
	}

	{
		glGenVertexArrays(1, &obj.vao);
		glBindVertexArray(obj.vao);

		GLint posLoc = pro->getAttribLocation("VertexPosition");
		glEnableVertexAttribArray(posLoc);
		glBindBuffer(GL_ARRAY_BUFFER, obj.vaaos[0]);
		glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

		GLint texLoc = pro->getAttribLocation("VertexTex");
		glEnableVertexAttribArray(texLoc);
		glBindBuffer(GL_ARRAY_BUFFER, obj.vaaos[1]);
		glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

		glGenTextures(1, &obj.tbo);
		glBindTexture(GL_TEXTURE_RECTANGLE, obj.tbo);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glBindTexture(GL_TEXTURE_RECTANGLE, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
}

void setupObject(GLSLProgramWrapper*& pro, Object& obj, std::string filename) {
	std::string warn, err;
	bool res = tinyobj::LoadObj(&obj.attr, &obj.shapes, &obj.materials, &warn, &err, filename.c_str());
	if (!warn.empty()) {
		std::cerr << warn << std::endl;
	}
	if (!res) {
		std::cerr << err << std::endl;
		exit(1);
	}

	std::cout << "verts.size() norms.size() : " << obj.attr.vertices.size() << " " << obj.attr.normals.size() << " " << std::endl;
	std::cout << "shape.size() : " << obj.shapes.size() << std::endl;

	{
		std::vector<glm::vec3> verts;
		std::vector<glm::vec2> texs;
		for (const auto& shape : obj.shapes) {
			std::cout << shape.name << std::endl;

			size_t index_offset = 0;
			for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
				int num_vertices = shape.mesh.num_face_vertices[f];
				if (num_vertices == 3) {
					for (size_t v = 0; v < 3; v++) {
						tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

						float vx = 0.01 * obj.attr.vertices[3 * idx.vertex_index + 0];
						float vy = 0.01 * obj.attr.vertices[3 * idx.vertex_index + 1];
						float vz = 0.01 * obj.attr.vertices[3 * idx.vertex_index + 2];

						//float nx = obj.attr.normals[3 * idx.normal_index + 0];
						//float ny = obj.attr.normals[3 * idx.normal_index + 1];
						//float nz = obj.attr.normals[3 * idx.normal_index + 2];

						tinyobj::real_t tx =
							obj.attr.texcoords[2 * idx.texcoord_index + 0];
						tinyobj::real_t ty =
							obj.attr.texcoords[2 * idx.texcoord_index + 1];

						verts.push_back(glm::vec3(vx, vy, vz));
						texs.push_back(glm::vec2(tx, ty));
					}
				}
				index_offset += num_vertices;
			}
		}

		glGenBuffers(2, obj.vaaos);
		glBindBuffer(GL_ARRAY_BUFFER, obj.vaaos[0]);
		glBufferData(GL_ARRAY_BUFFER, 3 * verts.size() * sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, obj.vaaos[1]);
		glBufferData(GL_ARRAY_BUFFER, 2 * texs.size() * sizeof(GLfloat), texs.data(), GL_STATIC_DRAW);

		obj.elemCount = verts.size();
	}
	{
		glGenVertexArrays(1, &obj.vao);
		glBindVertexArray(obj.vao);

		GLint posLoc = pro->getAttribLocation("VertexPosition");
		glEnableVertexAttribArray(posLoc);
		glBindBuffer(GL_ARRAY_BUFFER, obj.vaaos[0]);
		glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

		GLint texLoc = pro->getAttribLocation("VertexTex");
		glEnableVertexAttribArray(texLoc);
		glBindBuffer(GL_ARRAY_BUFFER, obj.vaaos[1]);
		glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	{
		cv::Mat img = cv::imread("utc_all2.png");
		cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
		cv::flip(img, img, 0);
		glActiveTexture(GL_TEXTURE1);
		glGenTextures(1, &obj.tbo);
		glBindTexture(GL_TEXTURE_2D, obj.tbo);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

Scene::Scene() {}

Scene::~Scene() { delete backPro; }

int Scene::getheight() { return HEIGHT; }

int Scene::getwidth() { return WIDTH; }

void Scene::keycallback(GLFWwindow* window, int key, int scancode, int action,
	int mods) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}
}

void Scene::setupProgram() {
	backPro = new GLSLProgramWrapper();
	backPro->compileShader("background.vert", GLSLShaderType::VERTEX);
	backPro->compileShader("background.frag", GLSLShaderType::FRAGMENT);
	backPro->link();

	objPro = new GLSLProgramWrapper();
	objPro->compileShader("object.vert", GLSLShaderType::VERTEX);
	objPro->compileShader("object.frag", GLSLShaderType::FRAGMENT);
	objPro->link();
}

void Scene::preProcess() {
	cap.open(0);
	if (!cap.isOpened() || !cap.read(frame)) {
		std::cout << "Failed to open camera capture" << std::endl;
		exit(1);
	}
	std::cout << "Camera Resolution : " << frame.cols << " x " << frame.rows << std::endl;

	setupBackground(backPro, backObj, frame.cols, frame.rows);
	setupObject(objPro, unityChan, "sd_unitychan.obj");
	//setupObject(objPro, unityChan, "cube.obj");
}

void Scene::draw(GLFWwindow* window) {
	glViewport(0, 0, WIDTH, HEIGHT);
	glClearColor(0.5, 0.5, 0.5, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawBackground();
	drawUnityChan();

	glfwSwapBuffers(window);
	glfwPollEvents();
}

void Scene::drawBackground() {
	if (cap.read(frame)) {
		cv::imshow("Test", frame);
		cv::waitKey(1);
	}
	glDisable(GL_DEPTH_TEST);

	glBindTexture(GL_TEXTURE_RECTANGLE, backObj.tbo);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, frame.cols, frame.rows, GL_BGR, GL_UNSIGNED_BYTE, frame.data);

	backPro->enable();

	backPro->setUniform("TexHeight", (float)frame.rows);
	backPro->setUniform("TexWidth", (float)frame.cols);

	backPro->setUniform("TexImg", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, backObj.tbo);

	glBindVertexArray(backObj.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backObj.eao);
	glDrawElements(GL_TRIANGLES, backObj.elemCount, GL_UNSIGNED_INT, 0);
}

void Scene::drawUnityChan() {
	glEnable(GL_DEPTH_TEST);

	objPro->enable();
	glm::mat4 modelMat = glm::translate(glm::identity<glm::mat4>(), glm::vec3(0, -0.5, -1.5));
	glm::mat4 viewMat = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
	glm::mat4 projMat = glm::perspective(glm::radians(60.0f), 16.0f / 9, 0.1f, 10.0f);
	objPro->setUniform("mvp", projMat * viewMat * modelMat);

	objPro->setUniform("TexImg", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, unityChan.tbo);

	//glBindVertexArray(backObj.vao);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backObj.eao);
	//glDrawElements(GL_TRIANGLES, backObj.elemCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(unityChan.vao);
	glDrawArrays(GL_TRIANGLES, 0, unityChan.elemCount);
}
