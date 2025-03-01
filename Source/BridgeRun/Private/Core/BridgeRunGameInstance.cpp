// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunGameInstance.h"

UBridgeRunGameInstance::UBridgeRunGameInstance()
{
    // �� ������
}

void UBridgeRunGameInstance::Init()
{
    Super::Init();

    // �⺻ �� ���� �ʱ�ȭ
    TeamScores.SetNum(NumberOfTeams);
    for (int32 i = 0; i < TeamScores.Num(); i++)
    {
        TeamScores[i] = 0;
    }

    UE_LOG(LogTemp, Log, TEXT("BridgeRunGameInstance initialized with %d teams"), NumberOfTeams);
}

void UBridgeRunGameInstance::UpdateTeamScore(int32 TeamID, int32 NewScore)
{
    // ��ȿ�� �� ID���� Ȯ��
    if (TeamID >= 0 && TeamID < TeamScores.Num())
    {
        // ���� ������Ʈ
        TeamScores[TeamID] = NewScore;

        // �α� ���
        UE_LOG(LogTemp, Log, TEXT("Team %d Score Updated: %d"), TeamID, NewScore);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid TeamID: %d"), TeamID);
    }
}

void UBridgeRunGameInstance::AddTeamScore(int32 TeamID, int32 ScoreToAdd)
{
    // ��ȿ�� �� ID���� Ȯ��
    if (TeamID >= 0 && TeamID < TeamScores.Num())
    {
        // ���� �߰�
        TeamScores[TeamID] += ScoreToAdd;

        // �α� ���
        UE_LOG(LogTemp, Log, TEXT("Team %d Score Added: %d, New Total: %d"),
            TeamID, ScoreToAdd, TeamScores[TeamID]);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid TeamID: %d"), TeamID);
    }
}

int32 UBridgeRunGameInstance::GetTeamScore(int32 TeamID) const
{
    if (TeamID >= 0 && TeamID < TeamScores.Num())
    {
        return TeamScores[TeamID];
    }

    UE_LOG(LogTemp, Warning, TEXT("GetTeamScore: Invalid TeamID: %d"), TeamID);
    return 0;
}

int32 UBridgeRunGameInstance::GetWinningTeam() const
{
    int32 WinningTeam = -1;
    int32 HighestScore = -1;

    for (int32 i = 0; i < TeamScores.Num(); i++)
    {
        if (TeamScores[i] > HighestScore)
        {
            HighestScore = TeamScores[i];
            WinningTeam = i;
        }
    }

    return WinningTeam;
}

void UBridgeRunGameInstance::LogTeamScores()
{
    UE_LOG(LogTemp, Log, TEXT("===== TEAM SCORES ====="));
    for (int32 i = 0; i < TeamScores.Num(); i++)
    {
        UE_LOG(LogTemp, Log, TEXT("Team %d: %d points"), i, TeamScores[i]);
    }
    UE_LOG(LogTemp, Log, TEXT("======================="));
}