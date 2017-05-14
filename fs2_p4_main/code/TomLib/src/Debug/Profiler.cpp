#include "Profiler.h"

#include <assert.h>
#include <algorithm>

namespace TomLib
{
	Profiler::Profiler(const int maxSize, Profiler::Timer &timer) : mTimer(timer)
	{
		//mBlockList.resize(maxSize);

		mBlockArray = new Block[maxSize];
		mBlockListCount = 0;
		mBlockListMax = maxSize;
		mActive = false;
	}

	Profiler::~Profiler()
	{
		mActive = false;
		delete [] mBlockArray;
	}

	void Profiler::StartBlock(const int colour)
	{
		if(mActive)
		{
			return;
		}

		assert(mBlockListCount < mBlockListMax);
		Block *newBlock = &mBlockArray[mBlockListCount];
		mBlockListCount++;

		newBlock->mStart = mTimer.GetTime();
		newBlock->mColour = colour;
		newBlock->mLevel = (int) mBlockStack.size();
		
		mBlockStack.push(newBlock);
	}
	
	void Profiler::EndBlock()
	{
		if(mActive)
		{
			return;
		}

		Block *finishedBlock = mBlockStack.top();
		mBlockStack.pop();
		finishedBlock->mEnd = mTimer.GetTime();
		
		mBlockList.push_back(finishedBlock);
	}
	
	void Profiler::StartFrame()
	{
		if(mActive)
		{
			return;
		}

		mStartOfFrame = mTimer.GetTime();
			
		mBlockStack.empty();
		mBlockList.empty();
		mBlockListCount = 0;
	}
	
	void Profiler::EndFrame()
	{

	}
	
	void Profiler::Render(Profiler::Renderer &renderer)
	{
		if(mActive)
		{
			return;
		}

		const int barHeight = 10;

		renderer.RenderBar(renderer.mX + renderer.mWidth * 0 / 3, renderer.mY, renderer.mWidth / 3, barHeight, 0xff00ff00);
		renderer.RenderBar(renderer.mX + renderer.mWidth * 1 / 3, renderer.mY, renderer.mWidth / 3, barHeight, 0xffffff00);
		renderer.RenderBar(renderer.mX + renderer.mWidth * 2 / 3, renderer.mY, renderer.mWidth / 3, barHeight, 0xffff0000);
		
		const int headerY = renderer.mY + barHeight * 2;

		const int blocks = (int) mBlockList.size();
		for(int i = 0; i < blocks; i++)
		{
			Block *block = mBlockList[i];
			
			long start = block->mStart;
			long end   = block->mEnd;
			
			const long frameTime15 = 1000 / 15;
			const long widthLong = (long) renderer.mWidth;
			
			//assert(end > start);
			long drawX     = widthLong * (start - mStartOfFrame) / frameTime15;
			long drawWidth = widthLong * (end - start)           / frameTime15;
			
			const int y = headerY + block->mLevel * barHeight * 2;
			
			renderer.RenderBar((int) (renderer.mX + drawX), y, (int) drawWidth, barHeight, block->mColour);	
		}
	}

}
