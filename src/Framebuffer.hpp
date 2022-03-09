#pragma once

#include <vector>


class Framebuffer
{
public:
	Framebuffer(float width = 64.0f, float height = 64.0f);

	unsigned int getId() const { return _id; }
	unsigned int getTextureId() const { return _textureID; }
	void resize(float width, float height);

	void bind(float viewportWidth, float viewportHeight);

	void unbind();

	void free();

	void update(const std::vector<unsigned char>& buffer);

private:
	unsigned int _id;
	unsigned int _textureID;
	unsigned int _rboID;

	int _width;
	int _height;
};

