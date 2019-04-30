#pragma once
#include<glad/glad.h>
#include<map>
#include<string>
using namespace std;
class GLSLShader {
public:
	GLSLShader(void);
	~GLSLShader(void);
	void LoadFromString(GLenum whichshader, const string& source);
	void LoadFromFile(GLenum whichshader, const string& filename);
	void CreateAndLinkProgram();
	void Use();
	void UnUse();
	void AddAttribute(const string& attribute);
	void AddUniform(const string& uniform);

	GLuint operator[](const string& attibute);
	GLuint operator()(const string& uniform);
	void DeleteShaderProgram();

private:
	enum SHADERTYPE
	{
		VERTEX_SHADER,
		FRAGMENT_SHADER,
		GEOMETRY_SHADER
	};

	GLuint _program;
	int _totalShaders;
	GLuint _shaders[3];
	map<string, GLuint> _attributesList;
	map<string, GLuint> _uniformLocationList;
};