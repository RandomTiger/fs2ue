#pragma once

#ifdef UNITY_BUILD

#pragma message("UNITY BUILD: " __FILE__)

#include "DirectX/vD3d.h"
#include "DirectX/vDsound.h"

#include "UnityBuild.h"
#if !defined(FS2_UE)
#include "Graphics/GrD3D.h"
#include "Graphics/GrD3DInternal.h"
#endif

#include "Sound/ds3d.h"
#include "Sound/acm.h"
#include "Sound/AudioStr.h"
#include "Sound/channel.h"
#include "Sound/ds.h"
#include "Sound/dscap.h"
#include "Sound/midifile.h"
#include "Sound/midiseq.h"
#include "Sound/RBAudio.h"
#include "Sound/rtvoice.h"
#include "Sound/WinMIDI.h"

#endif