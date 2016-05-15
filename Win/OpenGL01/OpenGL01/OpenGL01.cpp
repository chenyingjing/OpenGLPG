#include "glew.h"
#include "glfw3.h"
#include "glm/glm.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace glm;


GLuint gVAO = 0;
GLuint gVBO = 0;

const GLchar* ReadShader(const char* filename)
{
	FILE *infile;
	errno_t err = fopen_s(&infile, filename, "rb");
	if (err)
	{
		return NULL;
	}

	if (!infile) {
#ifdef _DEBUG
		std::cerr << "Unable to open file '" << filename << "'" << std::endl;
#endif /* DEBUG */
		return NULL;
	}

	fseek(infile, 0, SEEK_END);
	int len = ftell(infile);
	fseek(infile, 0, SEEK_SET);

	GLchar* source = new GLchar[len + 1];
	fread((void *)source, 1, len, infile);
	source[len] = 0;
	fclose(infile);

	return source;
}

void init()
{
	GLuint program = glCreateProgram();


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
		glAttachShader(program, shader);
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
		glAttachShader(program, shader);
	}

	glLinkProgram(program);

	//glDetachShader(program, shader);

	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (linkStatus == GL_FALSE)
	{
		std::string msg("Program linking failure: ");

		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* strInfoLog = new char[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		msg += strInfoLog;
		delete[] strInfoLog;

		glDeleteProgram(program); program = 0;
		throw std::runtime_error(msg);
	}

	glUseProgram(program);

	// make and bind the VAO
	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);

	// make and bind the VBO
	glGenBuffers(1, &gVBO);
	glBindBuffer(GL_ARRAY_BUFFER, gVBO);

	GLfloat vertexData[] = {
		//  X     Y     Z
		0.0f, 0.8f, 0.0f,
		-0.8f,-0.8f, 0.0f,
		0.8f,-0.8f, 0.0f,
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	GLint attrib = glGetAttribLocation(program, "vert1");
	if (attrib == -1)
		throw std::runtime_error(std::string("Program attribute not found: ") + "vert1");

	glEnableVertexAttribArray(attrib);
	glVertexAttribPointer(attrib, 3, GL_FLOAT, GL_FALSE, 0, NULL);


	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glClearColor(0.6784313725490196, 0.607843137254902, 0.3215686274509804, 1);
}

void Render(GLFWwindow* window)
{
	// clear everything
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(gVAO);

	// draw the VAO
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);

	glfwSwapBuffers(window);
}

int main(void)
{
	//mat2 m(0, 1, 2, 3);

	//mat2 result = m * m;

	//if (result == mat2(2, 3, 6, 11)) {
	//	std::cout << "OK\n";
	//}
	//else {
	//	std::cout << "NG\n";
	//}

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;



	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

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

	init();

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Poll for and process events */
		glfwPollEvents();

		Render(window);
	}

	glfwTerminate();
	return 0;
}
