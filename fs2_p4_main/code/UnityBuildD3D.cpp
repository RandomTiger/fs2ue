#ifdef UNITY_BUILD

#pragma message("UNITY BUILD: " __FILE__)

#include "UnityBuildD3D.h"

#include "Graphics\GrD3D.cpp"
#include "Graphics\GrD3DRender.cpp"
#include "Graphics\GrD3DTexture.cpp"

#include "Sound\acm.cpp"
#include "Sound\AudioStr.cpp"
#include "Sound\ds.cpp"
#include "Sound\ds3d.cpp"
#include "Sound\dscap.cpp"
#include "Sound\midifile.cpp"
#include "Sound\RBAudio.cpp"
#include "Sound\rtvoice.cpp"
#include "Sound\Sound.cpp"
#include "Sound\WinMIDI.cpp"
#include "Sound\winmidi_base.cpp"

#endif