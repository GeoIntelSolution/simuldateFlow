#include"GLSLShader.h"
#include<fstream>
#include<iostream>
GLSLShader::GLSLShader(void)
{
	_totalShaders = 0;
	_shaders[VERTEX_SHADER] = 0;
	_shaders[FRAGMENT_SHADER] = 0;
	_shaders[GEOMETRY_SHADER] = 0;
	_attributesList.clear();
	_uniformLocationList.clear();
}

GLSLShader::~GLSLShader(void) {
	_attributesList.clear();
	_uniformLocationList.clear();
}

void  GLSLShader::LoadFromString(GLenum whichShader, const string& source) {
	GLuint shader = glCreateShader(whichShader);
	const char* tmp = source.c_str();
	glShaderSource(shader, 1, &tmp, NULL);

	GLint status;
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* infoLog = new GLchar[infoLogLength];
		glGetShaderInfoLog(shader, infoLogLength, NULL,infoLog);
		cerr<< "Compile log:" << infoLog << endl;
		delete[] infoLog;
	}

	_shaders[_totalShaders++] = shader;
}

void GLSLShader::CreateAndLinkProgram()
{
	_program = glCreateProgram();
	if (_shaders[VERTEX_SHADER] != 0) {
		glAttachShader(_program, _shaders[VERTEX_SHADER]);
	}

	if (_shaders[FRAGMENT_SHADER] != 0) {
		glAttachShader(_program, _shaders[FRAGMENT_SHADER]);
	}

	if (_shaders[GEOMETRY_SHADER] != 0) {
		glAttachShader(_program, _shaders[GEOMETRY_SHADER]);
	}

	GLint status;
	glLinkProgram(_program);
	glGetProgramiv(_program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* infoLog = new GLchar[infoLogLength];
		glGetProgramInfoLog(_program, infoLogLength, NULL, infoLog);
		cerr << "Link log: " << infoLog << endl;
		delete[] infoLog;
	}
	glDeleteShader(_shaders[VERTEX_SHADER]);
	glDeleteShader(_shaders[FRAGMENT_SHADER]);
	glDeleteShader(_shaders[GEOMETRY_SHADER]);
}

void GLSLShader::Use() {
	glUseProgram(_program);
}

void GLSLShader::UnUse() {
	glUseProgram(0);
}

void GLSLShader::AddAttribute(const string& attribute) {
	_attributesList[attribute] = glGetAttribLocation(_program, attribute.c_str());
}

void GLSLShader::AddUniform(const string& uniform) {
	_uniformLocationList[uniform] = glGetUniformLocation(_program, uniform.c_str());
}



GLuint GLSLShader::operator[](const string& attribute) {
	return _attributesList[attribute];
}

GLuint GLSLShader::operator()(const string &uniform) {
	return _uniformLocationList[uniform];
}

void GLSLShader::DeleteShaderProgram() {
	glDeleteProgram(_program);
}

void GLSLShader::LoadFromFile(GLenum type, const string& filename) {
	ifstream fp;
	fp.open(filename.c_str(), ios_base::in);
	if (fp) {
		string line, buffer;
		while (getline(fp, line)) {
			buffer.append(line);
			buffer.append("\r\n");
		}

		LoadFromString(type, buffer);
	}
}
