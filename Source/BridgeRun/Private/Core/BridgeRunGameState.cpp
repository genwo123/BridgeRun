// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunGameState.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameMode.h"

ABridgeRunGameState::ABridgeRunGameState()
{
    // �⺻ 4�� ���� (������ ���� 3-4��)
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

    // �⺻ ��� �ð� ���� (4��)
    MatchTime = 240.0f;

    // ���� �� �ʱ�ȭ
    TotemValues.Add(ETotemType::Normal, 10);
    TotemValues.Add(ETotemType::Gold, 20);
    TotemValues.Add(ETotemType::Diamond, 30);

    // ������ ���� �ʱ�ȭ
    ZoneMultipliers.Add(ETotemZoneType::Team, 1.0f);     // �⺻ ����
    ZoneMultipliers.Add(ETotemZoneType::Neutral, 1.5f);  // �߸��� ����

    // ����� ���� �ʱ�ȭ
    RankPoints.Add(1, 5);  // 1��: 5��
    RankPoints.Add(2, 2);  // 2��: 2��
    RankPoints.Add(3, -1); // 3��: -1��
    RankPoints.Add(4, -2); // 4��: -2��

    // ù ���� ��ǥ ���� ����
    RoundTargetScore = 40;
}

void ABridgeRunGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // ������ �Ӽ� ���
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
    // ��ȿ�� �� ID���� Ȯ��
    if (TeamId >= 0 && TeamId < Teams.Num())
    {
        Teams[TeamId].Score = NewScore;

        // ������ ��ǥ ������ �����ϸ� ���� ���� ��ȣ
        if (NewScore >= RoundTargetScore && !bIsInPreparationPhase && HasAuthority())
        {
            CompleteRound();
        }
    }
}

void ABridgeRunGameState::CompleteRound_Implementation()
{
    if (!HasAuthority()) return;

    // ���� ���� ó��
    bIsInPreparationPhase = true;

    // �� ��� ���
    CalculateTeamRanks();

    // ����� ���� ���� �ο�
    AssignRankPoints();

    // ���� ���� �غ�
    CurrentRound++;

    // ���庰 ��ǥ ���� �� ���� ����
    if (CurrentRound == 2)
    {
        RoundTargetScore = 45; // 2���� ��ǥ ����
    }
    else if (CurrentRound == 3)
    {
        RoundTargetScore = 50; // 3���� ��ǥ ����
    }
    else if (CurrentRound > 3)
    {
        // ��� ���� ����, ���� ���� ����
        DetermineWinner();
        return;
    }

    // ���� ���� �� �� �� ���� �ʱ�ȭ
    for (FBasicTeamInfo& Team : Teams)
    {
        Team.Score = 0;
    }

    // �غ� �ð� �� ���� ���� ����
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        [this]() { StartRound(); },
        PreparationTime,
        false
    );

    // ��� �ð� ����
    MatchTime = RoundTime;
}

void ABridgeRunGameState::StartRound_Implementation()
{
    if (!HasAuthority()) return;

    // ���� ����
    bIsInPreparationPhase = false;
    bGoldenTimeActive = false;

    // 3���忡�� ���Ÿ�� Ÿ�̸� ����
    if (CurrentRound == 3)
    {
        // ���� ���� 1�� ���� ���Ÿ�� Ȱ��ȭ
        FTimerHandle GoldenTimeHandle;
        GetWorld()->GetTimerManager().SetTimer(
            GoldenTimeHandle,
            [this]() { ActivateGoldenTime(); },
            RoundTime - 60.0f, // 1�� ��
            false
        );
    }
}

void ABridgeRunGameState::ActivateGoldenTime_Implementation()
{
    if (!HasAuthority()) return;

    // ���Ÿ�� Ȱ��ȭ (3���� ������ 1��)
    bGoldenTimeActive = true;
}

void ABridgeRunGameState::AddTotemScore(int32 TeamId, ETotemType TotemType, ETotemZoneType ZoneType, float TimeHeld)
{
    if (!HasAuthority() || TeamId < 0 || TeamId >= Teams.Num())
        return;

    // ���� �⺻ ��ġ ��������
    int32 BaseValue = TotemValues[TotemType];

    // ������ ���� ��������
    float ZoneMultiplier = ZoneMultipliers[ZoneType];

    // ���� �ð��� ���� �߰� ����
    float TimeMultiplier = 1.0f;
    if (TimeHeld >= 60.0f)
    {
        TimeMultiplier = 2.0f; // 60�� �̻� ���� �� 2��
    }
    else if (TimeHeld >= 30.0f)
    {
        TimeMultiplier = 1.5f; // 30�� �̻� ���� �� 1.5��
    }

    // ���Ÿ�� ���� ����
    float GoldenTimeMultiplier = bGoldenTimeActive ? 2.0f : 1.0f;

    // ���� ���� ���
    int32 FinalScore = FMath::RoundToInt(BaseValue * ZoneMultiplier * TimeMultiplier * GoldenTimeMultiplier);

    // �� ���� ������Ʈ
    int32 NewScore = Teams[TeamId].Score + FinalScore;
    UpdateTeamScore(TeamId, NewScore);
}

void ABridgeRunGameState::CalculateTeamRanks()
{
    // ���� �������� �� ����
    Teams.Sort([](const FBasicTeamInfo& A, const FBasicTeamInfo& B) {
        return A.Score > B.Score; // �������� ����
        });

    // ��� �Ҵ�
    for (int32 i = 0; i < Teams.Num(); i++)
    {
        Teams[i].CurrentRank = i + 1;

        // 1�� ī��Ʈ ����
        if (i == 0)
        {
            Teams[i].FirstPlaceCount++;
        }
    }
}

void ABridgeRunGameState::AssignRankPoints()
{
    // �� ���� ����� ���� �ο�
    for (FBasicTeamInfo& Team : Teams)
    {
        // ����� ���� ��������
        int32* Points = RankPoints.Find(Team.CurrentRank);
        if (Points)
        {
            // ���� ������ �߰�
            Team.TotalScore += *Points;
        }
    }

    // ���� ������ �� ������
    Teams.Sort([](const FBasicTeamInfo& A, const FBasicTeamInfo& B) {
        return A.TotalScore > B.TotalScore; // �������� ����
        });
}

void ABridgeRunGameState::DetermineWinner()
{
    if (!HasAuthority()) return;

    // ���� ���� ����
    bGameOver = true;

    // ���� ������ �� ����
    Teams.Sort([](const FBasicTeamInfo& A, const FBasicTeamInfo& B) {
        // ������ ������ 1�� Ƚ���� ��
        if (A.TotalScore == B.TotalScore)
        {
            return A.FirstPlaceCount > B.FirstPlaceCount;
        }
        return A.TotalScore > B.TotalScore; // �������� ����
        });

    // 1�� ���� ���ڷ� ����
    if (Teams.Num() > 0)
    {
        WinnerTeamId = Teams[0].TeamId;
    }
}