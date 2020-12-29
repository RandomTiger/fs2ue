#include "Io/TextToSpeech.h"

#if !defined(FS2_UE)
#ifndef UNITY_BUILD

#include <windows.h>
#include <atlbase.h>
#include <sapi.h>

#endif

TextToSpeech g_TextToSpeech;

bool TextToSpeech::Init()
{
#if defined(TXT2SPEECH_ENABLED)
    HRESULT hr = CoCreateInstance(
		CLSID_SpVoice, 
		NULL, 
		CLSCTX_ALL, 
		IID_ISpVoice, 
		(void **)&m_pVoiceDevice);

	m_speechInit = SUCCEEDED(hr);

	return m_speechInit;
#else
	return true;
#endif
}

void TextToSpeech::Deinit()
{
#if defined(TXT2SPEECH_ENABLED)
	if(m_speechInit == false) return;

	m_pVoiceDevice->Release();
#endif
}

bool TextToSpeech::Play(char *text)
{
#if defined(TXT2SPEECH_ENABLED)
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
#else
	return true;
#endif
}

bool TextToSpeech::Play(unsigned short *text)
{
#if defined(TXT2SPEECH_ENABLED)
	if(m_speechInit == false) return true;
	if(text == NULL) return false;

	Stop();
  	return SUCCEEDED(m_pVoiceDevice->Speak(text, SPF_ASYNC, NULL));
#else
	return true;
#endif
}

bool TextToSpeech::Pause()
{
#if defined(TXT2SPEECH_ENABLED)
	if(m_speechInit == false) return true;
	return SUCCEEDED(m_pVoiceDevice->Pause());
#else
	return true;
#endif
}

bool TextToSpeech::Resume()
{
#if defined(TXT2SPEECH_ENABLED)
	if(m_speechInit == false) return true;
	return SUCCEEDED(m_pVoiceDevice->Resume());
#else
	return true;
#endif
}

bool TextToSpeech::Stop()
{
#if defined(TXT2SPEECH_ENABLED)
	if(m_speechInit == false) return true;
    return SUCCEEDED(m_pVoiceDevice->Speak( NULL, SPF_PURGEBEFORESPEAK, NULL ));
#else
	return true;
#endif
}

bool TextToSpeech::SetVolume(int volume)
{
#if defined(TXT2SPEECH_ENABLED)
    return SUCCEEDED(m_pVoiceDevice->SetVolume(volume));
#else
	return true;
#endif
}

bool TextToSpeech::SetVoice(void *new_voice)
{
#if defined(TXT2SPEECH_ENABLED)
    return SUCCEEDED(m_pVoiceDevice->SetVoice( (ISpObjectToken *) new_voice ));
#else
	return true;
#endif
}
#endif