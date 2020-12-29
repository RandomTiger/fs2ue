#pragma once

namespace TomLib
{
	class Profiler;
}

namespace DebugSys
{
	void Init(const int screenWidth, const int screenHeight);
	void Deinit();
	void Render();
	TomLib::Profiler *GetProfiler();
}
