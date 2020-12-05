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
        PublicDefinitions.Add("UNITY_BUILD");
        PublicDefinitions.Add("FS2_UE");

        //PublicIncludePaths.AddRange(new string[] { "fs2_source//code" });


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
