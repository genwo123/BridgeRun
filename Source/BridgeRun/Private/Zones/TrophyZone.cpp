// Copyright BridgeRun Game, Inc. All Rights Reserved.

#include "Zones/TrophyZone.h"
#include "Item/Item_Trophy.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Core/BridgeRunGameInstance.h"
#include "Net/UnrealNetwork.h"

ATrophyZone::ATrophyZone()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    InitializeComponents();

    // 초기값 설정
    PlacedTrophy = nullptr;
    ScoreTime = 0.0f;
    RemainingTime = 0.0f;
    CurrentScore = 0;
    ScoreMultiplier = 1.0f;  // 기본 배율은 1.0
}

void ATrophyZone::InitializeComponents()
{
    // 트리거 박스 설정
    SetupTriggerBox();

    // 텍스트 컴포넌트 설정
    SetupTextComponents();
}

void ATrophyZone::SetupTriggerBox()
{
    // TriggerBox 생성 및 기본 설정
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    if (TriggerBox)
    {
        RootComponent = TriggerBox;
        TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ATrophyZone::OnOverlapBegin);
        TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ATrophyZone::OnOverlapEnd);
        TriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
        TriggerBox->SetIsReplicated(true);
    }
}

void ATrophyZone::SetupTextComponents()
{
    // TimerText 설정
    TimerText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TimerText"));
    if (TimerText)
    {
        TimerText->SetupAttachment(RootComponent);
        TimerText->SetHorizontalAlignment(EHTA_Center);
        TimerText->SetVerticalAlignment(EVRTA_TextCenter);
        TimerText->SetWorldSize(70.0f);
        TimerText->SetText(FText::FromString(TEXT("")));
        TimerText->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    }

    // ScoreText 제거됨 - UI로 대체
}

void ATrophyZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATrophyZone, PlacedTrophy);
    DOREPLIFETIME_CONDITION(ATrophyZone, RemainingTime, COND_None);
    DOREPLIFETIME(ATrophyZone, CurrentScore);
    DOREPLIFETIME(ATrophyZone, ScoreMultiplier);  // 배율 복제 추가
}

void ATrophyZone::BeginPlay()
{
    Super::BeginPlay();
}

void ATrophyZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    // 서버 검증 및 유효성 확인
    if (!HasAuthority() || !IsValid(OtherActor))
        return;

    // 트로피 확인
    AItem_Trophy* Trophy = Cast<AItem_Trophy>(OtherActor);
    if (Trophy)
    {
        // 이미 배치된 트로피가 있는지 확인
        if (PlacedTrophy)
        {
            // 이미 배치된 트로피가 있는 경우, 팀 ID가 다른지 확인
            if (Trophy->OwningTeamID != PlacedTrophy->OwningTeamID)
            {
                // 기존 트로피 제거
                PlacedTrophy->Destroy();
                PlacedTrophy = nullptr;

                // 타이머 정리
                if (UWorld* World = GetWorld())
                {
                    World->GetTimerManager().ClearTimer(ScoreTimerHandle);
                    World->GetTimerManager().ClearTimer(UpdateTimerHandle);
                }

                // 새 트로피 배치
                ServerHandleTrophyPlacement(Trophy);
            }
        }
        else
        {
            // 배치된 트로피가 없는 경우 새로 배치
            ServerHandleTrophyPlacement(Trophy);
        }
    }
}

void ATrophyZone::ServerHandleTrophyPlacement_Implementation(AItem_Trophy* Trophy)
{
    // 유효성 검증
    if (!IsValid(Trophy) || PlacedTrophy)
        return;

    // 트로피 설정 및 타이머 초기화
    PlacedTrophy = Trophy;
    RemainingTime = ScoreTime;

    if (UWorld* World = GetWorld())
    {
        // 점수 타이머 설정
        World->GetTimerManager().SetTimer(
            ScoreTimerHandle,
            this,
            &ATrophyZone::OnScoreTimerComplete,
            ScoreTime,
            false
        );

        // 업데이트 타이머 설정 - 더 짧은 간격으로 업데이트 (0.05초)
        World->GetTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &ATrophyZone::UpdateTimer,
            0.05f,  // 0.1초에서 0.05초로 변경
            true
        );
    }

    // 블루프린트 이벤트 호출 (트로피 배치 알림)
    BP_TrophyPlaced(Trophy);
}

void ATrophyZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    // 서버 검증 및 유효성 확인
    if (!HasAuthority() || !IsValid(OtherActor))
        return;

    // 트로피 확인 및 처리
    AItem_Trophy* Trophy = Cast<AItem_Trophy>(OtherActor);
    if (Trophy && Trophy == PlacedTrophy)
    {
        // 타이머 정리
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(ScoreTimerHandle);
            World->GetTimerManager().ClearTimer(UpdateTimerHandle);
        }

        // 상태 초기화
        PlacedTrophy = nullptr;
        RemainingTime = 0.0f;

        // 타이머 텍스트 업데이트
        UpdateTimerText();
    }
}

void ATrophyZone::UpdateTimer()
{
    // 서버 검증 및 유효성 확인
    if (!HasAuthority() || !IsValid(PlacedTrophy))
        return;

    if (UWorld* World = GetWorld())
    {
        // 남은 시간 업데이트
        float CurrentRemaining = World->GetTimerManager().GetTimerRemaining(ScoreTimerHandle);

        // 타이머가 거의 완료되었는지 확인 (0.1초 이하)
        if (CurrentRemaining < 0.1f)
        {
            RemainingTime = 0.0f;  // 0으로 표시
        }
        else
        {
            RemainingTime = CurrentRemaining;
        }

        // 타이머 텍스트 업데이트
        UpdateTimerText();

        // 네트워크 상태 업데이트
        ForceNetUpdate();
    }
}

void ATrophyZone::UpdateTimerText()
{
    // 타이머 텍스트 업데이트
    if (IsValid(TimerText))
    {
        FString TimerString = FString::Printf(TEXT("%.1f"), RemainingTime);
        TimerText->SetText(FText::FromString(TimerString));
    }
}

void ATrophyZone::OnRep_RemainingTime()
{
    // 남은 시간 복제 시 텍스트 업데이트
    UpdateTimerText();
}

// 배율 복제 이벤트 함수
void ATrophyZone::OnRep_ScoreMultiplier()
{
    // 블루프린트 이벤트 호출
    BP_MultiplierChanged(ScoreMultiplier);
}

// 배율 설정 함수 (블루프린트에서 호출)
void ATrophyZone::SetScoreMultiplier(float NewMultiplier)
{
    if (HasAuthority())
    {
        // 서버에서 직접 설정
        ScoreMultiplier = FMath::Max(NewMultiplier, 0.1f);  // 최소값 보장
        MulticastOnMultiplierChanged(ScoreMultiplier);
    }
    else
    {
        // 클라이언트에서는 서버에 요청
        ServerSetScoreMultiplier(NewMultiplier);
    }
}

// 배율 설정 서버 함수
void ATrophyZone::ServerSetScoreMultiplier_Implementation(float NewMultiplier)
{
    // 최소값 보장
    ScoreMultiplier = FMath::Max(NewMultiplier, 0.1f);

    // 멀티캐스트로 모든 클라이언트에게 알림
    MulticastOnMultiplierChanged(ScoreMultiplier);
}

// 배율 변경 멀티캐스트 함수
void ATrophyZone::MulticastOnMultiplierChanged_Implementation(float NewMultiplier)
{
    ScoreMultiplier = NewMultiplier;

    // BP 이벤트 호출
    BP_MultiplierChanged(NewMultiplier);
}

void ATrophyZone::ServerUpdateScore_Implementation(int32 NewScore)
{
    // 서버 검증
    if (!HasAuthority())
        return;

    // 현재 점수만 업데이트하고 별도의 팀 점수 업데이트는 하지 않음
    CurrentScore = NewScore;
    MulticastOnScoreUpdated(CurrentScore);
}

void ATrophyZone::MulticastOnScoreUpdated_Implementation(int32 NewScore)
{
    // 단순히 로컬 점수만 업데이트 (팀 점수 업데이트는 하지 않음)
    CurrentScore = NewScore;

    // 블루프린트 이벤트 호출
    BP_ScoreUpdated(TeamID, NewScore);
}

void ATrophyZone::OnScoreTimerComplete()
{
    // 서버 검증 및 유효성 확인
    if (!HasAuthority() || !IsValid(PlacedTrophy))
        return;

    // 타이머 표시를 명시적으로 0으로 설정
    RemainingTime = 0.0f;
    UpdateTimerText();
    ForceNetUpdate();

    // 트로피 값 기반 점수 계산 - 배율 적용
    int32 BaseScore = PlacedTrophy->TrophyValue;
    int32 ScoreToAdd = FMath::RoundToInt(BaseScore * ScoreMultiplier);

    // 게임 인스턴스 가져오기
    UBridgeRunGameInstance* GameInst = Cast<UBridgeRunGameInstance>(GetGameInstance());
    if (!GameInst) return;

    // 트로피 팀 ID 확인
    int32 TrophyTeamID = PlacedTrophy->OwningTeamID;

    // 점수 부여 - 트로피존 타입에 따라 달라짐
    if (ZoneType == ETrophyZoneType::TeamBase)
    {
        // 팀 베이스: 트로피존의 팀 ID로 점수 부여
        GameInst->UpdateTeamScore(TeamID, ScoreToAdd);
        UE_LOG(LogTemp, Log, TEXT("Team Base: Team %d earned %d points (x%.1f multiplier)"),
            TeamID, ScoreToAdd, ScoreMultiplier);
    }
    else // Neutral Zone
    {
        // 중립 지역: 트로피의 팀 ID로 점수 부여
        if (TrophyTeamID >= 0)
        {
            GameInst->UpdateTeamScore(TrophyTeamID, ScoreToAdd);
            UE_LOG(LogTemp, Log, TEXT("Neutral Zone: Trophy Team %d earned %d points (x%.1f multiplier)"),
                TrophyTeamID, ScoreToAdd, ScoreMultiplier);
        }
    }

    // 로컬 점수 업데이트 (UI 표시용)
    CurrentScore += ScoreToAdd;
    MulticastOnScoreUpdated(CurrentScore);

    // 타이머 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }

    // 트로피 정리
    PlacedTrophy->Destroy();
    PlacedTrophy = nullptr;
}

void ATrophyZone::OnRep_PlacedTrophy()
{
    // 트로피 상태 변경 시 타이머 텍스트 업데이트
    if (IsValid(TimerText))
    {
        if (PlacedTrophy)
        {
            FString TimerString = FString::Printf(TEXT("%.1f"), RemainingTime);
            TimerText->SetText(FText::FromString(TimerString));
        }
        else
        {
            TimerText->SetText(FText::FromString(TEXT("")));
        }
    }
}

void ATrophyZone::OnRep_CurrentScore()
{
    // 점수 변경 시 BP 이벤트만 호출 (ScoreText 업데이트 제거)
    BP_ScoreUpdated(TeamID, CurrentScore);
}
