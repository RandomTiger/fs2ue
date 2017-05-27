#pragma once

#ifdef UNITY_BUILD

#if !defined(FS2_UE)
#define USE_D3DX_MATH
#endif

#pragma message("UNITY BUILD: " __FILE__)

#include <D3d9.h>
#include <d3d9types.h>
#if defined(USE_D3DX_MATH)
#include <D3dx9math.h>
#endif

#include "UnityBuild.h"

#include "Graphics/GrInternal.h"
#include "Graphics/GrD3D9.h"
#include "Graphics/GrD3D9Line.h"
#include "Graphics/GrD3D9PixelShaderCode.h"
#include "Graphics/GrD3D9Render.h"
#include "Graphics/GrD3D9Shader.h"
#include "Graphics/GrD3D9Texture.h"
#include "Graphics/GrD3D9VertexShaderCode.h"

#endif