#pragma once

const int GRAMMARID1   = 161;                     // Arbitrary grammar id

struct ISpPhrase;

class VoiceRecognition
{
public:
	VoiceRecognition() : m_initialised(false) {}

	bool Init(int event_id, int grammar_id, int command_resource);
	void Deinit();
	void ProcessEvent();

	bool IsInit() {return m_initialised;}

	char *GetLastErrorString();
private:
	void ExecuteCommand(ISpPhrase *pPhrase);
	const static bool DEBUG_ON = false;

	bool m_initialised;

	static const int kErrorMsgBufferSize = 1024;
	char m_errorMsgBuffer[kErrorMsgBufferSize];
};

extern VoiceRecognition g_VoiceRecognition;