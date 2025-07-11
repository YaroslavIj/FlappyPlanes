// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FlappyPlanes : ModuleRules
{
	public FlappyPlanes(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OnlineSubsystem", "OnlineSubsystemEOS", "OnlineSubsystemUtils", "Niagara" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        //DynamicallyLoadedModuleNames.Add("OnlineSubsystemEOS");

        //PublicIncludePaths.Add("D:/EOS_SDK/SDK/Include");
        //PublicLibraryPaths.Add("D:/EOS_SDK/SDK/Lib");
        //PublicAdditionalLibraries.Add("D:/EOS_SDK/SDK/Lib/EOSSDK-Win64-Shipping.lib");

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
