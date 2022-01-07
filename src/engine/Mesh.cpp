#include "Mesh.h"

#include <glad/glad.h>
#include <SDL_log.h>

Mesh::~Mesh() {
	if (vbo) glDeleteBuffers(1, &vbo);
	if (ibo) glDeleteBuffers(1, &ibo);

	if (vertexBuffer) { delete[] (char*)vertexBuffer; vertexBuffer = 0; }
	if (indexBuffer) { delete[] indexBuffer; indexBuffer = 0; }
}

void Mesh::bind() const {
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

Mesh* Mesh::createBufferObjects() {
	// does it make sense?
	if (!vertexBuffer || !indexBuffer) {
		SDL_assert(vertexBuffer && indexBuffer);
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot initialize empty vertex buffer!");
		return this;
	}

	// create vbo first
	glGenBuffers(1, &vbo);
	SDL_assert(vbo);
	if (!vbo) {
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed creating vbo");
		return this;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertexBuffer, GL_STATIC_DRAW);

#ifdef _DEBUG
	SDL_Log("VB Size: %d bytes at %x", vertexBufferSize, vertexBuffer);
#endif

	// create ibo next
	glGenBuffers(1, &ibo);
	SDL_assert(ibo);

	if (!ibo) {
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed creating ibo");
		glDeleteBuffers(1, &ibo);

		return this;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, indexBuffer, GL_STATIC_DRAW);

#ifdef _DEBUG
	SDL_Log("IB Size: %d bytes at %x", indexBufferSize, indexBuffer);

	SDL_Log("====> VB-IB = %d, %d\n", vbo, ibo);
#endif

	return this;
}