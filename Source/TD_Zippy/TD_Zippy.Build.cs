// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TD_Zippy : ModuleRules
{
	public TD_Zippy(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
