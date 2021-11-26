#pragma once

#include "Matrix3.h"
#include "RGBA.h"
#include "MeshLoader.h"

#define InitializeProgram(name) ProgramInitializer Initializer = ProgramInitializer(this, name)
#define BindAttribute()
#define AttachShader(name) ProgramAttacher name = ProgramAttacher(this, #name)
#define LinkProgram() ProgramLinker Linker = ProgramLinker(this)

class UniformBase;
class Shader;

class ShaderProgram
{
public:
	std::shared_ptr<MeshLoader> Meshes = Engine::Create<MeshLoader>();

	ShaderProgram() { Meshes->SetProgram(this); }
	~ShaderProgram();

	GLint RegisterUniform(UniformBase* uniform);
	void Use();
	GLuint GetProgramID() const;
	const std::string& GetName() const;
	void InitAttributes() const;

	static ShaderProgram* GetCurrentProgram();

	friend class ProgramInitializer;
	friend class ProgramAttacher;
	friend class ProgramLinker;

	struct Primitives
	{
		const Mesh* Square = nullptr;
		const Mesh* Cube = nullptr;
		const Mesh* WireCube = nullptr;
		const Mesh* BoundingVolume = nullptr;
		const Mesh* HalfBoundingVolume = nullptr;
		const Mesh* Cone = nullptr;
		const Mesh* Arrow = nullptr;
		const Mesh* Sphere = nullptr;
		const Mesh* Ring = nullptr;

		Primitives() {}
		Primitives(const Mesh* square, const Mesh* cube, const Mesh* wireCube, const Mesh* boundingVolume, const Mesh* halfBoundingVolume, const Mesh* cone, const Mesh* arrow, const Mesh* sphere, const Mesh* ring) : Square(square), Cube(cube), WireCube(wireCube), BoundingVolume(boundingVolume), HalfBoundingVolume(halfBoundingVolume), Cone(cone), Arrow(arrow), Sphere(sphere), Ring(ring) {}
	};

	Primitives CoreMeshes;

private:
	typedef std::map<GLint, UniformBase*> UniformMap;
	typedef std::map<std::string, GLint> IDMap;

	UniformMap Uniforms;
	IDMap UniformIDs;

	GLuint ProgramID = 0;

	std::string Name;

	static ShaderProgram* CurrentProgram;

	void InitAttribute(const std::string& attribute, int offset, int size) const;
	void Initialize();
	void AttachProgramShader(Shader* shader);
	void Link();
	void CheckErrors(GLuint data, int action) const;
};

class ProgramInitializer
{
public:
	ProgramInitializer(ShaderProgram* program, const std::string& name);
};

class ProgramAttacher
{
public:
	ProgramAttacher(ShaderProgram* program, const std::string& shaderName);
};

class ProgramLinker
{
public:
	ProgramLinker(ShaderProgram* program);
};
