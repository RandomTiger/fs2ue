#include "ComponentSystem.h"
#include <assert.h>

int MessageBase::mMessageIDCount = 0;

void ComponentSystem::AddBehaviour(ComponentBase *ptr, const int type)
{
	mBehaviourList.push_back(Behaviour(ptr, type));
}

void ComponentSystem::PostAMessage(MessageBase *lpMsg)
{
	assert(lpMsg);

	bool lbMessageProcessed = false;

	const unsigned int lListSize = (unsigned int) mBehaviourList.size();
	for(unsigned int i = 0; i < lListSize; i++)
	{
		lbMessageProcessed |= mBehaviourList[i].ProcessMessage(lpMsg);
	}

	assert(lbMessageProcessed);
	// todo, check the message has been processed
}