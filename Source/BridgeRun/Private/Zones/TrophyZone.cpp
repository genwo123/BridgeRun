// Copyright BridgeRun Game, Inc. All Rights Reserved.

#include "Zones/TrophyZone.h"
#include "Item/Item_Trophy.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

ATrophyZone::ATrophyZone()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // TriggerBox ���� �� �⺻ ����
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    if (TriggerBox)
    {
        RootComponent = TriggerBox;
        TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ATrophyZone::OnOverlapBegin);
        TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ATrophyZone::OnOverlapEnd);
        TriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
        TriggerBox->SetIsReplicated(true);
    }

    // TimerText ����
    TimerText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TimerText"));
    if (TimerText)
    {
        TimerText->SetupAttachment(RootComponent);
        TimerText->SetHorizontalAlignment(EHTA_Center);
        TimerText->SetVerticalAlignment(EVRTA_TextCenter);
        TimerText->SetWorldSize(70.0f);
        TimerText->SetText(FText::FromString(TEXT("")));
    }

    // ScoreText ����
    ScoreText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ScoreText"));
    if (ScoreText)
    {
        ScoreText->SetupAttachment(RootComponent);
        ScoreText->SetHorizontalAlignment(EHTA_Center);
        ScoreText->SetVerticalAlignment(EVRTA_TextCenter);
        ScoreText->SetWorldSize(70.0f);
        ScoreText->SetText(FText::FromString(TEXT("")));
    }

    // �ʱⰪ ����
    PlacedTrophy = nullptr;
    ScoreTime = 0.0f;
    RemainingTime = 0.0f;
    CurrentScore = 0;
}

void ATrophyZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATrophyZone, PlacedTrophy);
    DOREPLIFETIME_CONDITION(ATrophyZone, RemainingTime, COND_None);  // �Ǵ� REPNOTIFY_Always ���
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
        World->GetTimerManager().SetTimer(
            ScoreTimerHandle,
            this,
            &ATrophyZone::OnScoreTimerComplete,
            ScoreTime,
            false
        );

        World->GetTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &ATrophyZone::UpdateTimer,
            0.1f,
            true
        );
    }

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

        // Ÿ�̸� �ؽ�Ʈ ������Ʈ
        if (IsValid(TimerText))
        {
            FString TimerString = FString::Printf(TEXT("%.1f"), RemainingTime);
            TimerText->SetText(FText::FromString(TimerString));
        }

        // ��Ʈ��ũ ���� ������Ʈ
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

    // ������ ScoreAmount ��� Ʈ������ �� ���
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