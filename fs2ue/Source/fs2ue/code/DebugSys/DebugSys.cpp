
#ifndef UNITY_BUILD
#ifdef FS2_UE
#include "DebugSys.h"
#endif
#endif

#ifndef UNITY_BUILD
#include "TomLib/src/Debug/Profiler.h"
#include "io/timer.h"
#include "Graphics/2d.h"
#include "Render/3d.h"
#endif

#define RGBA_GETALPHA(rgb)      ((rgb) >> 24)
#define RGBA_GETRED(rgb)        (((rgb) >> 16) & 0xff)
#define RGBA_GETGREEN(rgb)      (((rgb) >> 8) & 0xff)
#define RGBA_GETBLUE(rgb)       ((rgb) & 0xff)

namespace DebugSys
{

class FreespaceTimer : public TomLib::Profiler::Timer
{
public:
	long GetTime()
	{
		return timer_get_milliseconds();
	}
};

class FreespaceProfileRenderer : public TomLib::Profiler::Renderer
{
public:
	FreespaceProfileRenderer(const int x, const int y, const int width, const int height) : TomLib::Profiler::Renderer(x, y, width, height)
	{}

	void RenderBar(const int x, const int y, const int w, const int h, const int colour)
	{
		gr_rect_internal(x, y, w, h, 
			RGBA_GETRED(colour),
			RGBA_GETGREEN(colour),
			RGBA_GETBLUE(colour),
			255);
	}
};

FreespaceProfileRenderer *g_pProfileRenderer = 0;
FreespaceTimer g_ProfileTimer;
const int kMaxBlocks = 50;
TomLib::Profiler g_Profiler(kMaxBlocks, g_ProfileTimer);

void Init(const int screenWidth, const int screenHeight)
{
	const int marginX = screenWidth / 10;
	const int barY = screenHeight / 10;
	g_pProfileRenderer = new FreespaceProfileRenderer(marginX, marginX, screenWidth - 2 * marginX, barY);
	
	g_Profiler.StartFrame();
	g_Profiler.StartBlock(0xffffffff);
}

void Deinit()
{
	delete g_pProfileRenderer;
}

void Render()
{
	assert(g_pProfileRenderer);
	g_Profiler.Render(*g_pProfileRenderer);
}

TomLib::Profiler *GetProfiler()
{
	return &g_Profiler;
}

} // namespace DebugSys