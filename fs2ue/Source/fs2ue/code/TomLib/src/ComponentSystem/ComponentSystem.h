#pragma once

#include <vector>
#include <string>

#define MESSAGE_CLASS() \
public:\
	int GetMessageID() { return GetStaticMessageID(); }\
	static int GetStaticMessageID() { return mMessageID; }\
private:\
	static int mMessageID;

class MessageBase
{
public:
	const static int kInvalidMsgId = -1;

	MessageBase() {}
	virtual ~MessageBase() {}
	virtual int GetMessageID() = 0;

	static int GetNewMessageID() {return mMessageIDCount++;}

protected:

	static int mMessageIDCount;
};

class ComponentBase
{
public:
	virtual ~ComponentBase() {}

	void Init();
	void Deinit();

	virtual bool ProcessMessage(MessageBase *lpMsg) = 0;
};

class ComponentSystem
{
public:
	struct Behaviour
	{
		Behaviour(ComponentBase * ptr, const int lType) : mpPtr(ptr), mType(lType) {}

		inline bool ProcessMessage(MessageBase *lpMsg) 
		{
			return mpPtr->ProcessMessage(lpMsg);
		}

		inline ComponentBase *GetBehaviour(const int lType)
		{
			return (mType == lType) ?  mpPtr : 0;
		}
		

	private:

		ComponentBase *mpPtr;
		int mType;
	};

	std::string mComponentSysName;
	std::vector<Behaviour> mBehaviourList;

	void AddBehaviour(ComponentBase *ptr, const int type);
	void PostAMessage(MessageBase *lpMsg);
};