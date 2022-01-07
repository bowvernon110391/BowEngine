#pragma once

class Shader;
class ShaderInput;
class AbstractRenderObject;
class Mesh;
struct SubMesh;
class Pipeline;

class RenderCommand {
public:
	virtual ~RenderCommand() {}
	virtual void operator()() = 0;

	// for debugging?
	virtual void debug(char* const str) = 0;
};

// to bind a new shader by a render pass
class RC_BindShader : public RenderCommand {
	Shader* s;
	Pipeline* r;
public:
	RC_BindShader(Shader* s, Pipeline* r):s(s), r(r){}
	virtual void operator()();
	virtual void debug(char* const str);
};

// to bind a new material to a shader
class RC_BindShaderData : public RenderCommand {
	Shader* s;
	ShaderInput* sd;
	Pipeline* r;
public:
	RC_BindShaderData(Shader* s, ShaderInput* sd, Pipeline* r): s(s), sd(sd), r(r) {}
	virtual void operator()(); 
	virtual void debug(char* const str); 
};

// to setup shader data from render object
class RC_BindShaderDataFromObject : public RenderCommand {
	Shader* s;
	AbstractRenderObject* o;
	Pipeline* r;
public:
	RC_BindShaderDataFromObject(Shader* s, AbstractRenderObject* o, Pipeline *r): s(s), o(o), r(r) 
	{}
	virtual void operator()();

	// Inherited via RenderCommand
	virtual void debug(char* const str);
};

// to bind a buffer object
class RC_BindBufferObject : public RenderCommand {
	Shader* s;
	Mesh* m;
public:
	RC_BindBufferObject(Mesh* m, Shader* s) : m(m), s(s) {}
	virtual void operator()();
	virtual void debug(char* const str);
};

// to make a draw call
class RC_DrawElements : public RenderCommand {
	SubMesh* s;
	unsigned int primitive;
public:
	RC_DrawElements(SubMesh* s, unsigned int primitive) : s(s), primitive(primitive) {
	}
	virtual void operator()();
	virtual void debug(char* const str);
};
