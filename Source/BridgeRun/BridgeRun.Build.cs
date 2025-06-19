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

        //  Photon 헤더 파일 경로 추가
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

        // 추가 종속성이 필요한 경우를 위한 Private Dependencies
        PrivateDependencyModuleNames.AddRange(
            new string[] {
                // 필요한 private 모듈들
            }
        );

        // Photon SDK 설정
        SetupPhotonSDK(Target);
    }

    // Photon SDK 설정 함수
    private void SetupPhotonSDK(ReadOnlyTargetRules Target)
    {
        // Windows 플랫폼 설정
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // UE 4.27: Definitions → PublicDefinitions
            PublicDefinitions.Add("_EG_WINDOWS_PLATFORM");

            //  vc17 release 버전 사용 (Visual Studio 2022)
            PublicAdditionalLibraries.AddRange(new string[] {
                Path.Combine(PhotonPath, "Common-cpp", "lib", "Common-cpp_vc17_release_windows_md_x64.lib"),
                Path.Combine(PhotonPath, "LoadBalancing-cpp", "lib", "LoadBalancing-cpp_vc17_release_windows_md_x64.lib"),
                Path.Combine(PhotonPath, "Photon-cpp", "lib", "Photon-cpp_vc17_release_windows_md_x64.lib")
            });
        }
    }

    // Photon 폴더 경로 (UE 4.27 호환)
    private string PhotonPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "Photon")); }
                                          
    }
}