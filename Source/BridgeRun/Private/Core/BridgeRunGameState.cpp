// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunGameState.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameMode.h"

ABridgeRunGameState::ABridgeRunGameState()
{
    // 기본 4팀 생성 (문서에 따라 3-4팀)
    Teams.Reserve(4);
    for (int32 i = 0; i < 4; i++)
    {
        FBasicTeamInfo NewTeam;
        NewTeam.TeamId = i;
        NewTeam.Score = 0;
        NewTeam.TotalScore = 0;
        NewTeam.CurrentRank = i + 1;
        NewTeam.FirstPlaceCount = 0;
        Teams.Add(NewTeam);
    }

    // 기본 경기 시간 설정 (4분)
    MatchTime = 240.0f;

    // 토템 값 초기화
    TotemValues.Add(ETotemType::Normal, 10);
    TotemValues.Add(ETotemType::Gold, 20);
    TotemValues.Add(ETotemType::Diamond, 30);

    // 토템존 배율 초기화
    ZoneMultipliers.Add(ETotemZoneType::Team, 1.0f);     // 기본 배율
    ZoneMultipliers.Add(ETotemZoneType::Neutral, 1.5f);  // 중립존 배율

    // 등수별 점수 초기화
    RankPoints.Add(1, 5);  // 1등: 5점
    RankPoints.Add(2, 2);  // 2등: 2점
    RankPoints.Add(3, -1); // 3등: -1점
    RankPoints.Add(4, -2); // 4등: -2점

    // 첫 라운드 목표 점수 설정
    RoundTargetScore = 40;
}

void ABridgeRunGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 복제할 속성 등록
    DOREPLIFETIME(ABridgeRunGameState, Teams);
    DOREPLIFETIME(ABridgeRunGameState, MatchTime);
    DOREPLIFETIME(ABridgeRunGameState, CurrentRound);
    DOREPLIFETIME(ABridgeRunGameState, bIsInPreparationPhase);
    DOREPLIFETIME(ABridgeRunGameState, bGoldenTimeActive);
    DOREPLIFETIME(ABridgeRunGameState, RoundTargetScore);
    DOREPLIFETIME(ABridgeRunGameState, bGameOver);
    DOREPLIFETIME(ABridgeRunGameState, WinnerTeamId);
}

void ABridgeRunGameState::UpdateTeamScore_Implementation(int32 TeamId, int32 NewScore)
{
    // 유효한 팀 ID인지 확인
    if (TeamId >= 0 && TeamId < Teams.Num())
    {
        Teams[TeamId].Score = NewScore;

        // 점수가 목표 점수에 도달하면 라운드 종료 신호
        if (NewScore >= RoundTargetScore && !bIsInPreparationPhase && HasAuthority())
        {
            CompleteRound();
        }
    }
}

void ABridgeRunGameState::CompleteRound_Implementation()
{
    if (!HasAuthority()) return;

    // 라운드 종료 처리
    bIsInPreparationPhase = true;

    // 팀 등수 계산
    CalculateTeamRanks();

    // 등수에 따른 점수 부여
    AssignRankPoints();

    // 다음 라운드 준비
    CurrentRound++;

    // 라운드별 목표 점수 및 설정 변경
    if (CurrentRound == 2)
    {
        RoundTargetScore = 45; // 2라운드 목표 점수
    }
    else if (CurrentRound == 3)
    {
        RoundTargetScore = 50; // 3라운드 목표 점수
    }
    else if (CurrentRound > 3)
    {
        // 모든 라운드 종료, 최종 승자 결정
        DetermineWinner();
        return;
    }

    // 라운드 종료 후 각 팀 점수 초기화
    for (FBasicTeamInfo& Team : Teams)
    {
        Team.Score = 0;
    }

    // 준비 시간 후 다음 라운드 시작
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        [this]() { StartRound(); },
        PreparationTime,
        false
    );

    // 경기 시간 리셋
    MatchTime = RoundTime;
}

void ABridgeRunGameState::StartRound_Implementation()
{
    if (!HasAuthority()) return;

    // 라운드 시작
    bIsInPreparationPhase = false;
    bGoldenTimeActive = false;

    // 3라운드에서 골든타임 타이머 설정
    if (CurrentRound == 3)
    {
        // 라운드 종료 1분 전에 골든타임 활성화
        FTimerHandle GoldenTimeHandle;
        GetWorld()->GetTimerManager().SetTimer(
            GoldenTimeHandle,
            [this]() { ActivateGoldenTime(); },
            RoundTime - 60.0f, // 1분 전
            false
        );
    }
}

void ABridgeRunGameState::ActivateGoldenTime_Implementation()
{
    if (!HasAuthority()) return;

    // 골든타임 활성화 (3라운드 마지막 1분)
    bGoldenTimeActive = true;
}

void ABridgeRunGameState::AddTotemScore(int32 TeamId, ETotemType TotemType, ETotemZoneType ZoneType, float TimeHeld)
{
    if (!HasAuthority() || TeamId < 0 || TeamId >= Teams.Num())
        return;

    // 토템 기본 가치 가져오기
    int32 BaseValue = TotemValues[TotemType];

    // 토템존 배율 가져오기
    float ZoneMultiplier = ZoneMultipliers[ZoneType];

    // 유지 시간에 따른 추가 배율
    float TimeMultiplier = 1.0f;
    if (TimeHeld >= 60.0f)
    {
        TimeMultiplier = 2.0f; // 60초 이상 유지 시 2배
    }
    else if (TimeHeld >= 30.0f)
    {
        TimeMultiplier = 1.5f; // 30초 이상 유지 시 1.5배
    }

    // 골든타임 배율 적용
    float GoldenTimeMultiplier = bGoldenTimeActive ? 2.0f : 1.0f;

    // 최종 점수 계산
    int32 FinalScore = FMath::RoundToInt(BaseValue * ZoneMultiplier * TimeMultiplier * GoldenTimeMultiplier);

    // 팀 점수 업데이트
    int32 NewScore = Teams[TeamId].Score + FinalScore;
    UpdateTeamScore(TeamId, NewScore);
}

void ABridgeRunGameState::CalculateTeamRanks()
{
    // 점수 기준으로 팀 정렬
    Teams.Sort([](const FBasicTeamInfo& A, const FBasicTeamInfo& B) {
        return A.Score > B.Score; // 내림차순 정렬
        });

    // 등수 할당
    for (int32 i = 0; i < Teams.Num(); i++)
    {
        Teams[i].CurrentRank = i + 1;

        // 1등 카운트 증가
        if (i == 0)
        {
            Teams[i].FirstPlaceCount++;
        }
    }
}

void ABridgeRunGameState::AssignRankPoints()
{
    // 각 팀에 등수별 점수 부여
    for (FBasicTeamInfo& Team : Teams)
    {
        // 등수별 점수 가져오기
        int32* Points = RankPoints.Find(Team.CurrentRank);
        if (Points)
        {
            // 누적 점수에 추가
            Team.TotalScore += *Points;
        }
    }

    // 누적 점수로 팀 재정렬
    Teams.Sort([](const FBasicTeamInfo& A, const FBasicTeamInfo& B) {
        return A.TotalScore > B.TotalScore; // 내림차순 정렬
        });
}

void ABridgeRunGameState::DetermineWinner()
{
    if (!HasAuthority()) return;

    // 게임 종료 설정
    bGameOver = true;

    // 최종 점수로 팀 정렬
    Teams.Sort([](const FBasicTeamInfo& A, const FBasicTeamInfo& B) {
        // 점수가 같으면 1등 횟수로 비교
        if (A.TotalScore == B.TotalScore)
        {
            return A.FirstPlaceCount > B.FirstPlaceCount;
        }
        return A.TotalScore > B.TotalScore; // 내림차순 정렬
        });

    // 1등 팀을 승자로 설정
    if (Teams.Num() > 0)
    {
        WinnerTeamId = Teams[0].TeamId;
    }
}