// Copyright BridgeRun Game, Inc. All Rights Reserved.
#include "Core/BridgeRunPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Characters/Citizen.h"

ABridgeRunPlayerState::ABridgeRunPlayerState()
{
    bReplicates = true;
    bAlwaysRelevant = true;

    // 기본값 설정
    TeamID = -1;
    PlayerNickname = TEXT("");
    bReadyStatus = false;
    bHostStatus = false;

    // 통계 초기화
    TotalPlanksBuilt = 0;
    TotalTentsBuilt = 0;
    TotalTrophyScore = 0;
    TotalHitCount = 0;
    RoundPlanksBuilt = 0;
    RoundTentsBuilt = 0;
    RoundTrophyScore = 0;
    RoundHitCount = 0;
}

void ABridgeRunPlayerState::BeginDestroy()
{
    // Event Dispatcher 정리
    OnPlayerStatsChanged.Clear();
    OnRoundStatsReset.Clear();

    Super::BeginDestroy();
}

void ABridgeRunPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 팀 시스템
    DOREPLIFETIME(ABridgeRunPlayerState, TeamID);

    // 로비 시스템
    DOREPLIFETIME(ABridgeRunPlayerState, PlayerNickname);
    DOREPLIFETIME(ABridgeRunPlayerState, bReadyStatus);
    DOREPLIFETIME(ABridgeRunPlayerState, bHostStatus);

    // 누적 통계
    DOREPLIFETIME(ABridgeRunPlayerState, TotalPlanksBuilt);
    DOREPLIFETIME(ABridgeRunPlayerState, TotalTentsBuilt);
    DOREPLIFETIME(ABridgeRunPlayerState, TotalTrophyScore);
    DOREPLIFETIME(ABridgeRunPlayerState, TotalHitCount);

    // 라운드 통계
    DOREPLIFETIME(ABridgeRunPlayerState, RoundPlanksBuilt);
    DOREPLIFETIME(ABridgeRunPlayerState, RoundTentsBuilt);
    DOREPLIFETIME(ABridgeRunPlayerState, RoundTrophyScore);
    DOREPLIFETIME(ABridgeRunPlayerState, RoundHitCount);
}

// =========================
// 팀 시스템
// =========================

void ABridgeRunPlayerState::SetTeamID(int32 NewTeamID)
{
    if (TeamID != NewTeamID)
    {
        TeamID = NewTeamID;

        if (HasAuthority())
        {
            OnRep_TeamID();
        }

        ForceNetUpdate();
    }
}

void ABridgeRunPlayerState::OnRep_TeamID()
{
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
                Character->MulticastSetTeamMaterial(TeamID);
            }
        }
    }
}

// =========================
// 로비 시스템
// =========================

void ABridgeRunPlayerState::SetPlayerNickname(const FString& NewNickname)
{
    if (HasAuthority())
    {
        PlayerNickname = NewNickname;
    }
    else
    {
        ServerSetPlayerNickname(NewNickname);
    }
}

void ABridgeRunPlayerState::ServerSetPlayerNickname_Implementation(const FString& NewNickname)
{
    PlayerNickname = NewNickname;
    ForceNetUpdate();
}

bool ABridgeRunPlayerState::IsNicknameValid() const
{
    return !PlayerNickname.IsEmpty() && PlayerNickname.Len() > 0 && PlayerNickname.Len() <= 20;
}

void ABridgeRunPlayerState::OnRep_PlayerNickname()
{
    // UI 업데이트 이벤트 (필요시 브로드캐스트)
    if (OnPlayerStatsChanged.IsBound())
    {
        OnPlayerStatsChanged.Broadcast(this, -1);
    }
}

void ABridgeRunPlayerState::SetReadyStatus(bool bNewReady)
{
    if (HasAuthority())
    {
        bReadyStatus = bNewReady;
    }
    else
    {
        ServerSetReadyStatus(bNewReady);
    }
}

void ABridgeRunPlayerState::ServerSetReadyStatus_Implementation(bool bNewReady)
{
    bReadyStatus = bNewReady;
    ForceNetUpdate();
}

void ABridgeRunPlayerState::OnRep_ReadyStatus()
{
    // Ready 상태 변경 시 UI 업데이트
    if (OnPlayerStatsChanged.IsBound())
    {
        OnPlayerStatsChanged.Broadcast(this, -2); // Ready 상태 변경 신호
    }
}

void ABridgeRunPlayerState::SetHostStatus(bool bNewHost)
{
    if (HasAuthority())
    {
        bHostStatus = bNewHost;
        ForceNetUpdate();
    }
}

void ABridgeRunPlayerState::OnRep_HostStatus()
{
    // 방장 상태 변경 시 UI 업데이트
    if (OnPlayerStatsChanged.IsBound())
    {
        OnPlayerStatsChanged.Broadcast(this, -3); // 방장 상태 변경 신호
    }
}

// =========================
// 게임 통계 시스템
// =========================

void ABridgeRunPlayerState::ServerAddPlankBuilt_Implementation()
{
    if (!HasAuthority()) return;

    TotalPlanksBuilt++;
    RoundPlanksBuilt++;
}

void ABridgeRunPlayerState::ServerAddTentBuilt_Implementation()
{
    if (!HasAuthority()) return;

    TotalTentsBuilt++;
    RoundTentsBuilt++;
}

void ABridgeRunPlayerState::ServerAddTrophyScore_Implementation(int32 TrophyPoints)
{
    if (!HasAuthority()) return;

    TotalTrophyScore += TrophyPoints;
    RoundTrophyScore += TrophyPoints;
}

void ABridgeRunPlayerState::ServerAddHitCount_Implementation()
{
    if (!HasAuthority()) return;

    TotalHitCount++;
    RoundHitCount++;
}

void ABridgeRunPlayerState::ServerResetRoundStats_Implementation()
{
    if (!HasAuthority()) return;

    RoundPlanksBuilt = 0;
    RoundTentsBuilt = 0;
    RoundTrophyScore = 0;
    RoundHitCount = 0;
}

void ABridgeRunPlayerState::OnRep_TotalStats()
{
    
}

void ABridgeRunPlayerState::OnRep_RoundStats()
{
    
}