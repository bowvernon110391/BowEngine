#include "Game.h"
#include "Shader.h"
#include "Texture2d.h"
#include "Mesh.h"
#include "Material.h"
#include "LargeMesh.h"
#include "ShaderSource.h"
#include "MaterialInputs.h"

ShaderInput* Game::loadBasicShaderData(const char* name, void* pdata)
{
	return new MaterialInput();
}

Texture2D* Game::loadTexture(const char* name, void* pdata) {
	// GLenum error;
	// error = glGetError();
	// assert(error == GL_NO_ERROR);

	// load the texture, do not set anything yet?
	Texture2D* tex = Texture2D::loadFromFile(name, true)->withFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	// error = glGetError();
	// assert(error == GL_NO_ERROR);

	return tex;
}

Mesh* Game::loadMesh(const char* name, void* pdata) {
	return Mesh::loadBCFFromFile(name)->createBufferObjects();
}

Material* Game::loadBasicMaterial(const char* name, void* pdata) {
	// just spawn empty material
	return new Material();
}

LargeMesh* Game::loadLargeMesh(const char* name, void* pdata)
{
	return LargeMesh::loadLMFFromFile(name)->createBufferObjects();
}

ShaderSource* Game::loadShaderSource(const char* name, void* pdata)
{
	char* buf = Helper::readFileContent(name);

	ShaderSource* src = new ShaderSource(buf);

	delete[] buf;
	return src;
}
