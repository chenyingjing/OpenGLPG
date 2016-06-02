#include "glew.h"
#include "glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "tdogl/Camera.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

//uses stb_image to try load files
#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint gVAO = 0;
GLuint gVBO = 0;
GLuint gTex;
GLuint gProgram;
float gDegreesRotated = 45.0f;
tdogl::Camera gCamera;
double gScrollY = 0.0;

// records how far the y axis has been scrolled
void OnScroll(GLFWwindow* window, double deltaX, double deltaY) {
	gScrollY += deltaY;
}

void Update(float secondsElapsed, GLFWwindow* window) {
	//const GLfloat degreesPerSecond = 180.0f;
	const GLfloat degreesPerSecond = 0.0f;
	gDegreesRotated += secondsElapsed * degreesPerSecond;
	while (gDegreesRotated > 360.0f) gDegreesRotated -= 360.0f;

	//move position of camera based on WASD keys
	const float moveSpeed = 2.0; //units per second
	if (glfwGetKey(window, 'S')) {
		gCamera.offsetPosition(secondsElapsed * moveSpeed * -gCamera.forward());
	}
	else if (glfwGetKey(window, 'W')) {
		gCamera.offsetPosition(secondsElapsed * moveSpeed * gCamera.forward());
	}
	if (glfwGetKey(window, 'A')) {
		gCamera.offsetPosition(secondsElapsed * moveSpeed * -gCamera.right());
	}
	else if (glfwGetKey(window, 'D')) {
		gCamera.offsetPosition(secondsElapsed * moveSpeed * gCamera.right());
	}

	if (glfwGetKey(window, 'Z')) {
		gCamera.offsetPosition(secondsElapsed * moveSpeed * -glm::vec3(0, 1, 0));
	}
	else if (glfwGetKey(window, 'X')) {
		gCamera.offsetPosition(secondsElapsed * moveSpeed * glm::vec3(0, 1, 0));
	}

	//rotate camera based on mouse movement
	const float mouseSensitivity = 0.1f;
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	gCamera.offsetOrientation(mouseSensitivity * (float)mouseY, mouseSensitivity * (float)mouseX);
	glfwSetCursorPos(window, 0, 0); //reset the mouse, so it doesn't go out of the window

	const float zoomSensitivity = -0.2f;
	float fieldOfView = gCamera.fieldOfView() + zoomSensitivity * (float)gScrollY;
	if (fieldOfView < 5.0f) fieldOfView = 5.0f;
	if (fieldOfView > 130.0f) fieldOfView = 130.0f;
	gCamera.setFieldOfView(fieldOfView);
	gScrollY = 0;

}

const GLchar* ReadShader(const char* filename)
{
	FILE *infile = fopen(filename, "rb");
	//errno_t err = fopen_s(&infile, filename, "rb");
//	if (err)
//	{
//		return NULL;
//	}

	if (!infile) {
#ifdef _DEBUG
		std::cerr << "Unable to open file '" << filename << "'" << std::endl;
#endif /* DEBUG */
		return NULL;
	}

	fseek(infile, 0, SEEK_END);
	long len = ftell(infile);
	fseek(infile, 0, SEEK_SET);

	GLchar* source = new GLchar[len + 1];
	fread((void *)source, 1, len, infile);
	source[len] = 0;
	fclose(infile);

	return source;
}

void init()
{
	gProgram = glCreateProgram();


	{
		GLuint shader = glCreateShader(GL_VERTEX_SHADER);
		const GLchar* source = ReadShader("vertex-shader.txt");
		if (source == NULL)
		{
			return;
		}
		glShaderSource(shader, 1, &source, NULL);
		delete[] source;
		glCompileShader(shader);
		GLint status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			std::string msg("Compile failure in shader:\n");

			GLint infoLogLength;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
			char* strInfoLog = new char[infoLogLength + 1];
			glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
			msg += strInfoLog;
			delete[] strInfoLog;

			glDeleteShader(shader); shader = 0;
			throw std::runtime_error(msg);
		}
		glAttachShader(gProgram, shader);
	}

	{
		GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
		const GLchar* source = ReadShader("fragment-shader.txt");
		if (source == NULL)
		{
			return;
		}
		glShaderSource(shader, 1, &source, NULL);
		delete[] source;
		glCompileShader(shader);
		GLint status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			std::string msg("Compile failure in shader:\n");

			GLint infoLogLength;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
			char* strInfoLog = new char[infoLogLength + 1];
			glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
			msg += strInfoLog;
			delete[] strInfoLog;

			glDeleteShader(shader); shader = 0;
			throw std::runtime_error(msg);
		}
		glAttachShader(gProgram, shader);
	}

	glLinkProgram(gProgram);

	//glDetachShader(program, shader);

	GLint linkStatus;
	glGetProgramiv(gProgram, GL_LINK_STATUS, &linkStatus);
	if (linkStatus == GL_FALSE)
	{
		std::string msg("Program linking failure: ");

		GLint infoLogLength;
		glGetProgramiv(gProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* strInfoLog = new char[infoLogLength + 1];
		glGetProgramInfoLog(gProgram, infoLogLength, NULL, strInfoLog);
		msg += strInfoLog;
		delete[] strInfoLog;

		glDeleteProgram(gProgram); gProgram = 0;
		throw std::runtime_error(msg);
	}

	glUseProgram(gProgram);

/*
	glm::mat4 projection = glm::perspective(glm::radians(50.0f), 800.0f / 600.0f, 0.1f, 10.0f);
	GLint uniformProjection = glGetUniformLocation(gProgram, "projection");
	if (uniformProjection == -1)
		throw std::runtime_error(std::string("Program uniform not found: ") + "projection");
	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));


	glm::mat4 camera = glm::lookAt(glm::vec3(3, 3, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	//glm::mat4 camera = glm::lookAt(glm::vec3(0.5, 0.5, 0.5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	GLint uniformCamera = glGetUniformLocation(gProgram, "camera");
	if (uniformCamera == -1)
		throw std::runtime_error(std::string("Program uniform not found: ") + "camera");

	glUniformMatrix4fv(uniformCamera, 1, GL_FALSE, glm::value_ptr(camera));
*/
	

	int width, height, channels;
	unsigned char* pixels = stbi_load("wooden-crate.jpg", &width, &height, &channels, 0);
	if (!pixels)
	{
		throw std::runtime_error(stbi_failure_reason());
	}

	unsigned long rowSize = channels *width;
	unsigned char* rowBuffer = new unsigned char[rowSize];
	unsigned halfRows = height / 2;

	for (unsigned rowIdx = 0; rowIdx < halfRows; ++rowIdx) {
		unsigned char* row = pixels + (rowIdx * width + 0) * channels;//GetPixelOffset(0, rowIdx, _width, _height, _format);
		unsigned char* oppositeRow = pixels + ((height - rowIdx - 1) * width + 0) * channels;//GetPixelOffset(0, _height - rowIdx - 1, _width, _height, _format);

		memcpy(rowBuffer, row, rowSize);
		memcpy(row, oppositeRow, rowSize);
		memcpy(oppositeRow, rowBuffer, rowSize);
	}

	delete[] rowBuffer;

	glGenTextures(1, &gTex);
	glBindTexture(GL_TEXTURE_2D, gTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLint internalformat;
	switch (channels) {
	case 1: internalformat = GL_LUMINANCE; break;
	case 2: internalformat = GL_LUMINANCE_ALPHA; break;
	case 3: internalformat = GL_RGB; break;
	case 4: internalformat = GL_RGBA; break;
	default: throw std::runtime_error("Unrecognised Bitmap::Format");
	}

	glTexImage2D(GL_TEXTURE_2D,
		0,
		internalformat,
		(GLsizei)width,
		(GLsizei)height,
		0,
		internalformat,
		GL_UNSIGNED_BYTE,
		pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(pixels);

	
	
	// make and bind the VAO
	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);

	// make and bind the VBO
	glGenBuffers(1, &gVBO);
	glBindBuffer(GL_ARRAY_BUFFER, gVBO);

	GLfloat vertexData[] = {
		//  X     Y     Z       U     V
		// bottom
		-1.0f,-1.0f,-1.0f,   0.0f, 0.0f,
		1.0f,-1.0f,-1.0f,   1.0f, 0.0f,
		-1.0f,-1.0f, 1.0f,   0.0f, 1.0f,
		1.0f,-1.0f,-1.0f,   1.0f, 0.0f,
		1.0f,-1.0f, 1.0f,   1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,   0.0f, 1.0f,

		// top
		-1.0f, 1.0f,-1.0f,   0.0f, 0.0f,
		-1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
		1.0f, 1.0f,-1.0f,   1.0f, 0.0f,
		1.0f, 1.0f,-1.0f,   1.0f, 0.0f,
		-1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,   1.0f, 1.0f,

		// front
		-1.0f,-1.0f, 1.0f,   1.0f, 0.0f,
		1.0f,-1.0f, 1.0f,   0.0f, 0.0f,
		-1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,   0.0f, 0.0f,
		1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,   1.0f, 1.0f,

		// back
		-1.0f,-1.0f,-1.0f,   0.0f, 0.0f,
		-1.0f, 1.0f,-1.0f,   0.0f, 1.0f,
		1.0f,-1.0f,-1.0f,   1.0f, 0.0f,
		1.0f,-1.0f,-1.0f,   1.0f, 0.0f,
		-1.0f, 1.0f,-1.0f,   0.0f, 1.0f,
		1.0f, 1.0f,-1.0f,   1.0f, 1.0f,

		// left
		-1.0f,-1.0f, 1.0f,   0.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,   1.0f, 0.0f,
		-1.0f,-1.0f,-1.0f,   0.0f, 0.0f,
		-1.0f,-1.0f, 1.0f,   0.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,   1.0f, 0.0f,

		// right
		1.0f,-1.0f, 1.0f,   1.0f, 1.0f,
		1.0f,-1.0f,-1.0f,   1.0f, 0.0f,
		1.0f, 1.0f,-1.0f,   0.0f, 0.0f,
		1.0f,-1.0f, 1.0f,   1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,   0.0f, 0.0f,
		1.0f, 1.0f, 1.0f,   0.0f, 1.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	GLint attrib = glGetAttribLocation(gProgram, "vert");
	if (attrib == -1)
		throw std::runtime_error(std::string("Program attribute not found: ") + "vert");

	glEnableVertexAttribArray(attrib);
	glVertexAttribPointer(attrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), NULL);

	GLint vertTexCoord = glGetAttribLocation(gProgram, "vertTexCoord");
	if (vertTexCoord == -1)
		throw std::runtime_error(std::string("Program attribute not found: ") + "vertTexCoord");

	glEnableVertexAttribArray(vertTexCoord);
	glVertexAttribPointer(vertTexCoord, 2, GL_FLOAT, GL_TRUE, 5 * sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));


	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glClearColor(0.196078431372549f, 0.3137254901960784f, 0.5882352941176471f, 1);
}

void Render(GLFWwindow* window)
{
	// clear everything
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLint uniformCamera = glGetUniformLocation(gProgram, "camera");
	if (uniformCamera == -1)
		throw std::runtime_error(std::string("Program uniform not found: ") + "camera");

	glUniformMatrix4fv(uniformCamera, 1, GL_FALSE, glm::value_ptr(gCamera.matrix()));


	GLint uniformModel = glGetUniformLocation(gProgram, "model");
	if (uniformModel == -1)
		throw std::runtime_error(std::string("Program uniform not found: ") + "tex");

	glm::tmat4x4<float, glm::highp> modelMatrix = glm::rotate(glm::mat4(), glm::radians(gDegreesRotated), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelMatrix));


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTex);

	GLint uniformTex = glGetUniformLocation(gProgram, "tex");
	if (uniformTex == -1)
		throw std::runtime_error(std::string("Program uniform not found: ") + "tex");

	glUniform1i(uniformTex, 0);



	glBindVertexArray(gVAO);

	// draw the VAO
	glDrawArrays(GL_TRIANGLES, 0, 6 * 2 * 3);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glfwSwapBuffers(window);
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(800, 600, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwSetScrollCallback(window, OnScroll);

	// GLFW settings
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, 0, 0);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE; //stops glew crashing on OSX :-/
	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		return -1;
	}

	// print out some info about the graphics drivers
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

	// OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	init();

	gCamera.setPosition(glm::vec3(0, 0, 4));
	gCamera.setViewportAspectRatio(800.0f / 600.0f);

	double lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		/* Poll for and process events */
		glfwPollEvents();

		double thisTime = glfwGetTime();
		Update((float)(thisTime - lastTime), window);
		lastTime = thisTime;

		Render(window);

		// check for errors
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
			std::cerr << "OpenGL Error " << error << std::endl;

		//exit program if escape key is pressed
		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
			glfwSetWindowShouldClose(window, GL_TRUE);
	}

	glfwTerminate();
	return 0;
}
