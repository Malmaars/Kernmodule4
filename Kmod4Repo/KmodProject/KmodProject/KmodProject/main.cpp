#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include<fstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


struct Light {
	glm::vec3 position;
	glm::vec3 color;

	Light(const glm::vec3& pos, const glm::vec3& col) : position(pos), color(col) {}
};

//Build -> rebuild Solution, Debug -> Start without debugging

//Waarom doen we de functies met kleine letters? Is dat iets wat moet met opengl
//Of is dat een regel bij opengl programmers?
//En waarom moeten we überhaupt de functies declareren voordat we ze ook echt aanmaken?

//Forward Declaration
int init(GLFWwindow* &window);
void createShaders();
void createProgram(GLuint& programID, const char* vertex, const char* fragment);
void createTGeometry(GLuint& vao, int& size, int& numIndices);
GLuint loadTexture(const char* path);

//Util
void loadFile(const char* filename, char*& output);

//Program IDs
GLuint simpleProgram;

const int WIDTH = 1280, HEIGHT = 720;

int main() {
	
	GLFWwindow* window;
	int res = init(window);

	if (res != 0) {
		return res;
	}

	GLuint triangleVAO;
	int triangleSize;
	int triangleIndexCount;
	createTGeometry(triangleVAO, triangleSize, triangleIndexCount);
	createShaders();

	GLuint boxTex = loadTexture("textures./container2.png");
	GLuint boxNormal = loadTexture("textures./container2_normal.png");

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	//set texture channels
	glUseProgram(simpleProgram);
	glUniform1i(glGetUniformLocation(simpleProgram, "mainTex"), 0);
	glUniform1i(glGetUniformLocation(simpleProgram, "normalTex"), 1);

	//Create gl Viewport
	glViewport(0, 0, WIDTH, HEIGHT);

	//instead of a single light position and color, we will make multiple
	std::vector<Light> lights = {
		Light(glm::vec3(2, 2, 2), glm::vec3(1.0f, 0.0f, 0.0f)),
		Light(glm::vec3(0, -2, -2), glm::vec3(0.0f, 0.0f, 1.0f)),
		Light(glm::vec3(-2, -2, -2), glm::vec3(0.0f, 1.0f, 0.0f))
	};

	glm::vec3 cameraPosition = glm::vec3(0, 2.5f, -5.0f);
	//Matrices!

	glm::mat4 world = glm::mat4(1.0f);
	world = glm::rotate(world, glm::radians(45.0f), glm::vec3(0, 1, 0));
	world = glm::scale(world, glm::vec3(1, 1, 1));
	world = glm::translate(world, glm::vec3(0, 0, 0));

	glm::mat4 view = glm::lookAt(cameraPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	glm::mat4 projection = glm::perspective(glm::radians(25.0f), WIDTH / (float)HEIGHT, 0.1f, 100.0f);

	// Time variables for rotation
	float lastFrameTime = glfwGetTime();
	float totalTime = 0.0f;
		
	//Render Loop
	while(!glfwWindowShouldClose(window)) {
		
		// Calculate frame time for smoother animations
		float currentFrameTime = glfwGetTime();
		float deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;
		totalTime += deltaTime;
		
		//events pollen
		glfwPollEvents();

		//rendering

		// Calculate the world matrix with rotation
		glm::mat4 world = glm::mat4(1.0f);
		world = glm::rotate(world, totalTime * glm::radians(50.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around x-axis 
		world = glm::rotate(world, totalTime * glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around y-axis 
		world = glm::rotate(world, totalTime * glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around z-axis

		//glClearColor(0.1f, 0.8f, 0.76f, 0); 
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f); 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

		glUseProgram(simpleProgram); 


		glUniformMatrix4fv(glGetUniformLocation(simpleProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
		glUniformMatrix4fv(glGetUniformLocation(simpleProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(simpleProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glUniform3fv(glGetUniformLocation(simpleProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

		// Send light data to shader
		for (size_t i = 0; i < lights.size(); ++i) {
			std::string lightPositionName = "lights[" + std::to_string(i) + "].position";
			std::string lightColorName = "lights[" + std::to_string(i) + "].color";

			glUniform3fv(glGetUniformLocation(simpleProgram, lightPositionName.c_str()), 1, glm::value_ptr(lights[i].position));
			glUniform3fv(glGetUniformLocation(simpleProgram, lightColorName.c_str()), 1, glm::value_ptr(lights[i].color));
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, boxTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, boxNormal);

		glBindVertexArray(triangleVAO);
		//glDrawArrays(GL_TRIANGLES, 0, triangleSize);
		glDrawElements(GL_TRIANGLES, triangleIndexCount, GL_UNSIGNED_INT, 0);

		//buffers swappen
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

int init(GLFWwindow*&window) {
	//init glfw
	glfwInit();
	//window hints
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create window & make context current
	window = glfwCreateWindow(WIDTH, HEIGHT, "Hello World!", NULL, NULL);

	if (window == NULL) {
		//error
		std::cout << "Failed to create window!" << std::endl;
		glfwTerminate();
		return-1;
	}

	glfwMakeContextCurrent(window);

	//Load GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	return 0;
}

void createTGeometry(GLuint& vao, int& size, int& numIndices) {
	
	//to create different triangles, I'd want this as a parameter, but I don't understand c++ well enough to do that
	//in  fact, I understand so little of c++, I don't even know what to look up to get this to work
	
	// need 24 vertices for normal/uv-mapped Cube
	float vertices[] = {
		// positions            //colors            // tex coords   // normals          //tangents      //bitangents
		0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,

		0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,
		0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 0.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,

		0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,
		-0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,

		-0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,      -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,      -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,

		-0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,      0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,
		0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,      0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,

		-0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,

		0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,
		-0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,

		-0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   1.f, 1.f,       -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,

		-0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,
		0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,

		0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,
		0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,

		0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f
	};

	unsigned int indices[] = {  // note that we start from 0!
		// DOWN
		0, 1, 2,   // first triangle
		0, 2, 3,    // second triangle
		// BACK
		14, 6, 7,   // first triangle
		14, 7, 15,    // second triangle
		// RIGHT
		20, 4, 5,   // first triangle
		20, 5, 21,    // second triangle
		// LEFT
		16, 8, 9,   // first triangle
		16, 9, 17,    // second triangle
		// FRONT
		18, 10, 11,   // first triangle
		18, 11, 19,    // second triangle
		// UP
		22, 12, 13,   // first triangle
		22, 13, 23,    // second triangle
	};

	int stride = (3 + 3 + 2 + 3 + 3 + 3) * sizeof(float);

	size = sizeof(vertices) / stride;
	numIndices = sizeof(indices) / sizeof(int);

	unsigned int EBO;
	glGenBuffers(1, &EBO);

	glGenVertexArrays(1, &vao);
	//ik snap nog niet helemaal hoe de & nou werkt. Waarom hier dan weer niet?
	glBindVertexArray(vao);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//ik hoef niet heel goed te begrijpen hoe dit werkt. cool, want ik begrijp het niet :)
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	//colors
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, stride, (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 3, GL_FLOAT, GL_TRUE, stride, (void*)(11 * sizeof(float)));
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(5, 3, GL_FLOAT, GL_TRUE, stride, (void*)(14 * sizeof(float)));
	glEnableVertexAttribArray(5);
}

void createShaders() {
	createProgram(simpleProgram, "shaders/simpleVertexShader.shader", "shaders/fragmentShader.shader");
}

void createProgram(GLuint& programID, const char* vertex, const char* fragment) {
	//Create a GL program with a vertex & fragment shader
	char* vertexSrc;
	char* fragmentSrc;
	loadFile(vertex, vertexSrc);
	loadFile(fragment, fragmentSrc);

	GLuint vertexShaderID, fragmentShaderID;

	vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID, 1, &vertexSrc, nullptr);
	glCompileShader(vertexShaderID);

	int success;
	char infoLog[512];
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShaderID, 512, nullptr, infoLog);
		std::cout << "ERROR COMPILING VERTEX SHADER\n" << infoLog << std::endl;
	}

	fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, &fragmentSrc, nullptr);
	glCompileShader(fragmentShaderID);

	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShaderID, 512, nullptr, infoLog);
		std::cout << "ERROR COMPILING FRAGMENT SHADER\n" << infoLog << std::endl;
	}

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(programID, 512, nullptr, infoLog);
		std::cout << "ERROR LINKING PROGRAM\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	delete vertexSrc;
	delete fragmentSrc;
}

void loadFile(const char* filename, char*& output) {
	//open the file
	std::ifstream file(filename, std::ios::binary);

	//if the file was succesfully opened
	if (file.is_open()) {
		//get length of file
		file.seekg(0, file.end);
		int length = file.tellg();
		file.seekg(0, file.beg);

		//allocate memort for the char pointer
		output = new char[length + 1];

		//read data as a block
		file.read(output, length);

		//add null terminator to end of char pointer
		output[length] = '\0';

		//close the file
		file.close();
	}
	else {
		//if the file failed to open, set the char pointer to NULL
		output = NULL;
	}


}

GLuint loadTexture(const char* path) 
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, numChannels;
	unsigned char* data = stbi_load(path, &width, &height, &numChannels, 0);
	if (data) {
		if (numChannels == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}

		if (numChannels == 4) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);

	}
	else {
		std::cout << "Error loading texture: " << path << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}

//https://glad.dav1d.de/