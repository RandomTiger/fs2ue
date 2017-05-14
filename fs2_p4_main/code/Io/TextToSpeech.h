#pragma once

struct ISpVoice;

class TextToSpeech
{
public:
	TextToSpeech() : m_speechInit(false), m_pVoiceDevice(0) {}

	bool Init();
	void Deinit();
	bool Play(char *text);
	bool Play(unsigned short *text);
	bool Pause();
	bool Resume();
	bool Stop();

	bool SetVolume(int volume);
	bool SetVoice(void *newVoice);

private:
	ISpVoice *m_pVoiceDevice;
	bool m_speechInit;
	
	static const int MAX_SPEECH_CHAR_LEN = 10000;
	unsigned short m_conversionBuffer[MAX_SPEECH_CHAR_LEN];
};

extern TextToSpeech g_TextToSpeech; 
