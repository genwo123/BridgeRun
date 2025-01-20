// Private/Core/BridgeRunGameState.cpp
#include "Core/BridgeRunGameState.h"
#include "Net/UnrealNetwork.h"

ABridgeRunGameState::ABridgeRunGameState()
{
    // �⺻ 8�� ����
    for (int32 i = 0; i < 8; i++)
    {
        FBasicTeamInfo NewTeam;
        NewTeam.TeamId = i;
        NewTeam.Score = 0;
        Teams.Add(NewTeam);
    }
    MatchTime = 300.0f; // 5��
}

void ABridgeRunGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABridgeRunGameState, Teams);
    DOREPLIFETIME(ABridgeRunGameState, MatchTime);
}

void ABridgeRunGameState::UpdateTeamScore_Implementation(int32 TeamId, int32 NewScore)
{
    if (TeamId >= 0 && TeamId < Teams.Num())
    {
        Teams[TeamId].Score = NewScore;
    }
}