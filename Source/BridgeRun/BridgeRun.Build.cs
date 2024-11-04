// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BridgeRun : ModuleRules
{
    public BridgeRun(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                "BridgeRun/Public"
            }
        );

        PrivateIncludePaths.AddRange(
            new string[] {
                "BridgeRun/Private"
            }
        );

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "HeadMountedDisplay"
            }
        );
    }
}