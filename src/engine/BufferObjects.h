#pragma once

#include <glad/glad.h>

//
// Fbo: Frambe buffer object
class FBO {
public:
	FBO(int width, int height, GLenum textureType = GL_TEXTURE_2D, 
		GLenum format = GL_RGB, GLenum attachment = GL_COLOR_ATTACHMENT0);
	~FBO();

	// static function to help unbind
	void static unbind();

	// all function of this
	void bind();
	// create it
	bool create();
	// destroy it
	void destroy();
	// check completeness
	static bool isComplete();

	GLuint getTextureId() { return fboTexture; }
	int getWidth() { return width;  }
	int getHeight() { return height; }
	float getAspect() { return (float)width / (float)height; }

protected:
	GLuint fboId;
	GLuint fboTexture;

	GLenum textureType, format, attachment;

	int width, height;
};

// 
// Rbo: Render buffer object
class RBO {
public:
	RBO(int width, int height, GLenum format = GL_DEPTH_COMPONENT24, GLenum attachment = GL_DEPTH_ATTACHMENT);
	~RBO();

	static void unbind();

	void bind();
	bool create();
	void destroy();

protected:
	GLuint rboId;
	int width, height;
	GLenum format, attachment;
};