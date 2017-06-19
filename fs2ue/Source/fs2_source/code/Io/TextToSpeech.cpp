#if !defined(FS2_UE)
#ifndef UNITY_BUILD
#include <windows.h>
#include <atlbase.h>
#include <sapi.h>

#include "Io/TextToSpeech.h"
#endif

TextToSpeech g_TextToSpeech;

bool TextToSpeech::Init()
{
    HRESULT hr = CoCreateInstance(
		CLSID_SpVoice, 
		NULL, 
		CLSCTX_ALL, 
		IID_ISpVoice, 
		(void **)&m_pVoiceDevice);

	m_speechInit = SUCCEEDED(hr);

	return m_speechInit;
}

void TextToSpeech::Deinit()
{
	if(m_speechInit == false) return;

	m_pVoiceDevice->Release();
}

bool TextToSpeech::Play(char *text)
{
	if(m_speechInit == false) return true;
	if(text == NULL) return false;

	int len = strlen(text);

	if(len > (MAX_SPEECH_CHAR_LEN - 1)) {
		len = MAX_SPEECH_CHAR_LEN - 1;
	}

	int i = 0;
	for( ; i < len; i++) {
		m_conversionBuffer[i] = (unsigned short) text[i];
	}

	m_conversionBuffer[i] = '\0';

	return Play(m_conversionBuffer);
}

bool TextToSpeech::Play(unsigned short *text)
{
	if(m_speechInit == false) return true;
	if(text == NULL) return false;

	Stop();
  	return SUCCEEDED(m_pVoiceDevice->Speak(text, SPF_ASYNC, NULL));
}

bool TextToSpeech::Pause()
{
	if(m_speechInit == false) return true;
	return SUCCEEDED(m_pVoiceDevice->Pause());
}

bool TextToSpeech::Resume()
{
	if(m_speechInit == false) return true;
	return SUCCEEDED(m_pVoiceDevice->Resume());
}

bool TextToSpeech::Stop()
{
	if(m_speechInit == false) return true;
    return SUCCEEDED(m_pVoiceDevice->Speak( NULL, SPF_PURGEBEFORESPEAK, NULL ));
}

bool TextToSpeech::SetVolume(int volume)
{
    return SUCCEEDED(m_pVoiceDevice->SetVolume(volume));
}

bool TextToSpeech::SetVoice(void *new_voice)
{
    return SUCCEEDED(m_pVoiceDevice->SetVoice( (ISpObjectToken *) new_voice ));
}
#endif