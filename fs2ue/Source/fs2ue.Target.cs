/*
* Copyright (C) Thomas Liam Whittaker 2017.  All rights reserved.
*
* All modified source code herein is the property of Thomas Liam Whittaker. You may not sell
* or otherwise commercially exploit the source or things you created based on the
* source. Original source code is owned by Volition, Inc and covered by their copywrite.
*
*/

using UnrealBuildTool;
using System.Collections.Generic;

public class fs2ueTarget : TargetRules
{
	public fs2ueTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

        ExtraModuleNames.Add("fs2ue");
    }
 
}
