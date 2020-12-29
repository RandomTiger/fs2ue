#pragma once

#include <vector>
#include <stack>


namespace TomLib
{

class Profiler {
	
public:

	class Renderer
	{
	public:
		Renderer(const int x, const int y, const int width, const int barHeight) 
			: mX(x), mY(y), mWidth(width), mBarHeight(barHeight) {}
		virtual void RenderBar(const int x, const int y, const int width, const int height, const int colour) = 0;

	private:
		int mX, mY, mWidth, mBarHeight;

		friend class Profiler;
	};
	
	class Timer
	{
	public:
		virtual long GetTime() = 0;
	};

	Profiler(const int maxSize, Profiler::Timer &timer);
	~Profiler();

	void StartBlock(const int colour);
	void EndBlock();
	
	void StartFrame();
	void EndFrame();
	
	void Render(Renderer &renderer);

	void SetActive(const bool state) {mActive = state;};

private:

	class Block
	{
	public:
		Block() {}
		
		long mStart;
		long mEnd;
		int mColour;
		int mLevel;
	};
	
	std::vector<Block *> mBlockList;
	std::stack<Block *> mBlockStack;
	Block *mBlockArray;
	int mBlockListCount;
	int mBlockListMax;
	
	long mStartOfFrame;
	bool mActive;

	Timer &mTimer;
	Profiler & operator=( const Profiler & ) {}
};

}