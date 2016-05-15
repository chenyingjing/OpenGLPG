#include <iostream>

//uses stb_image to try load files
#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;


void main()
{
	cout << "hello" << endl;

	int width, height, channels;
	unsigned char* pixels = stbi_load("bmp1.bmp", &width, &height, &channels, 0);
	if (!pixels)
	{
		throw std::runtime_error(stbi_failure_reason());
	}

	stbi_image_free(pixels);
}
