using UnrealBuildTool;
using System.IO;

public class BridgeRun : ModuleRules
{
    public BridgeRun(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // ��� Public ���� ���� �߰�
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

        // ��� Private ���� ���� �߰�
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

        // �߰� ���Ӽ��� �ʿ��� ��츦 ���� Private Dependencies
        PrivateDependencyModuleNames.AddRange(
            new string[] {
                // �ʿ��� private ����
            }
        );
    }
}