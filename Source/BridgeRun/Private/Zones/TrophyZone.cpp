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
    }

    // ScoreText 설정
    ScoreText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ScoreText"));
    if (ScoreText)
    {
        ScoreText->SetupAttachment(RootComponent);
        ScoreText->SetHorizontalAlignment(EHTA_Center);
        ScoreText->SetVerticalAlignment(EVRTA_TextCenter);
        ScoreText->SetWorldSize(70.0f);
        ScoreText->SetText(FText::FromString(TEXT("")));
    }
}

void ATrophyZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATrophyZone, PlacedTrophy);
    DOREPLIFETIME_CONDITION(ATrophyZone, RemainingTime, COND_None);
    DOREPLIFETIME(ATrophyZone, CurrentScore);
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
    if (AItem_Trophy* Trophy = Cast<AItem_Trophy>(OtherActor))
    {
        ServerHandleTrophyPlacement(Trophy);
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

        // 업데이트 타이머 설정
        World->GetTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &ATrophyZone::UpdateTimer,
            0.1f,
            true
        );
    }
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
        RemainingTime = World->GetTimerManager().GetTimerRemaining(ScoreTimerHandle);

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

void ATrophyZone::OnScoreTimerComplete()
{
    // 서버 검증 및 유효성 확인
    if (!HasAuthority() || !IsValid(PlacedTrophy))
        return;

    // 트로피 값 기반 점수 추가
    int32 ScoreToAdd = PlacedTrophy->TrophyValue;
    ServerUpdateScore(CurrentScore + ScoreToAdd);

    // 타이머 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }

    // 트로피 정리
    PlacedTrophy->Destroy();
    PlacedTrophy = nullptr;
}

void ATrophyZone::ServerUpdateScore_Implementation(int32 NewScore)
{
    // 서버 검증
    if (!HasAuthority())
        return;

    // 점수 업데이트 및 멀티캐스트 전파
    CurrentScore = NewScore;
    MulticastOnScoreUpdated(CurrentScore);
}

void ATrophyZone::MulticastOnScoreUpdated_Implementation(int32 NewScore)
{
    // 모든 클라이언트에서 점수 업데이트
    CurrentScore = NewScore;
    UpdateScoreText();

    // 게임 인스턴스를 통한 팀 점수 업데이트
    if (UBridgeRunGameInstance* GameInst = Cast<UBridgeRunGameInstance>(GetGameInstance()))
    {
        GameInst->UpdateTeamScore(TeamID, NewScore);
    }

    // 블루프린트 이벤트 호출
    BP_ScoreUpdated(TeamID, NewScore);
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
    // 점수 변경 시 텍스트 업데이트
    UpdateScoreText();
}

void ATrophyZone::UpdateScoreText()
{
    // 점수 텍스트 업데이트
    if (ScoreText)
    {
        FString ScoreString = FString::Printf(TEXT("%d"), CurrentScore);
        ScoreText->SetText(FText::FromString(ScoreString));
    }
}