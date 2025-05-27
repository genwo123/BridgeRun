// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Characters/Citizen.h"
#include "Engine/Engine.h"

ABridgeRunPlayerState::ABridgeRunPlayerState()
{
    // ��Ʈ��ũ ���� Ȱ��ȭ
    bReplicates = true;
    bAlwaysRelevant = true;

    // �ʱ� �� ID�� -1 (���Ҵ�)
    TeamID = -1;

    // ��� ��� �ʱ�ȭ (0����)
    TotalPlanksBuilt = 0;
    TotalTentsBuilt = 0;
    TotalTrophyScore = 0;
    TotalHitCount = 0;

    RoundPlanksBuilt = 0;
    RoundTentsBuilt = 0;
    RoundTrophyScore = 0;
    RoundHitCount = 0;
}

void ABridgeRunPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // ���� �� ����
    DOREPLIFETIME(ABridgeRunPlayerState, TeamID);
    DOREPLIFETIME(ABridgeRunPlayerState, DisplayName);

    // ���� ��� (���� ��ü)
    DOREPLIFETIME(ABridgeRunPlayerState, TotalPlanksBuilt);
    DOREPLIFETIME(ABridgeRunPlayerState, TotalTentsBuilt);
    DOREPLIFETIME(ABridgeRunPlayerState, TotalTrophyScore);
    DOREPLIFETIME(ABridgeRunPlayerState, TotalHitCount);

    // ���� ��� (���庰)
    DOREPLIFETIME(ABridgeRunPlayerState, RoundPlanksBuilt);
    DOREPLIFETIME(ABridgeRunPlayerState, RoundTentsBuilt);
    DOREPLIFETIME(ABridgeRunPlayerState, RoundTrophyScore);
    DOREPLIFETIME(ABridgeRunPlayerState, RoundHitCount);
}

// =========================
// ���� �� ���� �Լ��� (����)
// =========================

void ABridgeRunPlayerState::SetTeamID(int32 NewTeamID)
{
    TeamID = NewTeamID;
}

int32 ABridgeRunPlayerState::GetTeamID() const
{
    return TeamID;
}

void ABridgeRunPlayerState::OnRep_TeamID()
{
    UE_LOG(LogTemp, Log, TEXT("PlayerState TeamID changed to %d"), TeamID);

    // ���� ĳ���Ͱ� �ִ� ��� �� ��Ƽ���� ������Ʈ
    AController* OwningController = Cast<AController>(GetOwner());
    if (OwningController)
    {
        APawn* ControlledPawn = OwningController->GetPawn();
        if (ControlledPawn)
        {
            ACitizen* Character = Cast<ACitizen>(ControlledPawn);
            if (Character)
            {
                Character->TeamID = TeamID;
                Character->SetTeamMaterial(TeamID);
            }
        }
    }
}

// =========================
// ���� �߰�: ��� ������Ʈ RPC �Լ���
// =========================

void ABridgeRunPlayerState::ServerAddPlankBuilt_Implementation()
{
    if (!HasAuthority()) return;

    // ���� ���� ���� ��� �� �� ����
    TotalPlanksBuilt++;
    RoundPlanksBuilt++;

    UE_LOG(LogTemp, Log, TEXT("Player %s: Plank built. Total: %d, Round: %d"),
        *GetPlayerName(), TotalPlanksBuilt, RoundPlanksBuilt);
}

void ABridgeRunPlayerState::ServerAddTentBuilt_Implementation()
{
    if (!HasAuthority()) return;

    // ���� ���� ���� ��� �� �� ����
    TotalTentsBuilt++;
    RoundTentsBuilt++;

    UE_LOG(LogTemp, Log, TEXT("Player %s: Tent built. Total: %d, Round: %d"),
        *GetPlayerName(), TotalTentsBuilt, RoundTentsBuilt);
}

void ABridgeRunPlayerState::ServerAddTrophyScore_Implementation(int32 TrophyPoints)
{
    if (!HasAuthority()) return;

    // ���� ���� ���� ��� �� �� ����
    TotalTrophyScore += TrophyPoints;
    RoundTrophyScore += TrophyPoints;

    UE_LOG(LogTemp, Log, TEXT("Player %s: Trophy score +%d. Total: %d, Round: %d"),
        *GetPlayerName(), TrophyPoints, TotalTrophyScore, RoundTrophyScore);
}

void ABridgeRunPlayerState::ServerAddHitCount_Implementation()
{
    if (!HasAuthority()) return;

    // ���� ���� ���� ��� �� �� ����
    TotalHitCount++;
    RoundHitCount++;

    UE_LOG(LogTemp, Log, TEXT("Player %s: Hit count. Total: %d, Round: %d"),
        *GetPlayerName(), TotalHitCount, RoundHitCount);
}

void ABridgeRunPlayerState::ServerResetRoundStats_Implementation()
{
    if (!HasAuthority()) return;

    // ���� ��踸 �ʱ�ȭ (���� ���� ����)
    RoundPlanksBuilt = 0;
    RoundTentsBuilt = 0;
    RoundTrophyScore = 0;
    RoundHitCount = 0;

    UE_LOG(LogTemp, Log, TEXT("Player %s: Round stats reset"), *GetPlayerName());
}

// =========================
// ���� �Լ��� (�ǽð� ������Ʈ �̺�Ʈ �߻�)
// =========================

void ABridgeRunPlayerState::OnRep_TotalStats()
{
    // BP���� ���ε��� �� �ִ� �̺�Ʈ ��ε�ĳ��Ʈ
    if (OnPlayerStatsChanged.IsBound())
    {
        OnPlayerStatsChanged.Broadcast(this, (int32)EStatType::Trophy); // ��ǥ�� Trophy Ÿ��
    }

}

void ABridgeRunPlayerState::OnRep_RoundStats()
{
    // BP���� ���ε��� �� �ִ� �̺�Ʈ ��ε�ĳ��Ʈ
    if (OnPlayerStatsChanged.IsBound())
    {
        OnPlayerStatsChanged.Broadcast(this, (int32)EStatType::Planks); // ��ǥ�� Planks Ÿ��
    }

}

void ABridgeRunPlayerState::SetDisplayName(const FString& NewDisplayName)
{
    if (HasAuthority()) // ���������� ���� ����
    {
        DisplayName = NewDisplayName;
        UE_LOG(LogTemp, Log, TEXT("Player %s display name set to: %s"),
            *GetPlayerName(), *DisplayName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SetDisplayName can only be called on server"));
    }
}

bool ABridgeRunPlayerState::IsDisplayNameValid() const
{
    return !DisplayName.IsEmpty() && DisplayName.Len() > 0 && DisplayName.Len() <= 20;
}

void ABridgeRunPlayerState::OnRep_DisplayName()
{
    UE_LOG(LogTemp, Log, TEXT("Player %s display name replicated to: %s"),
        *GetPlayerName(), *DisplayName);

    // UI ������Ʈ �̺�Ʈ ��ε�ĳ��Ʈ (�ʿ��� ���)
    if (OnPlayerStatsChanged.IsBound())
    {
        OnPlayerStatsChanged.Broadcast(this, -1); // Ư���� Ÿ������ �̸� ���� �˸�
    }
}