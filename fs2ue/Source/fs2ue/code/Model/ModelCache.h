#pragma once

struct VertexNorm
{
	float sx, sy, sz;
	int32 color;
	float tu, tv;
	float nx, ny, nz;
};

bool isModelCacheInProgress();
void SetModelCacheInProgress(const bool state);
