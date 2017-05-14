#pragma once

#include <vector>

namespace TomLib
{

class StateMachine {

public:

	class Action
	{
	public: 
		virtual void Start() = 0;
		virtual bool Process(const float lTime) = 0;
		virtual int End() = 0;
	};
	
	enum StateEnum
	{
		kStart,
		kProcess,
		kEnd,
		kFinished,
	};
	
	StateMachine(const int maxActions);
	void Add(const int actionID, Action *component);
	void Start();
	void Update(const float lTime);

	int GetCurrentState() {return mCurrentState;}

	bool ActionIDListContains(const int actionID);

private:
	int mCurrentState;
	StateEnum mState;
	int mNextState;

	std::vector<int> mActionIDList;
	std::vector<Action *> mMachineList;

	int mMaxActions;
};

}