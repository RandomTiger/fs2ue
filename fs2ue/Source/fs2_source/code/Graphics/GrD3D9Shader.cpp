#pragma once

#ifndef UNITY_BUILD
#include <vector>
#include <assert.h>
#include <D3dx9shader.h>
#include "GrD3D9Shader.h"
#include "GrD3D9Render.h"
#include "GrD3D9.h"

#include "GrD3D9VertexShaderCode.h"
#include "GrD3D9PixelShaderCode.h"
#endif

std::vector<VertexShader *> gVertexShaderList;
std::vector<PixelShader *>  gPixelShaderList;

bool SetupShaderSystem()
{
	bool result = true;
	// Define the vertex elements.
	D3DVERTEXELEMENT9 VertexElements[4] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT4,    D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, 20, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	gVertexShaderList.push_back(new VertexShader("Vertex passthrough"));
	result = result && gVertexShaderList[0]->CompileFromString(PassthroughVertexShader, "main");
	result = result && gVertexShaderList[0]->CreateDeclaration(VertexElements);

	gVertexShaderList.push_back(new VertexShader("Vertex passthrough: No texture"));
	result = result && gVertexShaderList[1]->CompileFromString(PassthroughVertexShaderNoTexture, "main");
	result = result && gVertexShaderList[1]->CreateDeclaration(VertexElements);

	gPixelShaderList.push_back(new PixelShader("Pixel passthrough"));
	result = result && gPixelShaderList[0]->CompileFromString(PassthroughPixelShader, "main");

	gPixelShaderList.push_back(new PixelShader("Pixel passthrough: No texture"));
	result = result && gPixelShaderList[1]->CompileFromString(NoTexturePixelShader, "main");
	return true;
}

void CleanupShaderSystem()
{
	for(unsigned int i = 0; i < gVertexShaderList.size(); i++)
	{
		delete gVertexShaderList[i];
		gVertexShaderList[i] = 0;
	}

	gVertexShaderList.clear();

	for(unsigned int i = 0; i < gPixelShaderList.size(); i++)
	{
		delete gPixelShaderList[i];
		gPixelShaderList[i] = 0;
	}

	gVertexShaderList.clear();
}

void ResetShaderSystem()
{
}

bool SetShader(VertexShaderType vshader, PixelShaderType pshader, const int left, const int right, const int top, const int bottom)
{
	bool result = true;

	if(vshader != VertexShader_None)
	{
		// View matrix
		D3DXVECTOR3 EyePt    = D3DXVECTOR3( 0.0f, 0.0f, 100.0f );
		D3DXVECTOR3 LookatPt = D3DXVECTOR3( 0.0f, 0.0f,   0.0f );
		D3DXVECTOR3 Up       = D3DXVECTOR3( 0.0f, 1.0f,   0.0f );
		D3DXMATRIX  matView;
		D3DXMATRIX matProj;
		D3DXMatrixOrthoOffCenterRH(&matProj, left, right, bottom, top, 0.00000001f, 10000.0f);
		D3DXMatrixLookAtRH( &matView, &EyePt, &LookatPt, &Up );

		D3DXMATRIX matWVP = matView * matProj;

		result = result && gVertexShaderList[vshader]->SetShader();
		result = result && GetDevice()->SetVertexShaderConstantF( 0, (FLOAT*)&matWVP, 4 );
	}
	
	if(pshader != PixelShader_None)
	{
		result = result && gPixelShaderList[pshader]->SetShader();
	}
	return result;
}

bool Shader::CompileFromString(const char * const data, const char * const entryPoint)
{
	return Compile((void *) data, strlen(data), entryPoint);
}

VertexShader::~VertexShader()
{
	if(m_pVertexShader)
	{
		m_pVertexShader->Release();
	}
}

bool VertexShader::SetShader() 
{
	HRESULT hr = GetDevice()->SetVertexShader( m_pVertexShader );
	assert(SUCCEEDED(hr));

	assert(m_pVertexDecl);
	hr = GetDevice()->SetVertexDeclaration( m_pVertexDecl );
	assert(SUCCEEDED(hr));

	return SUCCEEDED(hr);
}

bool VertexShader::Compile(void *data, const int dataLen, const char * const entryPoint)
{
  ID3DXBuffer* pShaderCode = NULL;
  ID3DXBuffer* pErrorMsg = NULL;
//  LPD3DXCONSTANTTABLE constantTable;

	HRESULT hr = 
		D3DXCompileShader((LPCSTR) data, dataLen, NULL, NULL, entryPoint, 
		D3DXGetVertexShaderProfile(GetDevice()), 0, &pShaderCode, &pErrorMsg, NULL);

	if(FAILED(hr))
	{
		OutputDebugStringA( pErrorMsg ? (CHAR*)pErrorMsg->GetBufferPointer() : "Error compiling vertex shader." );
		return false;
	}

	hr = GetDevice()->CreateVertexShader( (DWORD*)pShaderCode->GetBufferPointer(), &m_pVertexShader );  
	assert(SUCCEEDED(hr));
	pShaderCode->Release();

	return SUCCEEDED(hr);
}

bool VertexShader::CreateDeclaration(D3DVERTEXELEMENT9 *vertexElements)
{
	// Create a vertex declaration from the element descriptions.
	HRESULT hr = GetDevice()->CreateVertexDeclaration( vertexElements, &m_pVertexDecl );
	assert(SUCCEEDED(hr));
	return SUCCEEDED(hr);
}

PixelShader::~PixelShader()
{
	if(m_pPixelShader)
	{
		m_pPixelShader->Release();
	}
}

bool PixelShader::SetShader() 
{
	HRESULT hr = GetDevice()->SetPixelShader( m_pPixelShader );
	assert(SUCCEEDED(hr));
	return SUCCEEDED(hr);
}

bool PixelShader::Compile(void *data, const int dataLen, const char * const entryPoint)
{
  ID3DXBuffer* pShaderCode = NULL;
  ID3DXBuffer* pErrorMsg = NULL;
//  LPD3DXCONSTANTTABLE constantTable;

	HRESULT hr = 
		D3DXCompileShader((LPCSTR) data, dataLen, NULL, NULL, entryPoint, 
		D3DXGetPixelShaderProfile(GetDevice()), 0, &pShaderCode, &pErrorMsg, NULL);

	if(FAILED(hr))
	{
		OutputDebugStringA( pErrorMsg ? (CHAR*)pErrorMsg->GetBufferPointer() : "Error compiling pixel shader." );
		return false;
	}

	hr = GetDevice()->CreatePixelShader( (DWORD*)pShaderCode->GetBufferPointer(), &m_pPixelShader );  
	assert(SUCCEEDED(hr));
	pShaderCode->Release();

	return SUCCEEDED(hr);
}
