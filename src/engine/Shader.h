#ifndef __MUH_SHADER_H__
#define __MUH_SHADER_H__

#include <glad/glad.h>
#include <vector>
#include <string>
#include "Helper.h"
#include "Resource.h"
#include "ShaderSource.h"

#define U_SET(x) uniformLoc[(x)] >= 0
#define U_LOC(x) uniformLoc[(x)]

#define SU(x) Shader::UniformLoc::x
#define SU_LOC(s, x) (s->getUniformLocation(SU(x)))
#define SU_SET(s, x) (SU_LOC(x) >= 0)

//#define DEBUG_SHADER
template<typename Resource>
class ResourceManager;

class Shader : public Resource
{
	friend class ResourceManager<Shader>;
public:
	enum UniformLoc {
#define DECLARE_ENUM(x) x,
#include "shader_uniforms.h"
		custom_uniform
	};

	enum AttribLoc {
#include "shader_attributes.h"
		custom_attribute
	};

	// bind program
	void bind() const {
		glUseProgram(programId);
	}

	virtual const char* type() {
		return "SHADER";
	}
protected:

	static int getUniformId(const char* name) {
		static std::vector<std::string> uniformIds(32);
		static bool initialized = false;

		if (!initialized) {
			uniformIds.clear();
			const char* uniformNames[] = {
#undef DECLARE_ENUM
#define DECLARE_ENUM(x) #x ,
#include "shader_uniforms.h"
				"invalid"
			};
			initialized = true;
			for (int i = 0; i < sizeof(uniformNames) / sizeof(uniformNames[0]); i++) {
				uniformIds.push_back(uniformNames[i]);
			}
		}

		// can we find it there?
		std::string toSearch(name);
		// do linear search
		SDL_Log("searching over %d uniform entries...\n", uniformIds.size());
		for (size_t i = 0; i < uniformIds.size(); i++) {
			if (uniformIds[i] == toSearch) {
				return i;
			}
		}

		return -1;
	}

	static int getAttributeId(const char* name) {
		static std::vector<std::string> attributeIds;
		static bool initialized = false;

		if (!initialized) {
			attributeIds.clear();
			const char* attributeNames[] = {
#include "shader_attributes.h"
				"invalid"
			};
			initialized = true;
			for (int i = 0; i < sizeof(attributeNames) / sizeof(attributeNames[0]); i++) {
				attributeIds.push_back(attributeNames[i]);
			}
		}

		// do linear search
		std::string toSearch(name);
		for (size_t i = 0; i < attributeIds.size(); i++) {
			if (attributeIds[i] == toSearch) {
				return i;
			}
		}

		return -1;
	}

	GLint programId;
	std::vector<int> uniformLoc;
	std::vector<int> attributeLoc;

	Shader(int progId = -1) : programId(progId), attributeFlags(0) {
	}

	// compile one type, from multiple source
	static GLuint compileShader(GLenum shaderType, const std::vector<std::string>& srcs) {
		SDL_assert(srcs.size() != 0);

		// generate id?
		GLuint shd = glCreateShader(shaderType);
		SDL_assert(shd != 0);

		// build several sources
		const char** sources = new const char* [srcs.size()];
		int* srclengths = new GLsizei[srcs.size()];
		for (int i = 0; i < (int)srcs.size(); i++) {
			sources[i] = srcs[i].c_str();
			srclengths[i] = srcs[i].length();
		}

		// set shader's source
		glShaderSource(shd, srcs.size(), sources, srclengths);

		// compile it
		glCompileShader(shd);

		// delete srouces
		delete[] sources;
		delete[] srclengths;

		// check compilation status
		GLint compiled;
		glGetShaderiv(shd, GL_COMPILE_STATUS, &compiled);

		static char infoLog[1024];
		GLint infoLen = 0;
		glGetShaderiv(shd, GL_INFO_LOG_LENGTH, &infoLen);
		glGetShaderInfoLog(shd, 1024, NULL, infoLog);
		SDL_Log("SHADER_LOG: %s\n", infoLog);
		SDL_assert(compiled && "Shader Compilation failed!");

		if (!compiled) {
			// failed, delete shader
			glDeleteShader(shd);

			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed compiling shader! unknow reason: %d %d", compiled, infoLen);
			return 0;
		}
		else {
#ifdef DEBUG_SHADER
			GLint srcLen = 0;
			glGetShaderiv(shd, GL_SHADER_SOURCE_LENGTH, &srcLen);
			SDL_Log("SHADER_DEBUG: length(%d)\n", srcLen);

			char* src = new char[srcLen];
			GLsizei bufSize = srcLen;
			glGetShaderSource(shd, bufSize, &srcLen, src);

			SDL_Log("SHADER_SOURCE: len(%d)\n%s", srcLen, src);

			delete[] src;
#endif // _DEBUG

		}

		return shd;
	}

	// compile one type of shader
	static GLuint compileShader(GLenum shaderType, const char* src, int srcLen) {
		if (!src || srcLen <= 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid source");
			return 0;
		}

		// create shader
#ifdef _DEBUG
		SDL_Log("Generating shader id...");
#endif

		GLuint shd = glCreateShader(shaderType);
		if (shd == 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot generate shader %d", shaderType);
			return 0;
		}

#ifdef _DEBUG
		SDL_Log("Generated shader id: %u", shd);
#endif

		// load source
#ifdef _DEBUG
		SDL_Log("loading source...");
#endif
		glShaderSource(shd, 1, &src, &srcLen);

		// compile
#ifdef _DEBUG
		SDL_Log("Compiling shader...");
#endif
		glCompileShader(shd);

		// check compile status
#ifdef _DEBUG
		SDL_Log("Checking compile status...");
#endif
		GLint compiled;
		glGetShaderiv(shd, GL_COMPILE_STATUS, &compiled);

		static char infoLog[1024];
		GLint infoLen = 0;
		glGetShaderiv(shd, GL_INFO_LOG_LENGTH, &infoLen);
		glGetShaderInfoLog(shd, 1024, NULL, infoLog);
		SDL_Log("SHADER_LOG: %s\n", infoLog);
		SDL_assert(compiled && "Shader Compilation failed!");

		if (!compiled) {
			// failed, delete shader
			glDeleteShader(shd);

			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed compiling shader! unknow reason: %d %d", compiled, infoLen);
			return 0;
		}

#ifdef _DEBUG
		SDL_Log("Final shader id: %u", shd);
#endif

		return shd;
	}

	// compile shader from a shader key
	static bool compileShader(const ShaderKey& k, int* shaderId) {
		if (!shaderId) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "(!) Compiling shader without program Id (iot)");
			return false;
		}

		// hold all sources + macros
		std::vector<std::string> sources;

		// build all macros
		// light variant
		switch (k.light_type) {
		case LightType::UNLIT:
			sources.push_back("#define HAS_UNLIT\n");
			break;
		case LightType::AMBIENT:
			sources.push_back("#define HAS_AMBIENT\n");
			break;
		case LightType::DIRECTIONAL:
			sources.push_back("#define HAS_DIRECTIONAL_LIGHT\n");
			break;
		case LightType::POINT:
			sources.push_back("#define HAS_POINT_LIGHT\n");
			break;
		case LightType::SPOT:
			sources.push_back("#define HAS_SPOT_LIGHT\n");
			break;
		case LightType::EMISSION:
			sources.push_back("#define HAS_EMISSION\n");
			break;
		}

		// opacity variants
		switch (k.opacity_type) {
		case OpacityType::ALPHA_CLIP:
			sources.push_back("#define HAS_ALPHA_CLIP\n");
			break;
		case OpacityType::ALPHA_BLEND:
			sources.push_back("#define HAS_ALPHA_BLEND\n");
			break;
		}

		// geometry variants
		switch (k.geom_type) {
		case GeomType::SKINNED:
			sources.push_back("#define HAS_SKINNED\n");
			break;
		case GeomType::BILLBOARD:
			sources.push_back("#define HAS_BILLBOARD\n");
			break;
		}

		// 1st, compile the vertex shader,
		sources.push_back("#define VERTEX_SHADER\n");
		sources.push_back(k.source->src);

		GLuint vsId = compileShader(GL_VERTEX_SHADER, sources);
		if (!vsId) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "(!) Failed Compiling Vertex Shader!");
			return false;
		}

		// 2nd pop back twice, now for the fragment shader
		sources.pop_back();
		sources.pop_back();
		sources.push_back("#define FRAGMENT_SHADER\n");
		sources.push_back(k.source->src);

		GLuint fsId = compileShader(GL_FRAGMENT_SHADER, sources);
		if (!fsId) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "(!) Failed Compiling Fragment Shader!");
			return false;
		}

		// create program
		GLuint progId = glCreateProgram();
		SDL_assert(progId != 0);

		// attach both compiled
		glAttachShader(progId, vsId);
		glAttachShader(progId, fsId);

		// link
		glLinkProgram(progId);

		// cleanup
		glDeleteShader(vsId);
		glDeleteShader(fsId);

		// check link status
		GLint linked;

		glGetProgramiv(progId, GL_LINK_STATUS, &linked);

		if (!linked) {
			GLint infoLen = 0;

			glGetProgramiv(progId, GL_INFO_LOG_LENGTH, &infoLen);

			if (infoLen > 1) {
				char* infoLog = new char[infoLen];

				glGetProgramInfoLog(progId, infoLen, NULL, infoLog);
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "(!) Program linking failed: %s", infoLog);

				delete[] infoLog;
			}

			glDeleteProgram(progId);
			return false;
		}

#ifdef _DEBUG
		// Log the final bound attributes and uniforms
		int cnt = 0;
		GLenum type = 0;
		GLint size = 0;
		GLsizei length = 0;
		char name[32];

		// TOTAL ACTIVE ATTRIBS
		glGetProgramiv(progId, GL_ACTIVE_ATTRIBUTES, &cnt);
		SDL_Log("Active_Attributes: %d\n", cnt);
		for (int i = 0; i < cnt; ++i) {
			glGetActiveAttrib(progId, i, 32, &length, &size, &type, name);
			SDL_Log("%d : type(%d), size(%d), length(%d), name(%s)\n", i, type, size, length, name);
		}

		// TOTAL ACTIVE UNIFORMS
		glGetProgramiv(progId, GL_ACTIVE_UNIFORMS, &cnt);
		SDL_Log("Active_Uniforms: %d\n", cnt);
		for (int i = 0; i < cnt; ++i) {
			glGetActiveUniform(progId, i, 32, &length, &size, &type, name);
			SDL_Log("%d : type(%d), size(%d), length(%d), name(%s)\n", i, type, size, length, name);
		}
#endif

		*shaderId = progId;
		return true;
	}

	// compile shader from source inputs
	static bool compileShader(const char* vs, const char* fs, int vsLen, int fsLen, int* shaderId) {
		if (!shaderId) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "(!) Compiling shader without program Id (iot)");
			return false;
		}

		int vsId = compileShader(GL_VERTEX_SHADER, vs, vsLen);

		if (!vsId) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "(!) Failed Compiling Vertex Shader!");
			return false;
		}

		int fsId = compileShader(GL_FRAGMENT_SHADER, fs, fsLen);

		if (!fsId) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "(!) Failed Compiling Fragment Shader!");
			return false;
		}

		// create program object nao
		GLuint progId = glCreateProgram();

		if (!progId) {
			return false;
		}

		// attach both compiled
		glAttachShader(progId, vsId);
		glAttachShader(progId, fsId);

		// link it?
		glLinkProgram(progId);

		// check link status
		GLint linked;

		glGetProgramiv(progId, GL_LINK_STATUS, &linked);

		if (!linked) {
			GLint infoLen = 0;

			glGetProgramiv(progId, GL_INFO_LOG_LENGTH, &infoLen);

			if (infoLen > 1) {
				char* infoLog = new char[infoLen];

				glGetProgramInfoLog(progId, infoLen, NULL, infoLog);
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "(!) Program linking failed: %s", infoLog);

				delete[] infoLog;
			}

			glDeleteProgram(progId);
			return false;
		}

		*shaderId = progId;

		// delete shaders
		glDeleteShader(vsId);
		glDeleteShader(fsId);

#ifdef _DEBUG
		// Log the final bound attributes and uniforms
		int cnt = 0;
		GLenum type = 0;
		GLint size = 0;
		GLsizei length = 0;
		char name[32];

		// TOTAL ACTIVE ATTRIBS
		glGetProgramiv(progId, GL_ACTIVE_ATTRIBUTES, &cnt);
		SDL_Log("Active_Attributes: %d\n", cnt);
		for (int i = 0; i < cnt; ++i) {
			glGetActiveAttrib(progId, i, 32, &length, &size, &type, name);
			SDL_Log("%d : type(%d), size(%d), length(%d), name(%s)\n", i, type, size, length, name);
		}

		// TOTAL ACTIVE UNIFORMS
		glGetProgramiv(progId, GL_ACTIVE_UNIFORMS, &cnt);
		SDL_Log("Active_Uniforms: %d\n", cnt);
		for (int i = 0; i < cnt; ++i) {
			glGetActiveUniform(progId, i, 32, &length, &size, &type, name);
			SDL_Log("%d : type(%d), size(%d), length(%d), name(%s)\n", i, type, size, length, name);
		}
#endif

		return true;
	}
public:
	int attributeFlags;

	static Shader* fromKey(const ShaderKey& k) {
		Shader* s = new Shader();
		s->loadFromKey(k);
		// set uniform locations
		s->setUniformLocations();
		// set attrib locations
		s->setAttributeLocations();
		return s;
	}

	static Shader* fromFile(const char* vsFilename, const char* fsFilename) {
		// create shader object, load from file, set its data
		Shader* s = new Shader();
		s->loadFromFile(vsFilename, fsFilename);
		// set uniform locations
		s->setUniformLocations();
		// set common attrib locations
		s->setAttributeLocations();
		return s;
	}

	static Shader* fromMemory(const char* vsSource, size_t vsSourceLen, const char* fsSource, size_t fsSourceLen) {
		Shader* s = new Shader();
		s->loadFromMemory(vsSource, vsSourceLen, fsSource, fsSourceLen);
		s->setUniformLocations();
		s->setAttributeLocations();
		return s;
	}

	virtual ~Shader() {
		if (programId)
			glDeleteProgram(programId);
	}

	// fill active uniforms
	virtual void setUniformLocations() {
		// fill with negative one
		//uniformLoc.resize(32);
		std::fill(uniformLoc.begin(), uniformLoc.end(), -1);
		// gotta get all active uniforms
		GLint cnt, size;
		GLenum type;
		GLsizei bufLen = 32, length;
		char buf[32];
		glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &cnt);

		for (GLint i = 0; i < cnt; i++) {
			glGetActiveUniform(programId, i, bufLen, &length, &size, &type, buf);
#ifdef _DEBUG
			SDL_Log("active_uniform[%d]: name(%s), length(%d), size(%d), type(%d)\n", i, buf, length, size, type);
#endif // _DEBUG
			pushUniformLocation(buf, getUniformId(buf));
		}
	}

	virtual void setAttributeLocations() {
		//attributeLoc.resize(16);
		std::fill(attributeLoc.begin(), attributeLoc.end(), -1);
		// gotta get all active uniforms
		GLint cnt, size;
		GLenum type;
		GLsizei bufLen = 32, length;
		char buf[32];
		glGetProgramiv(programId, GL_ACTIVE_ATTRIBUTES, &cnt);

		for (GLint i = 0; i < cnt; i++) {
			glGetActiveAttrib(programId, i, bufLen, &length, &size, &type, buf);
#ifdef _DEBUG
			SDL_Log("active_attribute[%d]: name(%s), length(%d), size(%d), type(%d)\n", i, buf, length, size, type);
#endif // _DEBUG
			int attributeId = getAttributeId(buf);
			pushAttributeLocation(buf, attributeId);

			if (attributeId >= 0) {
				attributeFlags |= (1 << attributeId);
			}
		}

		// set shader attribute bitflags
	}

	// get attribute location by name
	int getAttribLocation(const char* name) {
		bind();
		return glGetAttribLocation(programId, name);
	}

	// get uniform location by name
	int getUniformLocation(const char* name) {
		bind();
		return glGetUniformLocation(programId, name);
	}

	// push uniform location to specific index
	void pushUniformLocation(const char* name, size_t id) {
		// first, gotta check if i <= size, which means we need to resize
		if (uniformLoc.size() <= id) {
			//uniformLoc.resize(id + 1);
			while (uniformLoc.size() <= id) uniformLoc.push_back(-1);
		}

		// just overwrite whatever's in there
		uniformLoc[id] = getUniformLocation(name);
	}

	// push attribute location to specific index
	void pushAttributeLocation(const char* name, size_t id) {
		if (attributeLoc.size() <= id) {
			//attributeLoc.resize(id + 1);
			SDL_assert(id < 256);	// could be outrageous
			// while we haven't reach desired size, push negative values
			while (attributeLoc.size() <= id) attributeLoc.push_back(-1);
		}
		attributeLoc[id] = getAttribLocation(name);
	}

	int getUniformLocation(int arrayIdx) const {
		if (arrayIdx >= (int)uniformLoc.size()) {
			return -1;
		}
#ifdef DEBUG_SHADER
		SDL_Log("Getting Uniform: %d\n", arrayIdx);
#endif // DEBUG_SHADER

		SDL_assert(uniformLoc.size() > (size_t) arrayIdx);

		return uniformLoc[arrayIdx];
	}

	int getAttribLocation(int arrayIdx) const {
		if (arrayIdx >= (int)attributeLoc.size()) {
			return -1;
		}
#ifdef DEBUG_SHADER
		SDL_Log("Getting Attribute: %d\n", arrayIdx);
#endif // DEBUG_SHADER
		SDL_assert(attributeLoc.size() > (size_t)arrayIdx);

		return attributeLoc[arrayIdx];
	}

	void printDebug() {
#ifdef _DEBUG
		SDL_Log("VERTEX_ATTRIBUTE_FLAGS: %d -> %b", attributeFlags, attributeFlags);
		for (int i = 0; i < (int)attributeLoc.size(); i++) {
			SDL_Log("Attribute[%d]: %d\n", i, attributeLoc[i]);
		}
		for (int i = 0; i < (int)uniformLoc.size(); i++) {
			SDL_Log("Uniform[%d]: %d\n", i, uniformLoc[i]);
		}
#endif // _DEBUG

	}

	// load from key
	Shader* loadFromKey(const ShaderKey& k) {
		bool result = Shader::compileShader(k, &programId);
		return this;
	}

	// load from memory
	Shader* loadFromMemory(const char* vsSource, size_t vsLen, const char* fsSource, size_t fsLen) {
		bool result = Shader::compileShader(vsSource, fsSource, vsLen, fsLen, &programId);
		return this;
	}

	// load from file
	Shader* loadFromFile(const char* vsFilename, const char* fsFilename) {
		using namespace Helper;

		size_t vsLen;
		char* vs = readFileContent(vsFilename, &vsLen);

		if (!vs) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading vertex shader source: %s", vsFilename);
			return this;
		}
		// realloc
		vs = (char*)realloc(vs, vsLen + 1);
		vs[vsLen++] = 0;

		size_t fsLen;
		char* fs = readFileContent(fsFilename, &fsLen);

		if (!fs) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading fragment shader source: %s", fsFilename);
			delete[] vs;

			return this;
		}
		// realloc
		fs = (char*)realloc(fs, fsLen + 1);
		fs[fsLen++] = 0;

		// compile
		SDL_Log("Compiling... %s - %s", vsFilename, fsFilename);
		bool result = Shader::compileShader(vs, fs, vsLen, fsLen, &this->programId);

		if (result) {
			SDL_Log("Compiled: %s - %s", vsFilename, fsFilename);
		}

		//cleanup
		delete[] vs;
		delete[] fs;

		return this;
	}
};


// an abstract class representing shader input (can set data to shader)
class ShaderInput : public Resource {
public:
	virtual void setupData(const Shader* shd) const = 0;

	// Inherited via Resource
	virtual const char* type() override;
};


#endif