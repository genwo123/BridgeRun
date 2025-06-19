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

        //  Photon ��� ���� ��� �߰�
        PrivateIncludePaths.AddRange(
            new string[] {
                Path.Combine(PhotonPath, "Common-cpp", "inc"),
                Path.Combine(PhotonPath, "LoadBalancing-cpp", "inc"),
                Path.Combine(PhotonPath, "Photon-cpp", "inc")
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
                "NavigationSystem",
                "SimpleLobbySystem"
            }
        );

        // �߰� ���Ӽ��� �ʿ��� ��츦 ���� Private Dependencies
        PrivateDependencyModuleNames.AddRange(
            new string[] {
                // �ʿ��� private ����
            }
        );

        // Photon SDK ����
        SetupPhotonSDK(Target);
    }

    // Photon SDK ���� �Լ�
    private void SetupPhotonSDK(ReadOnlyTargetRules Target)
    {
        // Windows �÷��� ����
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // UE 4.27: Definitions �� PublicDefinitions
            PublicDefinitions.Add("_EG_WINDOWS_PLATFORM");

            //  vc17 release ���� ��� (Visual Studio 2022)
            PublicAdditionalLibraries.AddRange(new string[] {
                Path.Combine(PhotonPath, "Common-cpp", "lib", "Common-cpp_vc17_release_windows_md_x64.lib"),
                Path.Combine(PhotonPath, "LoadBalancing-cpp", "lib", "LoadBalancing-cpp_vc17_release_windows_md_x64.lib"),
                Path.Combine(PhotonPath, "Photon-cpp", "lib", "Photon-cpp_vc17_release_windows_md_x64.lib")
            });
        }
    }

    // Photon ���� ��� (UE 4.27 ȣȯ)
    private string PhotonPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "Photon")); }
                                          
    }
}