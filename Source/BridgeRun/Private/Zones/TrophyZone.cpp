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

    // 초기값 설정
    PlacedTrophy = nullptr;
    ScoreTime = 0.0f;
    RemainingTime = 0.0f;
    CurrentScore = 0;
}

void ATrophyZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATrophyZone, PlacedTrophy);
    DOREPLIFETIME_CONDITION(ATrophyZone, RemainingTime, COND_None);  // 또는 REPNOTIFY_Always 사용
    DOREPLIFETIME(ATrophyZone, CurrentScore);
}

void ATrophyZone::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("TrophyZone BeginPlay Called"));
}

void ATrophyZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!HasAuthority() || !IsValid(OtherActor))
        return;

    UE_LOG(LogTemp, Warning, TEXT("TrophyZone receiving: %s of class %s"),
        *OtherActor->GetName(),
        *OtherActor->GetClass()->GetName());

    if (AItem_Trophy* Trophy = Cast<AItem_Trophy>(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("TrophyZone detected Trophy"));
        ServerHandleTrophyPlacement(Trophy);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor is not a Trophy"));
    }
}

void ATrophyZone::ServerHandleTrophyPlacement_Implementation(AItem_Trophy* Trophy)
{
    if (!IsValid(Trophy) || PlacedTrophy)
        return;

    PlacedTrophy = Trophy;
    RemainingTime = ScoreTime;

    if (UWorld* World = GetWorld())
    {
        // 타이머 설정 확인
        World->GetTimerManager().SetTimer(
            ScoreTimerHandle,
            this,
            &ATrophyZone::OnScoreTimerComplete,
            ScoreTime,  // 여기가 총 지속 시간
            false       // 반복 없음
        );

        // 타이머 업데이트 설정 확인
        World->GetTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &ATrophyZone::UpdateTimer,
            0.1f,       // 0.1초마다 업데이트
            true        // 반복
        );
    }

    // 디버그 로그 추가
    UE_LOG(LogTemp, Warning, TEXT("Timers started. Score time: %.1f"), ScoreTime);
}

void ATrophyZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (!HasAuthority() || !IsValid(OtherActor))
        return;

    UE_LOG(LogTemp, Warning, TEXT("TrophyZone: OnOverlapEnd Called"));

    AItem_Trophy* Trophy = Cast<AItem_Trophy>(OtherActor);
    if (Trophy && Trophy == PlacedTrophy)
    {
        UE_LOG(LogTemp, Warning, TEXT("TrophyZone: Trophy removed, resetting timer"));

        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(ScoreTimerHandle);
            World->GetTimerManager().ClearTimer(UpdateTimerHandle);
        }

        PlacedTrophy = nullptr;
        RemainingTime = 0.0f;
    }
}

void ATrophyZone::UpdateTimer()
{
    if (!HasAuthority() || !IsValid(PlacedTrophy))
        return;

    if (UWorld* World = GetWorld())
    {
        RemainingTime = World->GetTimerManager().GetTimerRemaining(ScoreTimerHandle);

        // 타이머가 거의 종료되었는지 확인
        bool bIsTimerActive = World->GetTimerManager().IsTimerActive(ScoreTimerHandle);
        float ElapsedTime = World->GetTimerManager().GetTimerElapsed(ScoreTimerHandle);

        UE_LOG(LogTemp, Log, TEXT("Timer: Remaining=%.2f, Elapsed=%.2f, Active=%d"),
            RemainingTime, ElapsedTime, bIsTimerActive ? 1 : 0);

        // 타이머가 0.2초 미만이고 아직 활성화되어 있으면 강제 종료 고려
        if (RemainingTime < 0.2f && bIsTimerActive)
        {
            UE_LOG(LogTemp, Warning, TEXT("Timer almost complete, considering force complete"));
            // 필요시 여기서 OnScoreTimerComplete 직접 호출 가능
        }

        // 타이머 텍스트 업데이트
        if (IsValid(TimerText))
        {
            FString TimerString = FString::Printf(TEXT("%.1f"), RemainingTime);
            TimerText->SetText(FText::FromString(TimerString));
        }

        // 네트워크 상태 업데이트
        ForceNetUpdate();
    }
}

void ATrophyZone::OnRep_RemainingTime()
{
    if (IsValid(TimerText))
    {
        FString TimerString = FString::Printf(TEXT("%.1f"), RemainingTime);
        TimerText->SetText(FText::FromString(TimerString));
    }
}







void ATrophyZone::OnScoreTimerComplete()
{
    if (!HasAuthority() || !IsValid(PlacedTrophy))
        return;

    // 고정된 ScoreAmount 대신 트로피의 값 사용
    int32 ScoreToAdd = PlacedTrophy->TrophyValue;
    ServerUpdateScore(CurrentScore + ScoreToAdd);

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }

    PlacedTrophy->Destroy();
    PlacedTrophy = nullptr;

}


void ATrophyZone::ServerUpdateScore_Implementation(int32 NewScore)
{
    if (!HasAuthority())
        return;

    CurrentScore = NewScore;
    MulticastOnScoreUpdated(CurrentScore);
}

void ATrophyZone::MulticastOnScoreUpdated_Implementation(int32 NewScore)
{
    CurrentScore = NewScore;
    UpdateScoreText();

    // 게임 인스턴스를 통해 점수 업데이트
    if (UBridgeRunGameInstance* GameInst = Cast<UBridgeRunGameInstance>(GetGameInstance()))
    {
        // 해당 팀의 점수 업데이트
        GameInst->UpdateTeamScore(TeamID, NewScore);

        // 또는 점수 추가 방식 사용
        // GameInst->AddTeamScore(TeamID, ScoreToAdd);

        // 로그 출력
        GameInst->LogTeamScores();
    }

    BP_ScoreUpdated(TeamID, NewScore);
}

void ATrophyZone::OnRep_PlacedTrophy()
{
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
    UpdateScoreText();
}

void ATrophyZone::UpdateScoreText()
{
    if (ScoreText)
    {
        FString ScoreString = FString::Printf(TEXT("%d"), CurrentScore);
        ScoreText->SetText(FText::FromString(ScoreString));
    }
}