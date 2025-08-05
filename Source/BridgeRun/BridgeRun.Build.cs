using UnrealBuildTool;
using System.IO;

public class BridgeRun : ModuleRules
{
    public BridgeRun(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // === �׽�Ʈ: Build.cs ���� Ȯ�� ===
        System.Console.WriteLine("=== BridgeRun Build.cs �����! ===");

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

        // === Steam SDK ���� (�α� ��ȭ) ===
        string SteamSDKPath = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "Steamworks", "sdk");
        System.Console.WriteLine("Steam SDK ��� üũ: " + SteamSDKPath);
        System.Console.WriteLine("Directory.Exists: " + Directory.Exists(SteamSDKPath));

        if (Directory.Exists(SteamSDKPath))
        {
            System.Console.WriteLine(" Steam SDK ���� �߰�!");

            // ��� ���
            string HeaderPath = Path.Combine(SteamSDKPath, "public");
            PublicIncludePaths.Add(HeaderPath);
            System.Console.WriteLine("��� ��� �߰�: " + HeaderPath);

            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                string LibPath = Path.Combine(SteamSDKPath, "redistributable_bin", "win64");
                string LibFile = Path.Combine(LibPath, "steam_api64.lib");
                System.Console.WriteLine("���̺귯�� ���: " + LibPath);
                System.Console.WriteLine("���̺귯�� ����: " + LibFile);
                System.Console.WriteLine("���� ����: " + File.Exists(LibFile));

                if (File.Exists(LibFile))
                {
                    PublicAdditionalLibraries.Add(LibFile);
                    System.Console.WriteLine(" Steam ���̺귯�� �߰���!");
                }
                else
                {
                    System.Console.WriteLine(" steam_api64.lib ���� ����!");
                }
            }

            PublicDefinitions.Add("STEAM_SDK_ENABLED=1");
            System.Console.WriteLine(" STEAM_SDK_ENABLED=1 ������");
        }
        else
        {
            System.Console.WriteLine(" Steam SDK ���� ����!");
            PublicDefinitions.Add("STEAM_SDK_ENABLED=0");
        }

        // �⺻ �𸮾� ����
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

        System.Console.WriteLine("=== BridgeRun Build.cs �Ϸ�! ===");
    }
}