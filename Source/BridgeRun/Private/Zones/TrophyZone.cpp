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

    // �ʱⰪ ����
    PlacedTrophy = nullptr;
    ScoreTime = 0.0f;
    RemainingTime = 0.0f;
    CurrentScore = 0;
}

void ATrophyZone::InitializeComponents()
{
    // Ʈ���� �ڽ� ����
    SetupTriggerBox();

    // �ؽ�Ʈ ������Ʈ ����
    SetupTextComponents();
}

void ATrophyZone::SetupTriggerBox()
{
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
}

void ATrophyZone::SetupTextComponents()
{
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
    // ���� ���� �� ��ȿ�� Ȯ��
    if (!HasAuthority() || !IsValid(OtherActor))
        return;

    // Ʈ���� Ȯ��
    if (AItem_Trophy* Trophy = Cast<AItem_Trophy>(OtherActor))
    {
        ServerHandleTrophyPlacement(Trophy);
    }
}

void ATrophyZone::ServerHandleTrophyPlacement_Implementation(AItem_Trophy* Trophy)
{
    // ��ȿ�� ����
    if (!IsValid(Trophy) || PlacedTrophy)
        return;

    // Ʈ���� ���� �� Ÿ�̸� �ʱ�ȭ
    PlacedTrophy = Trophy;
    RemainingTime = ScoreTime;

    if (UWorld* World = GetWorld())
    {
        // ���� Ÿ�̸� ����
        World->GetTimerManager().SetTimer(
            ScoreTimerHandle,
            this,
            &ATrophyZone::OnScoreTimerComplete,
            ScoreTime,
            false
        );

        // ������Ʈ Ÿ�̸� ����
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
    // ���� ���� �� ��ȿ�� Ȯ��
    if (!HasAuthority() || !IsValid(OtherActor))
        return;

    // Ʈ���� Ȯ�� �� ó��
    AItem_Trophy* Trophy = Cast<AItem_Trophy>(OtherActor);
    if (Trophy && Trophy == PlacedTrophy)
    {
        // Ÿ�̸� ����
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(ScoreTimerHandle);
            World->GetTimerManager().ClearTimer(UpdateTimerHandle);
        }

        // ���� �ʱ�ȭ
        PlacedTrophy = nullptr;
        RemainingTime = 0.0f;
    }
}

void ATrophyZone::UpdateTimer()
{
    // ���� ���� �� ��ȿ�� Ȯ��
    if (!HasAuthority() || !IsValid(PlacedTrophy))
        return;

    if (UWorld* World = GetWorld())
    {
        // ���� �ð� ������Ʈ
        RemainingTime = World->GetTimerManager().GetTimerRemaining(ScoreTimerHandle);

        // Ÿ�̸� �ؽ�Ʈ ������Ʈ
        UpdateTimerText();

        // ��Ʈ��ũ ���� ������Ʈ
        ForceNetUpdate();
    }
}

void ATrophyZone::UpdateTimerText()
{
    // Ÿ�̸� �ؽ�Ʈ ������Ʈ
    if (IsValid(TimerText))
    {
        FString TimerString = FString::Printf(TEXT("%.1f"), RemainingTime);
        TimerText->SetText(FText::FromString(TimerString));
    }
}

void ATrophyZone::OnRep_RemainingTime()
{
    // ���� �ð� ���� �� �ؽ�Ʈ ������Ʈ
    UpdateTimerText();
}

void ATrophyZone::OnScoreTimerComplete()
{
    // ���� ���� �� ��ȿ�� Ȯ��
    if (!HasAuthority() || !IsValid(PlacedTrophy))
        return;

    // Ʈ���� �� ��� ���� �߰�
    int32 ScoreToAdd = PlacedTrophy->TrophyValue;
    ServerUpdateScore(CurrentScore + ScoreToAdd);

    // Ÿ�̸� ����
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }

    // Ʈ���� ����
    PlacedTrophy->Destroy();
    PlacedTrophy = nullptr;
}

void ATrophyZone::ServerUpdateScore_Implementation(int32 NewScore)
{
    // ���� ����
    if (!HasAuthority())
        return;

    // ���� ������Ʈ �� ��Ƽĳ��Ʈ ����
    CurrentScore = NewScore;
    MulticastOnScoreUpdated(CurrentScore);
}

void ATrophyZone::MulticastOnScoreUpdated_Implementation(int32 NewScore)
{
    // ��� Ŭ���̾�Ʈ���� ���� ������Ʈ
    CurrentScore = NewScore;
    UpdateScoreText();

    // ���� �ν��Ͻ��� ���� �� ���� ������Ʈ
    if (UBridgeRunGameInstance* GameInst = Cast<UBridgeRunGameInstance>(GetGameInstance()))
    {
        GameInst->UpdateTeamScore(TeamID, NewScore);
    }

    // �������Ʈ �̺�Ʈ ȣ��
    BP_ScoreUpdated(TeamID, NewScore);
}

void ATrophyZone::OnRep_PlacedTrophy()
{
    // Ʈ���� ���� ���� �� Ÿ�̸� �ؽ�Ʈ ������Ʈ
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
    // ���� ���� �� �ؽ�Ʈ ������Ʈ
    UpdateScoreText();
}

void ATrophyZone::UpdateScoreText()
{
    // ���� �ؽ�Ʈ ������Ʈ
    if (ScoreText)
    {
        FString ScoreString = FString::Printf(TEXT("%d"), CurrentScore);
        ScoreText->SetText(FText::FromString(ScoreString));
    }
}