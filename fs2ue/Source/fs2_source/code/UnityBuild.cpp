#ifdef UNITY_BUILD

#pragma message("UNITY BUILD: " __FILE__)
#include "UnityBuild.h"

//Get rid of ths
#include "Freespace.h"

#include "Anim/AnimPlay.cpp"
#include "Anim/PackUnpack.cpp"
#include "Asteroid/Asteroid.cpp"
#include "Behaviours/ScreenEffects.cpp"
#include "Bmpman/BmpMan.cpp"
#include "CFile/cfile.cpp"
#include "CFile/CfileArchive.cpp"
#include "CFile/CfileList.cpp"
#include "CFile/CfileSystem.cpp"
#include "Cfilearchiver/CfileArchiver.cpp"
#include "Cmdline/cmdline.cpp"
#include "CMeasure/CMeasure.cpp"
#include "ControlConfig/ControlsConfig.cpp"
#include "ControlConfig/ControlsConfigCommon.cpp"
#include "Cutscene/Cutscenes.cpp"
#include "Debris/Debris.cpp"
#include "DebugConsole/Console.cpp"
#include "DebugSys/DebugSys.cpp"
#include "Demo/Demo.cpp"
#include "ExceptionHandler/ExceptionHandler.cpp"
#include "Fireball/FireBalls.cpp"
#include "Fireball/WarpInEffect.cpp"
#include "Game/Game.cpp"
#include "GameHelp/ContextHelp.cpp"
#include "GameHelp/GameplayHelp.cpp"
#include "GameSequence/GameSequence.cpp"
#include "Gamesnd/EventMusic.cpp"
#include "Gamesnd/GameSnd.cpp"
#include "GlobalIncs/AlphaColors.cpp"
#include "GlobalIncs/crypt.cpp"
#include "GlobalIncs/SystemVars.cpp"
#include "GlobalIncs/version.cpp"
#include "GlobalIncs/WinDebug.cpp"
#include "Graphics/2d.cpp"
#include "Graphics/aaline.cpp"
#include "Graphics/Colors.cpp"
#include "Graphics/Font.cpp"
#include "Graphics/GrDummy.cpp"
#include "Hud/HUD.cpp"
#include "Hud/HudArtillery.cpp"
#include "Hud/HUDbrackets.cpp"
#include "Hud/HUDconfig.cpp"
#include "Hud/HUDescort.cpp"
#include "Hud/HUDets.cpp"
#include "Hud/HUDlock.cpp"
#include "Hud/HUDmessage.cpp"
#include "Hud/HUDObserver.cpp"
#include "Hud/HUDresource.cpp"
#include "Hud/HUDreticle.cpp"
#include "Hud/HUDshield.cpp"
#include "Hud/HUDsquadmsg.cpp"
#include "Hud/HUDtarget.cpp"
#include "Hud/HUDtargetbox.cpp"
#include "Hud/HUDWingmanStatus.cpp"
#include "Inetfile/CFtp.cpp"
#include "Inetfile/Chttpget.cpp"
#include "Inetfile/inetgetfile.cpp"
#include "Io/ForceFeedback.cpp"
#include "Io/Joy.cpp"
#include "Io/Joy_ff.cpp"
#include "Io/Key.cpp"
#include "Io/KeyControl.cpp"
#include "Io/Mouse.cpp"
#include "Io/MouseController.cpp"
#include "Io/PadController.cpp"
#include "Io/swff_lib.cpp"
#include "Io/TextToSpeech.cpp"
#include "Io/Timer.cpp"
#include "Io/VoiceRecognition.cpp"
#include "JumpNode/JumpNode.cpp"
#include "Lighting/Lighting.cpp"
#include "Localization/fhash.cpp"
#include "Localization/localize.cpp"
#include "Math/Fix.cpp"
#include "Math/Floating.cpp"
#include "Math/Fvi.cpp"
#include "Math/spline.cpp"
#include "Math/StaticRand.cpp"
#include "Math/VecMat.cpp"
#include "Math/vector.cpp"
#include "MenuUI/Barracks.cpp"
#include "MenuUI/Credits.cpp"
#include "MenuUI/fishtank.cpp"
#include "MenuUI/MainHallMenu.cpp"
#include "MenuUI/MainHallTemp.cpp"
#include "MenuUI/OptionsMenu.cpp"
#include "MenuUI/OptionsMenuMulti.cpp"
#include "MenuUI/PlayerMenu.cpp"
#include "MenuUI/ReadyRoom.cpp"
#include "MenuUI/SnazzyUI.cpp"
#include "MenuUI/TechMenu.cpp"
#include "MenuUI/TrainingMenu.cpp"
#include "Mission/MissionBriefCommon.cpp"
#include "Mission/MissionCampaign.cpp"
#include "Mission/MissionGoals.cpp"
#include "Mission/MissionGrid.cpp"
#include "Mission/MissionHotKey.cpp"
#include "Mission/MissionLoad.cpp"
#include "Mission/MissionLog.cpp"
#include "Mission/MissionMessage.cpp"
#include "Mission/MissionParse.cpp"
#include "Mission/MissionTraining.cpp"
#include "MissionUI/Chatbox.cpp"
#include "MissionUI/MissionBrief.cpp"
#include "MissionUI/MissionCmdBrief.cpp"
#include "MissionUI/MissionDebrief.cpp"
#include "MissionUI/MissionLoopBrief.cpp"
#include "MissionUI/MissionPause.cpp"
#include "MissionUI/MissionRecommend.cpp"
#include "MissionUI/MissionScreenCommon.cpp"
#include "MissionUI/MissionShipChoice.cpp"
#include "MissionUI/MissionStats.cpp"
#include "MissionUI/MissionWeaponChoice.cpp"
#include "MissionUI/RedAlert.cpp"
#include "Model/ModelCollide.cpp"
#include "Model/ModelInterp.cpp"
#include "Model/ModelOctant.cpp"
#include "Model/ModelRead.cpp"
#include "Nebula/Neb.cpp"
#include "Nebula/NebLightning.cpp"
#include "Network/Multi.cpp"
#include "Network/multilag.cpp"
#include "Network/MultiMsgs.cpp"
#include "Network/MultiTeamSelect.cpp"
#include "Network/MultiUI.cpp"
#include "Network/MultiUtil.cpp"
#include "Network/multi_campaign.cpp"
#include "Network/multi_data.cpp"
#include "Network/multi_dogfight.cpp"
#include "Network/multi_endgame.cpp"
#include "Network/multi_ingame.cpp"
#include "Network/multi_kick.cpp"
#include "Network/multi_log.cpp"
#include "Network/multi_obj.cpp"
#include "Network/multi_observer.cpp"
#include "Network/multi_oo.cpp"
#include "Network/multi_options.cpp"
#include "Network/multi_pause.cpp"
#include "Network/multi_pinfo.cpp"
#include "Network/multi_ping.cpp"
#include "Network/multi_pmsg.cpp"
#include "Network/multi_rate.cpp"
#include "Network/multi_respawn.cpp"
#include "Network/multi_team.cpp"
#include "Network/multi_update.cpp"
#include "Network/multi_voice.cpp"
#include "Network/multi_xfer.cpp"
#include "Network/PsNet.cpp"
#include "Network/Psnet2.cpp"
#include "Network/stand_gui.cpp"
#include "Object/CollideDebrisShip.cpp"
#include "Object/CollideDebrisWeapon.cpp"
#include "Object/CollideShipShip.cpp"
#include "Object/CollideShipWeapon.cpp"
#include "Object/CollideWeaponWeapon.cpp"
#include "Object/ObjCollide.cpp"
#include "Object/Object.cpp"
#include "Object/ObjectSnd.cpp"
#include "Object/ObjectSort.cpp"
#include "Observer/Observer.cpp"
#include "OsApi/OsApi.cpp"
#include "OsApi/OsRegistry.cpp"
#include "OsApi/OutWnd.cpp"
#include "Palman/PalMan.cpp"
#include "Parse/Encrypt.cpp"
#include "Parse/PARSELO.cpp"
#include "Parse/SEXP.cpp"
#include "Particle/Particle.cpp"
#include "PcxUtils/pcxutils.cpp"
#include "Physics/Physics.cpp"
#include "Playerman/Player.cpp"
#include "Playerman/ManagePilot.cpp"
#include "Playerman/PlayerControl.cpp"
#include "Popup/Popup.cpp"
#include "Popup/PopupDead.cpp"
#include "Radar/Radar.cpp"
#include "Render/3dClipper.cpp"
#include "Render/3ddraw.cpp"
#include "Render/3dLaser.cpp"
#include "Render/3dMath.cpp"
#include "Render/3dSetup.cpp"
#include "Scramble/scramble.cpp"
#include "Ship/Afterburner.cpp"
#include "Ship/ai.cpp"
#include "Ship/AiBig.cpp"
#include "Ship/AiCode.cpp"
#include "Ship/AiGoals.cpp"
#include "Ship/AWACS.cpp"
#include "Ship/Shield.cpp"
#include "Ship/Ship.cpp"
#include "Ship/ShipContrails.cpp"
#include "Ship/ShipFX.cpp"
#include "Ship/ShipHit.cpp"
#include "Starfield/Nebula.cpp"
#include "Starfield/StarField.cpp"
#include "Starfield/Supernova.cpp"
#include "Stats/Medals.cpp"
#include "Stats/Scoring.cpp"
#include "Stats/Stats.cpp"
#include "TgaUtils/TgaUtils.cpp"
#include "Threading/thread.cpp"
#include "UI/BUTTON.cpp"
#include "UI/CHECKBOX.cpp"
#include "UI/GADGET.cpp"
#include "UI/icon.cpp"
#include "UI/INPUTBOX.cpp"
#include "UI/KEYTRAP.cpp"
#include "UI/LISTBOX.cpp"
#include "UI/RADIO.cpp"
#include "UI/SCROLL.cpp"
#include "UI/slider.cpp"
#include "UI/SLIDER2.cpp"
#include "UI/UIDRAW.cpp"
#include "UI/UIMOUSE.cpp"
#include "UI/WINDOW.cpp"
#include "VCodec/CODEC1.cpp"
#include "Weapon/Beam.cpp"
#include "Weapon/Corkscrew.cpp"
#include "Weapon/Emp.cpp"
#include "Weapon/Flak.cpp"
#include "Weapon/MuzzleFlash.cpp"
#include "Weapon/Shockwave.cpp"
#include "Weapon/Swarm.cpp"
#include "Weapon/Trails.cpp"
#include "Weapon/Weapons.cpp"

#pragma message("UNITY BUILD: End")


#endif