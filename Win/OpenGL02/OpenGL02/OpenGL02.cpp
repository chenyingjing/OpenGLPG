#include "glew.h"
#include "glfw3.h"
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


	int width, height, channels;
	unsigned char* pixels = stbi_load("hazard.png", &width, &height, &channels, 0);
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

	delete rowBuffer;

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
		0.0f, 0.8f, 0.0f,   0.5f, 1.0f,
		-0.8f,-0.8f, 0.0f,   0.0f, 0.0f,
		0.8f,-0.8f, 0.0f,   1.0f, 0.0f,
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
	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTex);

	GLint uniform = glGetUniformLocation(gProgram, "tex");
	if (uniform == -1)
		throw std::runtime_error(std::string("Program uniform not found: ") + "tex");

	glUniform1i(uniform, 0);



	glBindVertexArray(gVAO);

	// draw the VAO
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

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
	window = glfwCreateWindow(800, 600, "Hello World", NULL, NULL);
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

	// OpenGL settings
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
