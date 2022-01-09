#include "BufferObjects.h"
#include <SDL_assert.h>

FBO::FBO(int width, int height, GLenum textureType, GLenum format, GLenum attachment)
	:textureType(textureType), format(format), attachment(attachment),
	width(width), height(height)
{
	//fboId = fboTexture = 0;
}

FBO::~FBO()
{
	destroy();
}

void FBO::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	//glBindTexture(textureType, fboTexture);
}

bool FBO::create()
{
	// generate id
	glGenFramebuffers(1, &fboId);
	SDL_assert(fboId && "Failed instantiating framebuffer id");
	// generate texture id too?
	glGenTextures(1, &fboTexture);
	SDL_assert(fboId && "Failed instantiating framebuffer texture id");
	glBindTexture(textureType, fboTexture);

	// bind it
	bind();

	// create the framebuffer texture first
	glTexImage2D(textureType, 0, format, (GLsizei)width, (GLsizei)height, 0, format, GL_UNSIGNED_BYTE, 0);
	// specify no filtering?
	glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// specify clamp to edge
	glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// setup attachment
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textureType, fboTexture, 0);

	unbind();
	glBindTexture(textureType, 0);

	return true;
}

void FBO::destroy()
{
	// delete it
	if (fboTexture) {
		glDeleteTextures(1, &fboTexture);
	}
	if (fboId) {
		glDeleteFramebuffers(1, &fboId);
	}
}

bool FBO::isComplete()
{
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

/*
* RENDER BUFFER SECTION
*/
RBO::RBO(int width, int height, GLenum format, GLenum attachment)
	:width(width), height(height), format(format), attachment(attachment)
{
	//rboId = 0;
}

RBO::~RBO()
{
	destroy();
}

void RBO::unbind()
{
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void RBO::bind()
{
	glBindRenderbuffer(GL_RENDERBUFFER, rboId);
}

bool RBO::create()
{
	// create it
	glGenRenderbuffers(1, &rboId);
	SDL_assert(rboId && "Failed glGenRenderBuffers");
	bind();

	// generate storage
	glRenderbufferStorage(GL_RENDERBUFFER, format, (GLsizei)width, (GLsizei)height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rboId);

	return true;
}

void RBO::destroy()
{
	glDeleteRenderbuffers(1, &rboId);
}
