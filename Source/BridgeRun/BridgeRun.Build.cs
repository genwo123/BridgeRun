using UnrealBuildTool;
using System.IO;

public class BridgeRun : ModuleRules
{
    public BridgeRun(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // === 테스트: Build.cs 실행 확인 ===
        System.Console.WriteLine("=== BridgeRun Build.cs 실행됨! ===");

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

        // === Steam SDK 설정 (로그 강화) ===
        string SteamSDKPath = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "Steamworks", "sdk");
        System.Console.WriteLine("Steam SDK 경로 체크: " + SteamSDKPath);
        System.Console.WriteLine("Directory.Exists: " + Directory.Exists(SteamSDKPath));

        if (Directory.Exists(SteamSDKPath))
        {
            System.Console.WriteLine(" Steam SDK 폴더 발견!");

            // 헤더 경로
            string HeaderPath = Path.Combine(SteamSDKPath, "public");
            PublicIncludePaths.Add(HeaderPath);
            System.Console.WriteLine("헤더 경로 추가: " + HeaderPath);

            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                string LibPath = Path.Combine(SteamSDKPath, "redistributable_bin", "win64");
                string LibFile = Path.Combine(LibPath, "steam_api64.lib");
                System.Console.WriteLine("라이브러리 경로: " + LibPath);
                System.Console.WriteLine("라이브러리 파일: " + LibFile);
                System.Console.WriteLine("파일 존재: " + File.Exists(LibFile));

                if (File.Exists(LibFile))
                {
                    PublicAdditionalLibraries.Add(LibFile);
                    System.Console.WriteLine(" Steam 라이브러리 추가됨!");
                }
                else
                {
                    System.Console.WriteLine(" steam_api64.lib 파일 없음!");
                }
            }

            PublicDefinitions.Add("STEAM_SDK_ENABLED=1");
            System.Console.WriteLine(" STEAM_SDK_ENABLED=1 설정됨");
        }
        else
        {
            System.Console.WriteLine(" Steam SDK 폴더 없음!");
            PublicDefinitions.Add("STEAM_SDK_ENABLED=0");
        }

        // 기본 언리얼 모듈들
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
                "SimpleLobbySystem",
                "OnlineSubsystem",
                "OnlineSubsystemSteam"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "OnlineSubsystem",
                "OnlineSubsystemSteam"
            }
        );

        System.Console.WriteLine("=== BridgeRun Build.cs 완료! ===");
    }
}