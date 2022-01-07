#ifndef __MUH_TEX2D__
#define __MUH_TEX2D__

#include <glad/glad.h>
#include <cstddef>
#include "Resource.h"

class Texture2D : public Resource {
public:
	GLuint texId;
	int width, height;
	unsigned char* texData;
	GLint format, minFilter, magFilter, wrapS, wrapT;
	bool retainPixelData, ownPixelData, useMipmap;

	Texture2D() {
		retainPixelData = false;
		ownPixelData = false;

		texId = 0;
		width = height = 0;
		texData = NULL;
		format = GL_RGBA;
		minFilter = GL_LINEAR;
		magFilter = GL_LINEAR;
		wrapS = GL_CLAMP_TO_EDGE;
		wrapT = GL_CLAMP_TO_EDGE;
		useMipmap = false;
	}

	virtual ~Texture2D() {
		if (texData) delete[] texData;
		destroyHandle();
	}

	virtual const char* type() { return "TEXTURE_2D"; }

	bool createHandle() {
		glGenTextures(1, &texId);

		if (!texId)
			return false;

		return true;
	}

	virtual void generateMipMap();

	bool upload(bool retainPixelData = false);

	void bind() {
		glBindTexture(GL_TEXTURE_2D, texId);
	}

	Texture2D* withFilter(GLint minFilter, GLint magFilter) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
		return this;
	}

	Texture2D* withWrap(GLint wrapS, GLint wrapT) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		return this;
	}

	void destroyHandle() {
		if (texId) {
			glDeleteTextures(1, &texId);
		}
	}

	static Texture2D* loadFromFile(const char* filename,
		bool useMipmap = false,
		GLint minFilter = GL_LINEAR, GLint magFilter = GL_LINEAR,
		GLint wrapS = GL_CLAMP_TO_EDGE, GLint wrapT = GL_CLAMP_TO_EDGE);

	static Texture2D* loadFromMemory(const char* buf, int bufSize,
		bool useMipmap = false,
		GLint minFilter = GL_LINEAR, GLint magFilter = GL_LINEAR,
		GLint wrapS = GL_CLAMP_TO_EDGE, GLint wrapT = GL_CLAMP_TO_EDGE);
};

#endif