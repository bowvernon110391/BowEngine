#include "MatrixInput.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

//#define DEBUG_MI

void MatrixInput::setupData(const Shader* shd) const
{
#ifdef DEBUG_MI
	printf("MATRIX_INPUT_START\n");
#endif

	// make sure it's valid before use
	SDL_assert(cam && mdl);

	// first, store tmp matrix
	glm::mat4 model = glm::mat4(1.0);	// for model, and mvp?
	const glm::mat4& view = cam->getViewMatrix();
	const glm::mat4& proj = cam->getProjectionMatrix();

	int u_loc;
	bool has_model = mdl->getTransform(&model);

#ifdef DEBUG_MI
	printf("MI: MODEL MATRIX? %s\n", (has_model ? "YES":"NO"));
	Helper::printMatrix(glm::value_ptr(model));
#endif


	// model matrix
	u_loc = shd->getUniformLocation(Shader::UniformLoc::m_model);
	if (u_loc >= 0 && has_model) {
#ifdef DEBUG_MI
		printf("MI: m_model(%d)\n", u_loc);
		Helper::printMatrix(glm::value_ptr(model));
#endif
		glUniformMatrix4fv(u_loc, 1, false, glm::value_ptr(model));
	}

	// view matrix
	u_loc = shd->getUniformLocation(Shader::UniformLoc::m_view);
	if (u_loc >= 0) {
		
#ifdef DEBUG_MI
		printf("MI: m_view(%d)\n", u_loc);
		Helper::printMatrix(glm::value_ptr(view));
#endif
		glUniformMatrix4fv(u_loc, 1, false, glm::value_ptr(view));
	}

	// projection matrix
	u_loc = shd->getUniformLocation(Shader::UniformLoc::m_projection);
	if (u_loc >= 0) {
#ifdef DEBUG_MI
		printf("MI: m_projection(%d)\n", u_loc);
		Helper::printMatrix(glm::value_ptr(proj));
#endif
		glUniformMatrix4fv(u_loc, 1, false, glm::value_ptr(proj));
	}

	// model view
	u_loc = shd->getUniformLocation(Shader::UniformLoc::m_model_view);
	glm::mat4 model_view;
	if (u_loc >= 0 && has_model) {
		model_view = cam->getViewMatrix() * model;
#ifdef DEBUG_MI
		printf("MI: m_model_view(%d)\n", u_loc);
		Helper::printMatrix(glm::value_ptr(model_view));
#endif
		glUniformMatrix4fv(u_loc, 1, false, glm::value_ptr(model_view));
	}

	// normal matrix
	u_loc = shd->getUniformLocation(Shader::UniformLoc::m_normal);
	if (u_loc >= 0) {
		glm::mat3 normal = glm::mat3(model_view);
		glUniformMatrix3fv(u_loc, 1, false, glm::value_ptr(normal));
	}

	// model view projection
	u_loc = shd->getUniformLocation(Shader::UniformLoc::m_model_view_projection);
	if (u_loc >= 0 && has_model) {
		glm::mat4 model_view_projection = proj * view * model;
#ifdef DEBUG_MI
		printf("MI: m_model_view_projection(%d)\n", u_loc);
		Helper::printMatrix(glm::value_ptr(model_view_projection));
#endif

		glUniformMatrix4fv(u_loc, 1, false, glm::value_ptr(model_view_projection));
	}

#ifdef DEBUG_MI
	printf("MATRIX_INPUT_END\n");
#endif

}
