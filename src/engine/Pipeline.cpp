#include "Pipeline.h"
#include "Renderer.h"

void Pipeline::setupData(const Shader* shd) const
{
	SDL_assert(renderer != nullptr);
	// set global data (time, viewport)
	int u_loc = shd->getUniformLocation(Shader::UniformLoc::time);
	if (u_loc >= 0) {
		glUniform1f(u_loc, renderer->render_time);
	}

	// viewport
	u_loc = shd->getUniformLocation(Shader::UniformLoc::viewport_dimension);
	const int* v = &renderer->viewport[0];
	if (u_loc >= 0) {
		glUniform4i(u_loc, v[0], v[1], v[2], v[3]);
	}
}
