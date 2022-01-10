#pragma once

#include <glad/glad.h>
#include <vector>

// FboAttachment (base for fbo attachment)
class FBO;
class FBOAttachment {
public:
	FBOAttachment() {}
	virtual ~FBOAttachment() {}

	// when creating?
	virtual bool onResize(int w, int h) = 0;

	// set it?
	void setFBO(FBO* f) { fbo = f; }

	FBO* fbo;
};

//
// Fbo: Frambe buffer object
class FBO {
public:
	FBO();
	~FBO();

	// static function to help unbind
	void static unbind();
	// all function of this
	void bind();
	int addAttachment(FBOAttachment* att);
	FBOAttachment* getAttachment(int idx) { return attachments[idx]; }
	// check completeness
	static bool isComplete();

	// force to POT in the closest range?
	// will resize the internal w & h
	void resizeToPOT(int w, int h);
	void resize(int w, int h);
	void onResize();

public:
	GLuint fboId;
	int width, height;

	std::vector<FBOAttachment*> attachments;
};

// 
// Rbo: Render buffer object
class RBOAttachment : public FBOAttachment {
public:
	RBOAttachment(GLenum format = GL_DEPTH_COMPONENT24, GLenum attachment = GL_DEPTH_ATTACHMENT);
	~RBOAttachment();

	// Inherited via FBOAttachment
	virtual bool onResize(int w, int h) override;
protected:
	GLuint rboId;
	GLenum format, attachment;

};

// 
// Texture attachment
class TextureAttachment : public FBOAttachment {
public:
	TextureAttachment(GLenum textureType = GL_TEXTURE_2D, GLenum format = GL_RGB, GLenum byteType = GL_UNSIGNED_BYTE, GLenum attachment = GL_COLOR_ATTACHMENT0);
	~TextureAttachment();

	// Inherited via FBOAttachment
	virtual bool onResize(int w, int h) override;
public:
	GLuint texId;
	GLenum format, attachment, textureType, byteType;
};