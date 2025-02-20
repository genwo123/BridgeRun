using UnrealBuildTool;
using System.IO;

public class BridgeRun : ModuleRules
{
    public BridgeRun(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // 모든 Public 하위 폴더 추가
        PublicIncludePaths.AddRange(
            new string[] {
                "BridgeRun/Public",
                "BridgeRun/Public/Characters",
                "BridgeRun/Public/Core",
                "BridgeRun/Public/Item",
                "BridgeRun/Public/Modes",
                "BridgeRun/Public/Zones"
            }
        );

        // 모든 Private 하위 폴더 추가
        PrivateIncludePaths.AddRange(
            new string[] {
                "BridgeRun/Private",
                "BridgeRun/Private/Characters",
                "BridgeRun/Private/Core",
                "BridgeRun/Private/Item",
                "BridgeRun/Private/Modes",
                "BridgeRun/Private/Zones"
            }
        );

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "HeadMountedDisplay",
                "UMG",
                "Slate",
                "SlateCore",
                "NavigationSystem"
            }
        );

        // 추가 종속성이 필요한 경우를 위한 Private Dependencies
        PrivateDependencyModuleNames.AddRange(
            new string[] {
                // 필요한 private 모듈들
            }
        );
    }
}