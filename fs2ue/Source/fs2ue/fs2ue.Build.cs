/*
* Copyright (C) Thomas Liam Whittaker 2017.  All rights reserved.
*
* All modified source code herein is the property of Thomas Liam Whittaker. You may not sell
* or otherwise commercially exploit the source or things you created based on the
* source. Original source code is owned by Volition, Inc and covered by their copywrite.
*
*/

using UnrealBuildTool;

public class fs2ue : ModuleRules
{
	public fs2ue(ReadOnlyTargetRules Target) : base(Target)
	{
        bLegacyPublicIncludePaths = false;
        //PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        //PrivatePCHHeaderFile = "code/GlobalIncs/PSTypes.h";

        // PublicDefinitions.Add("UNITY_BUILD");
        PublicDefinitions.Add("FS2_UE");

        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/TomLib/src" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Anim" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Asteroid" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Behaviours" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Bmpman" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/CFile" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Cmdline" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/CMeasure" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/ControlConfig" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Cutscene" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Debris" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/DebugConsole" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/DebugSys" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Demo" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/DirectX" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/ExceptionHandler" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Fireball" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/FREESPACE2" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Game" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/GameHelp" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/GameSequence" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Gamesnd" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Glide" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/GlobalIncs" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Graphics" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Hud" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Inetfile" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Io" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/JumpNode" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Lighting" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Localization" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Math" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/MenuUI" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Mission" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/MissionUI" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Model" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Model" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Nebula" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Network" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Object" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Observer" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/OsApi" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Palman" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Parse" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Particle" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/PcxUtils" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Physics" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Playerman" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Popup" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Radar" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Render" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Scramble" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Ship" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Sound" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Radar" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Starfield" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Stats" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/TgaUtils" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Threading" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/UI" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/VCodec" });
        PublicIncludePaths.AddRange(new string[] { "fs2ue/code/Weapon" });


        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "PhysX", "APEX" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        // https://github.com/Koderz/RuntimeMeshComponent/wiki/Making-the-RMC-available-to-your-project-in-CPP
        PublicDependencyModuleNames.AddRange(new string[] { "RenderCore", "RHI", "RuntimeMeshComponent" });

        // For 2.24 update
        PublicDependencyModuleNames.AddRange(new string[] { "PhysX", "APEX" });

        bLegacyPublicIncludePaths = true;
        bEnableUndefinedIdentifierWarnings = false;

        //        PublicAdditionalLibraries.Add("atls.lib");

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
