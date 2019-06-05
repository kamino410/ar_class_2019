#include "scene.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "PoseEstimation.h"
#include "MarkerTracker.h"


#define WIDTH 1920
#define HEIGHT WIDTH*9/16
#define HEIGHT_CROP_RATIO 2.0/16
#define kMarkerSize  0.3// = 0.048[m]
#define xscale 1.3
#define yscale 1.3

void setupBackground(GLSLProgramWrapper*& pro, Object& obj, int width, int height, bool cropped = false) {
	float verts[3 * 4];
	verts[0] = -1.0f, verts[1] = -1.0f, verts[2] = 0.99f;
	verts[3] = 1.0f, verts[4] = -1.0f, verts[5] = 0.99f;
	verts[6] = 1.0f, verts[7] = 1.0f, verts[8] = 0.99f;
	verts[9] = -1.0f, verts[10] = 1.0f, verts[11] = 0.99f;

	float texs[2 * 4];
	texs[0] = 1.0f, texs[1] = 1.0f - (cropped ? HEIGHT_CROP_RATIO : 0);
	texs[2] = 0.0f, texs[3] = 1.0f - (cropped ? HEIGHT_CROP_RATIO : 0);
	texs[4] = 0.0f, texs[5] = (cropped ? HEIGHT_CROP_RATIO : 0);
	texs[6] = 1.0f, texs[7] = (cropped ? HEIGHT_CROP_RATIO : 0);

	unsigned int indexes[3 * 2];
	indexes[0] = 0, indexes[1] = 1, indexes[2] = 2;
	indexes[3] = 0, indexes[4] = 3, indexes[5] = 2;

	Part part;
	{
		glGenBuffers(2, part.vaaos);
		glBindBuffer(GL_ARRAY_BUFFER, part.vaaos[0]);
		glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(GLfloat), verts, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, part.vaaos[1]);
		glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(GLfloat), texs, GL_STATIC_DRAW);

		glGenBuffers(1, &part.eao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, part.eao);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * 2 * sizeof(GLuint), indexes, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		part.elemCount = 6;
	}

	{
		glGenVertexArrays(1, &part.vao);
		glBindVertexArray(part.vao);

		GLint posLoc = pro->getAttribLocation("VertexPosition");
		glEnableVertexAttribArray(posLoc);
		glBindBuffer(GL_ARRAY_BUFFER, part.vaaos[0]);
		glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

		GLint texLoc = pro->getAttribLocation("VertexTex");
		glEnableVertexAttribArray(texLoc);
		glBindBuffer(GL_ARRAY_BUFFER, part.vaaos[1]);
		glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

		GLuint tbo;
		glGenTextures(1, &tbo);
		glBindTexture(GL_TEXTURE_RECTANGLE, tbo);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glBindTexture(GL_TEXTURE_RECTANGLE, 0);
		obj.tbos.push_back(tbo);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		obj.parts.push_back(part);
	}
}

void setupObject(GLSLProgramWrapper*& pro, Object& obj, std::string objfile, float scale, bool ignoreMultiTexture = false) {
	std::string warn, err;
	bool res = tinyobj::LoadObj(&obj.attr, &obj.shapes, &obj.materials, &warn, &err, objfile.c_str());
	if (!warn.empty()) {
		std::cerr << warn << std::endl;
	}
	if (!res) {
		std::cerr << err << std::endl;
		exit(1);
	}

	std::cout << objfile << std::endl;
	std::cout << "  verts.size() norms.size() : " << obj.attr.vertices.size() << " " << obj.attr.normals.size() << " " << std::endl;
	std::cout << "  shape.size() : " << obj.shapes.size() << std::endl;
	std::cout << "  materials.size() : " << obj.materials.size() << std::endl;

	std::cout << "  Loading shapes ..." << std::endl;
	std::vector<glm::vec3> verts;
	std::vector<glm::vec2> texs;
	for (const auto& shape : obj.shapes) {
		std::cout << "    " << shape.name << " " << shape.mesh.num_face_vertices.size();

		if (shape.mesh.num_face_vertices.size() == 0) {
			std::cout << " : skip (no meshes)" << std::endl;
			continue;
		}

		if (!ignoreMultiTexture) {
			bool flag = false;
			for (size_t i = 1; i < shape.mesh.material_ids.size(); i++) {
				if (shape.mesh.material_ids[0] != shape.mesh.material_ids[i]) {
					std::cout << " : skip (material not uniformed)" << std::endl;
					flag = true;
					break;
				}
			}
			if (flag) {
				continue;
			}
		}

		std::cout << " " << obj.materials[shape.mesh.material_ids[0]].diffuse_texname << std::endl;

		Part part;
		size_t index_offset = 0;
		for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
			int num_vertices = shape.mesh.num_face_vertices[f];
			if (num_vertices == 3) {
				for (size_t v = 0; v < 3; v++) {
					tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

					float vx = scale * obj.attr.vertices[3 * idx.vertex_index + 0];
					float vy = scale * obj.attr.vertices[3 * idx.vertex_index + 1];
					float vz = scale * obj.attr.vertices[3 * idx.vertex_index + 2];

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

		glGenBuffers(2, part.vaaos);
		glBindBuffer(GL_ARRAY_BUFFER, part.vaaos[0]);
		glBufferData(GL_ARRAY_BUFFER, 3 * verts.size() * sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, part.vaaos[1]);
		glBufferData(GL_ARRAY_BUFFER, 2 * texs.size() * sizeof(GLfloat), texs.data(), GL_STATIC_DRAW);

		part.elemCount = verts.size();

		glGenVertexArrays(1, &part.vao);
		glBindVertexArray(part.vao);

		GLint posLoc = pro->getAttribLocation("VertexPosition");
		glEnableVertexAttribArray(posLoc);
		glBindBuffer(GL_ARRAY_BUFFER, part.vaaos[0]);
		glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

		GLint texLoc = pro->getAttribLocation("VertexTex");
		glEnableVertexAttribArray(texLoc);
		glBindBuffer(GL_ARRAY_BUFFER, part.vaaos[1]);
		glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		part.tex_id = shape.mesh.material_ids[0];

		obj.parts.push_back(part);
	}

	std::cout << "  Loading textures ..." << std::endl;
	for (const auto& mat : obj.materials) {
		std::cout << "  " << mat.diffuse_texname << std::endl;
		cv::Mat img = cv::imread(mat.diffuse_texname, cv::IMREAD_COLOR);
		cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
		cv::flip(img, img, 0);

		glActiveTexture(GL_TEXTURE1);
		GLuint tbo;
		glGenTextures(1, &tbo);
		glBindTexture(GL_TEXTURE_2D, tbo);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		obj.tbos.push_back(tbo);
	}
}

Scene::Scene() {}

Scene::~Scene() { delete backPro; }

int Scene::getheight() { return HEIGHT; }

int Scene::getwidth() { return WIDTH; }

void Scene::keycallback(GLFWwindow * window, int key, int scancode, int action,
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

	img1 = cv::imread("img1.png"); cv::flip(img1, img1, 1);
	img2 = cv::imread("img2.png"); cv::flip(img2, img2, 1);
	img3 = cv::imread("img3.png"); cv::flip(img3, img3, 1);
	img4 = cv::imread("img4.png"); cv::flip(img4, img4, 1);

	setupBackground(backPro, cameraBack, frame.cols, frame.rows, true);
	setupBackground(backPro, imgBack, img1.cols, img1.rows);
	setupObject(objPro, unityChan, "sd_unitychan.obj", 0.01f);
	setupObject(objPro, gunbot, "Gun_Bot.obj", 0.5f, true);
	setupObject(objPro, spaceship, "spaceship.obj", 0.5f);
}

void Scene::draw(GLFWwindow * window) {
	if (cap.read(frame)) {
		cv::imshow("Test", frame);
		cv::waitKey(1);
	}

	// TODO
	// start image processing
	//const double kMarkerSize = 0.3;// 0.048; // [m]
	MarkerTracker markerTracker(kMarkerSize);
	glm::vec3 markerpos(0, -0.5, -1.5);
	glm::mat4 markerMat(1.0);
	std::vector<Marker> markers;
	markerTracker.findMarker(frame, markers);
	for (int i = 0; i < markers.size(); i++) {
		const int code = markers[i].code;
		for (int x = 0; x < 4; ++x) {
			for (int y = 0; y < 4; ++y) {
				markerMat[x][y] = markers[i].resultMatrix[y * 4 + x];
			}
		}
		// scaling
		markerMat[3][0] = xscale * markerMat[3][0];
		markerMat[3][1] = yscale * markerMat[3][1];
		// x-axis inversion
		for (int x = 0; x < 4; ++x) {
			markerMat[x][0] = -markerMat[x][0];
		}

		markerpos = glm::vec3(markerMat[3][0], markerMat[3][1], markerMat[3][2]);
	}

	// fin image processing


	glViewport(0, 0, WIDTH, HEIGHT);
	glClearColor(0.5, 0.5, 0.5, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 
	drawBackground(imgBack, img1);
	//drawBackground(cameraBack, frame);

	drawObj(markerpos, unityChan);
	drawObj(markerpos, spaceship);
	drawObj(markerpos, gunbot);

	glfwSwapBuffers(window);
	glfwPollEvents();
}

void Scene::drawBackground(Object obj, const cv::Mat & frame) {
	glDisable(GL_DEPTH_TEST);

	glBindTexture(GL_TEXTURE_RECTANGLE, obj.tbos[0]);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, frame.cols, frame.rows, GL_BGR, GL_UNSIGNED_BYTE, frame.data);

	backPro->enable();

	backPro->setUniform("TexHeight", (float)frame.rows);
	backPro->setUniform("TexWidth", (float)frame.cols);

	backPro->setUniform("TexImg", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, obj.tbos[0]);

	glBindVertexArray(obj.parts[0].vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.parts[0].eao);
	glDrawElements(GL_TRIANGLES, obj.parts[0].elemCount, GL_UNSIGNED_INT, 0);
}

void Scene::drawObj(glm::vec3 pos, Object obj) {
	glEnable(GL_DEPTH_TEST);

	objPro->enable();
	glm::mat4 modelMat = glm::translate(glm::identity<glm::mat4>(), pos);
	glm::mat4 viewMat = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
	glm::mat4 projMat = glm::perspective(glm::radians(60.0f), 16.0f / 9, 0.1f, 10.0f);
	objPro->setUniform("mvp", projMat * viewMat * modelMat);

	objPro->setUniform("TexImg", 1);
	glActiveTexture(GL_TEXTURE1);

	for (auto& part : obj.parts) {
		glBindTexture(GL_TEXTURE_2D, obj.tbos[part.tex_id]);

		glBindVertexArray(part.vao);
		glDrawArrays(GL_TRIANGLES, 0, part.elemCount);
	}
}
