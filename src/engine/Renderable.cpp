#include "Renderable.h"

void Renderable::draw(GLenum mode) const
{
	SDL_assert(shader && buffer && indexData && camera && object && setting);

	// the draw!! HUUUGE
	// 1st, bind shader
	shader->bind();

	// 2nd, bind buffer
	buffer->bind();

	// 3rd, setup vertex array state
	Renderer::setupVertexState(shader);
	Renderer::setupVertexArray(shader, buffer);

	// 4th, inject all shader inputs
	for (const ShaderInput* si : inputs) {
		si->setupData(shader);
	}
	// and matrix too
	MatrixInput mi(camera, object);
	mi.setupData(shader);

	// 5th, material setup
	Renderer::setupGLState(setting);

	// 6th, draw call
	glDrawElements(mode, indexData->elemCount, GL_UNSIGNED_SHORT, (void*)indexData->idxBegin);
}

