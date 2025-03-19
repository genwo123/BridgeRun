// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunGameState.h"
#include "Net/UnrealNetwork.h"

ABridgeRunGameState::ABridgeRunGameState()
{
    // 기본 8팀 생성
    Teams.Reserve(8);
    for (int32 i = 0; i < 8; i++)
    {
        FBasicTeamInfo NewTeam;
        NewTeam.TeamId = i;
        NewTeam.Score = 0;
        Teams.Add(NewTeam);
    }

    // 기본 경기 시간 설정 (5분)
    MatchTime = 300.0f;
}

void ABridgeRunGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 복제할 속성 등록
    DOREPLIFETIME(ABridgeRunGameState, Teams);
    DOREPLIFETIME(ABridgeRunGameState, MatchTime);
}

void ABridgeRunGameState::UpdateTeamScore_Implementation(int32 TeamId, int32 NewScore)
{
    // 유효한 팀 ID인지 확인
    if (TeamId >= 0 && TeamId < Teams.Num())
    {
        Teams[TeamId].Score = NewScore;
    }
}