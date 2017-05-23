#include "StateMachine.h"

#include <assert.h>
#include <algorithm>

namespace TomLib
{
	StateMachine::StateMachine(const int maxActions) : mMaxActions(maxActions)
	{

	}

	bool StateMachine::ActionIDListContains(const int actionID)
	{
		std::vector<int>::iterator it = find (mActionIDList.begin(), mActionIDList.end(), actionID);
		return it != mActionIDList.end();
	}
	
	void StateMachine::Add(const int actionID, Action *component)
	{
		mActionIDList.push_back(actionID);
		mMachineList.push_back(component);
	}
	
	void StateMachine::Start()
	{
		mCurrentState = 0;
		mState = kStart;
	}
	
	void StateMachine::Update(const float lTime)
	{
		Action *stateObject = mMachineList[mCurrentState];
		
		if(mState == kStart)
		{
			stateObject->Start();
			mState = kProcess;
		}
		else if(mState == kProcess)
		{
			if(stateObject->Process(lTime))
			{
				mState = kEnd;
			}
		}
		else if(mState == kEnd)
		{
			mNextState = stateObject->End();
			mState = kFinished;
		}
		else if(mState == kFinished)
		{
			mState = kStart;
			mCurrentState = mActionIDList[mNextState];
		}

	}

}
