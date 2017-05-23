#pragma once

#ifdef UNITY_BUILD

#pragma message("UNITY BUILD: " __FILE__)

#include <D3d9.h>
#include <d3d9types.h>
#include <D3dx9math.h>

#include "UnityBuild.h"

#include "GrInternal.h"
#include "Graphics\GrD3D9.h"
#include "Graphics\GrD3D9Line.h"
#include "Graphics\GrD3D9PixelShaderCode.h"
#include "Graphics\GrD3D9Render.h"
#include "Graphics\GrD3D9Shader.h"
#include "Graphics\GrD3D9Texture.h"
#include "Graphics\GrD3D9VertexShaderCode.h"

#endif