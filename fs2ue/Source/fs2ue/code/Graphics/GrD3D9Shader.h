#pragma once

#ifndef FS2_UE
#ifndef UNITY_BUILD
#include <string>
#include <windows.h> // ifndef FS2_UE
#include <d3d9types.h>
#endif

struct IDirect3DPixelShader9;
struct IDirect3DVertexDeclaration9;
struct IDirect3DVertexShader9;

class Shader
{
public:
	Shader(std::string name)
	{
		m_name = name;
	}

	virtual ~Shader() {}

	virtual bool SetShader() = 0;
	virtual bool Compile(void *data, const int dataLen, const char * const entryPoint) = 0;

	bool CompileFromString(const char * const data, const char * const entryPoint);

private:
	std::string m_name;

	Shader(Shader &copy);
};

class VertexShader : public Shader
{ 
public:
	VertexShader(std::string name) : Shader(name), m_pVertexShader(0), m_pVertexDecl(0) {}
	~VertexShader();

	bool SetShader();
	bool Compile(void *data, const int dataLen, const char * const entryPoint);
	bool CreateDeclaration(D3DVERTEXELEMENT9 *vertexElements);

private:
	IDirect3DVertexDeclaration9* m_pVertexDecl;
	IDirect3DVertexShader9 *m_pVertexShader;
};

class PixelShader : public Shader
{ 
public:
	PixelShader(std::string name) : Shader(name), m_pPixelShader(0) {}
	~PixelShader();

	bool SetShader();
	bool Compile(void *data, const int dataLen, const char * const entryPoint);

private:
	IDirect3DPixelShader9 *m_pPixelShader;
};

bool SetupShaderSystem();
void CleanupShaderSystem();
void ResetShaderSystem();

enum VertexShaderType
{
	VertexShader_None = -1,
	VertexShader_Passthrough,
	VertexShader_PassthroughNoTexture
};

enum PixelShaderType
{
	PixelShader_None = -1,
	PixelShader_Passthrough,
	PixelShader_PassthroughNoTexture
};

bool SetShader(VertexShaderType vshader, PixelShaderType pshader, const int left, const int right, const int top, const int bottom);
#endif