// BridgeRun.Build.cs
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
               "HeadMountedDisplay",
               "UMG",                  // UI 위젯 사용을 위해 추가
               "Slate",               // UMG 의존성
               "SlateCore"            // UMG 의존성
            }
        );
    }
}