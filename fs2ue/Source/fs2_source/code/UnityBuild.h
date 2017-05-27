#pragma once

#ifdef UNITY_BUILD

#pragma message("UNITY BUILD: " __FILE__)

#if !defined(_WIN64)
#define PREPROC_ENABLED_FF
#define PREPROC_ENABLED_JOY
#define PREPROC_ENABLED_DS
#define PREPROC_ENABLED_DI
#endif

#if !defined(FS2_UE)
#define PREPROC_ENABLED_NET
#endif

#include <windows.h>
#if defined(_DEBUG) && defined(LEAK_DETECTOR_ENABLED)
#include "vld.h"
#endif

#include <vector>
#include <io.h>
#include <math.h>
#include <time.h>
#include <xinput.h>
#include <assert.h>
#include <raserror.h>
#include <float.h>

#include <winsock.h>
#include <wsipx.h>
#include <windowsx.h>
#include <commctrl.h>
#include <mmreg.h>
#include <msacm.h>

#include "GlobalIncs/Pragma.h"
#include "TomLib/src/Debug/Profiler.h"
#include "TomLib/src/StateMachine/StateMachine.h"
#include "TomLib/src/ComponentSystem/ComponentSystem.h"

#include "Math/vector.h"
#include "Math/matrix.h"

#include "GlobalIncs/PsTypes.h"
#include "GlobalIncs/SystemVars.h"
#include "CFile/cfile.h"
#include "Parse/parselo.h"
#include "Graphics/2d.h"
#include "UI/UI.h"
#include "Model/MODEL.h"
#include "Network/multi_obj.h"
#include "Network/multi_options.h"
#include "Physics/Physics.h"
#include "Object/Object.h"

#include "GlobalIncs/SafeArray.h"

#include "Weapon/Trails.h"
#include "Palman/PalMan.h"
#include "Weapon/Weapon.h"
#include "Ship/ai.h"
#include "Ship/Ship.h"
#include "Sound/Sound.h"
#include "Mission/MissionParse.h"
#include "Stats/Scoring.h"
#include "Hud/HUDtarget.h"
#include "ControlConfig/ControlsConfig.h"
#include "Io/KeyControl.h"
#include "Cmdline/cmdline.h"
#include "Playerman/Player.h"
#include "Graphics/tmapper.h"
#include "Graphics/Font.h"

#include "Network/PsNet.h"
#include "Network/Psnet2.h"

#include "Io/sw_error.hpp"
#include "Io/sw_guid.hpp"

#include "Anim/AnimPlay.h"
#include "Anim/PackUnpack.h"
#include "Fireball/Fireballs.h"
#include "Asteroid/Asteroid.h"
#include "Behaviours/ScreenEffects.h"
#include "Bmpman/BmpMan.h"
#include "CFile/CfileArchive.h"
#include "CFile/CfileSystem.h"
#include "CMeasure/CMeasure.h"
#include "Cutscene/Cutscenes.h"
#include "Debris/Debris.h"
#include "DebugSys/DebugSys.h"
#include "Demo/Demo.h"
#include "DirectX/vasync.h"
#include "DirectX/vDinput.h"
#include "DirectX/vdplay.h"
#include "DirectX/vdplobby.h"
#include "DirectX/vdsetup.h"
#include "ExceptionHandler/ExceptionHandler.h"
#include "Game/Game.h"
#include "GameHelp/ContextHelp.h"
#include "GameHelp/GameplayHelp.h"
#include "GameSequence/GameSequence.h"
#include "Gamesnd/EventMusic.h"
#include "Gamesnd/GameSnd.h"
#include "GlobalIncs/AlphaColors.h"
#include "GlobalIncs/crypt.h"
#include "GlobalIncs/LinkList.h"
#include "GlobalIncs/version.h"
#include "Graphics/Bitblt.h"
#include "Graphics/Circle.h"
#include "Graphics/Colors.h"
#include "Graphics/Gradient.h"
#include "Graphics/GrInternal.h"
#include "Graphics/Line.h"
#include "Hud/HUD.h"
#include "Hud/HudArtillery.h"
#include "Hud/HUDbrackets.h"
#include "Hud/HUDgauges.h"
#include "Hud/HUDconfig.h"
#include "Hud/HUDescort.h"
#include "Hud/HUDets.h"
#include "Hud/HUDlock.h"
#include "Hud/HUDmessage.h"
#include "Hud/HUDObserver.h"
#include "Hud/HUDresource.h"
#include "Hud/HUDreticle.h"
#include "Hud/HUDshield.h"
#include "Hud/HUDsquadmsg.h"
#include "Hud/HUDtargetbox.h"
#include "Hud/HUDWingmanStatus.h"
#include "Inetfile/CFtp.h"
#include "Inetfile/Chttpget.h"
#include "Inetfile/inetgetfile.h"
#include "Io/ForceFeedback.h"
#include "Io/grammar.h"
#include "Io/InputController.h"
#include "Io/Joy.h"
#include "Io/Joy_ff.h"
#include "Io/Key.h"
#include "Io/Mouse.h"
#include "Io/MouseController.h"
#include "Io/PadController.h"
#include "Io/sphelper.h"
#include "Io/sw_force.h"
#include "Io/TextToSpeech.h"
#include "Io/Timer.h"
#include "Io/VoiceRecognition.h"
#include "JumpNode/JumpNode.h"
#include "Lighting/Lighting.h"
#include "Localization/fhash.h"
#include "Localization/localize.h"
#include "Math/fix.h"
#include "Math/Floating.h"
#include "Math/Fvi.h"
#include "Math/spline.h"
#include "Math/StaticRand.h"
#include "Math/VecMat.h"
#include "MenuUI/Barracks.h"
#include "MenuUI/Credits.h"
#include "MenuUI/fishtank.h"
#include "MenuUI/MainHallMenu.h"
#include "MenuUI/MainHallTemp.h"
#include "MenuUI/OptionsMenu.h"
#include "MenuUI/OptionsMenuMulti.h"
#include "MenuUI/PlayerMenu.h"
#include "MenuUI/ReadyRoom.h"
#include "MenuUI/SnazzyUI.h"
#include "MenuUI/TechMenu.h"
#include "MenuUI/TrainingMenu.h"
#include "Mission/MissionBriefCommon.h"
#include "Mission/MissionCampaign.h"
#include "Mission/MissionGoals.h"
#include "Mission/MissionGrid.h"
#include "Mission/MissionHotKey.h"
#include "Mission/MissionLoad.h"
#include "Mission/MissionLog.h"
#include "Mission/MissionMessage.h"
#include "Mission/MissionTraining.h"
#include "MissionUI/Chatbox.h"
#include "MissionUI/MissionBrief.h"
#include "MissionUI/MissionCmdBrief.h"
#include "MissionUI/MissionDebrief.h"
#include "MissionUI/MissionLoopBrief.h"
#include "MissionUI/MissionPause.h"
#include "MissionUI/MissionRecommend.h"
#include "MissionUI/MissionScreenCommon.h"
#include "MissionUI/MissionShipChoice.h"
#include "MissionUI/MissionStats.h"
#include "MissionUI/MissionWeaponChoice.h"
#include "MissionUI/RedAlert.h"
#include "Model/ModelsInc.h"
#include "Nebula/Neb.h"
#include "Nebula/NebLightning.h"
#include "Network/multi_ping.h"
#include "Network/Multi.h"
#include "Network/multilag.h"
#include "Network/multimsgs.h"
#include "Network/MultiTeamSelect.h"
#include "Network/MultiUI.h"
#include "Network/MultiUtil.h"
#include "Network/multi_campaign.h"
#include "Network/multi_data.h"
#include "Network/multi_dogfight.h"
#include "Network/multi_endgame.h"
#include "Network/multi_ingame.h"
#include "Network/multi_kick.h"
#include "Network/multi_log.h"
#include "Network/multi_observer.h"
#include "Network/multi_oo.h"
#include "Network/multi_pause.h"
#include "Network/multi_pinfo.h"
#include "Network/multi_pmsg.h"
#include "Network/multi_rate.h"
#include "Network/multi_respawn.h"
#include "Network/multi_team.h"
#include "Network/multi_update.h"
#include "Network/multi_voice.h"
#include "Network/multi_xfer.h"
#include "Network/stand_gui.h"
#include "Object/ObjCollide.h"
#include "Object/ObjectSnd.h"
#include "Observer/Observer.h"
#include "OsApi/MONOPUB.h"
#include "OsApi/OsApi.h"
#include "OsApi/OsRegistry.h"
#include "OsApi/OutWnd.h"
#include "Parse/Encrypt.h"
#include "Parse/SEXP.h"
#include "Particle/Particle.h"
#include "PcxUtils/pcxutils.h"
#include "Playerman/ManagePilot.h"
#include "Popup/Popup.h"
#include "Popup/PopupDead.h"
#include "Radar/Radar.h"
#include "Render/3D.h"
#include "Render/3dInternal.h"
#include "Scramble/scramble.h"
#include "Ship/Afterburner.h"
#include "Ship/AiBig.h"
#include "Ship/AiGoals.h"
#include "Ship/AiLocal.h"
#include "Ship/AWACS.h"
#include "Ship/ShipContrails.h"
#include "Ship/ShipFX.h"
#include "Ship/ShipHit.h"
#include "Ship/SubsysDamage.h"

#include "Sound/AudioStr.h"
#include "Sound/ds.h"
#include "Sound/ds3d.h"
#include "Sound/rtvoice.h"

#include "Starfield/Nebula.h"
#include "Starfield/StarField.h"
#include "Starfield/Supernova.h"
#include "Stats/Medals.h"
#include "Stats/Stats.h"
#include "TgaUtils/TgaUtils.h"
#include "Threading/thread.h"
#include "UI/UiDefs.h"
#include "VCodec/CODEC1.h"
#include "Weapon/Beam.h"
#include "Weapon/Corkscrew.h"
#include "Weapon/Emp.h"
#include "Weapon/Flak.h"
#include "Weapon/MuzzleFlash.h"
#include "Weapon/Shockwave.h"
#include "Weapon/Swarm.h"

#endif
