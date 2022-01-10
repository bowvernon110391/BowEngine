#include "BufferObjects.h"
#include "Helper.h"
#include <SDL_assert.h>


FBO::FBO()
	:width(0), height(0)
{
	fboId = 0;
	glGenFramebuffers(1, &fboId);
}

FBO::~FBO()
{
	// delete all attachment, then delete ourselves
	for (FBOAttachment* att : attachments) {
		if (att)
			delete att;
	}
	attachments.clear();

	glDeleteFramebuffers(1, &fboId);
}

void FBO::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
}

int FBO::addAttachment(FBOAttachment* att)
{
	if (!att)
		return false;
	int idx = attachments.size();
	// just add, nothing more
	att->setFBO(this);
	attachments.push_back(att);
	return idx;
}

bool FBO::isComplete()
{
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void FBO::resizeToPOT(int w, int h)
{
	resize(Helper::getNearestPOT(w), Helper::getNearestPOT(h));
}

void FBO::resize(int w, int h)
{
	if (width != w || height != h) {
		width = w;
		height = h;
		onResize();
	}
}

void FBO::onResize()
{
	// do the resizing?
	// we assume the width and height is already set
	bind();
	for (FBOAttachment* att : attachments) {
		att->onResize(width, height);
	}
}

RBOAttachment::RBOAttachment(GLenum format, GLenum attachment)
	:format(format), attachment(attachment)
{
	glGenRenderbuffers(1, &rboId);
}

RBOAttachment::~RBOAttachment()
{
	glDeleteRenderbuffers(1, &rboId);
}

bool RBOAttachment::onResize(int w, int h)
{
	// first, bind it?
	glBindRenderbuffer(GL_RENDERBUFFER, rboId);
	// storage specifier
	glRenderbufferStorage(GL_RENDERBUFFER, format, w, h);
	// set attachment
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rboId);

	// unbind it
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return true;
}

/*
* Texture attachment for this
*/

TextureAttachment::TextureAttachment(GLenum textureType, GLenum format, GLenum byteType, GLenum attachment)
	:textureType(textureType), format(format), attachment(attachment),
	byteType(byteType)
{
	glGenTextures(1, &texId);
}

TextureAttachment::~TextureAttachment()
{
	glDeleteTextures(1, &texId);
}

bool TextureAttachment::onResize(int w, int h)
{
	// create it?
	glBindTexture(textureType, texId);

	glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(textureType, 0, format, w, h, 0, format, byteType, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textureType, texId, 0);

	// unbind texture
	glBindTexture(textureType, 0);
	return true;
}
