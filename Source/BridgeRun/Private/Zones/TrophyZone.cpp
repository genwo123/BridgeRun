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
    ScoreMultiplier = 1.0f;  // �⺻ ������ 1.0
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
        TimerText->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    }

    // ScoreText ���ŵ� - UI�� ��ü
}

void ATrophyZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATrophyZone, PlacedTrophy);
    DOREPLIFETIME_CONDITION(ATrophyZone, RemainingTime, COND_None);
    DOREPLIFETIME(ATrophyZone, CurrentScore);
    DOREPLIFETIME(ATrophyZone, ScoreMultiplier);  // ���� ���� �߰�
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
    AItem_Trophy* Trophy = Cast<AItem_Trophy>(OtherActor);
    if (Trophy)
    {
        // �̹� ��ġ�� Ʈ���ǰ� �ִ��� Ȯ��
        if (PlacedTrophy)
        {
            // �̹� ��ġ�� Ʈ���ǰ� �ִ� ���, �� ID�� �ٸ��� Ȯ��
            if (Trophy->OwningTeamID != PlacedTrophy->OwningTeamID)
            {
                // ���� Ʈ���� ����
                PlacedTrophy->Destroy();
                PlacedTrophy = nullptr;

                // Ÿ�̸� ����
                if (UWorld* World = GetWorld())
                {
                    World->GetTimerManager().ClearTimer(ScoreTimerHandle);
                    World->GetTimerManager().ClearTimer(UpdateTimerHandle);
                }

                // �� Ʈ���� ��ġ
                ServerHandleTrophyPlacement(Trophy);
            }
        }
        else
        {
            // ��ġ�� Ʈ���ǰ� ���� ��� ���� ��ġ
            ServerHandleTrophyPlacement(Trophy);
        }
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

        // ������Ʈ Ÿ�̸� ���� - �� ª�� �������� ������Ʈ (0.05��)
        World->GetTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &ATrophyZone::UpdateTimer,
            0.05f,  // 0.1�ʿ��� 0.05�ʷ� ����
            true
        );
    }

    // �������Ʈ �̺�Ʈ ȣ�� (Ʈ���� ��ġ �˸�)
    BP_TrophyPlaced(Trophy);
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

        // Ÿ�̸� �ؽ�Ʈ ������Ʈ
        UpdateTimerText();
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
        float CurrentRemaining = World->GetTimerManager().GetTimerRemaining(ScoreTimerHandle);

        // Ÿ�̸Ӱ� ���� �Ϸ�Ǿ����� Ȯ�� (0.1�� ����)
        if (CurrentRemaining < 0.1f)
        {
            RemainingTime = 0.0f;  // 0���� ǥ��
        }
        else
        {
            RemainingTime = CurrentRemaining;
        }

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

// ���� ���� �̺�Ʈ �Լ�
void ATrophyZone::OnRep_ScoreMultiplier()
{
    // �������Ʈ �̺�Ʈ ȣ��
    BP_MultiplierChanged(ScoreMultiplier);
}

// ���� ���� �Լ� (�������Ʈ���� ȣ��)
void ATrophyZone::SetScoreMultiplier(float NewMultiplier)
{
    if (HasAuthority())
    {
        // �������� ���� ����
        ScoreMultiplier = FMath::Max(NewMultiplier, 0.1f);  // �ּҰ� ����
        MulticastOnMultiplierChanged(ScoreMultiplier);
    }
    else
    {
        // Ŭ���̾�Ʈ������ ������ ��û
        ServerSetScoreMultiplier(NewMultiplier);
    }
}

// ���� ���� ���� �Լ�
void ATrophyZone::ServerSetScoreMultiplier_Implementation(float NewMultiplier)
{
    // �ּҰ� ����
    ScoreMultiplier = FMath::Max(NewMultiplier, 0.1f);

    // ��Ƽĳ��Ʈ�� ��� Ŭ���̾�Ʈ���� �˸�
    MulticastOnMultiplierChanged(ScoreMultiplier);
}

// ���� ���� ��Ƽĳ��Ʈ �Լ�
void ATrophyZone::MulticastOnMultiplierChanged_Implementation(float NewMultiplier)
{
    ScoreMultiplier = NewMultiplier;

    // BP �̺�Ʈ ȣ��
    BP_MultiplierChanged(NewMultiplier);
}

void ATrophyZone::ServerUpdateScore_Implementation(int32 NewScore)
{
    // ���� ����
    if (!HasAuthority())
        return;

    // ���� ������ ������Ʈ�ϰ� ������ �� ���� ������Ʈ�� ���� ����
    CurrentScore = NewScore;
    MulticastOnScoreUpdated(CurrentScore);
}

void ATrophyZone::MulticastOnScoreUpdated_Implementation(int32 NewScore)
{
    // �ܼ��� ���� ������ ������Ʈ (�� ���� ������Ʈ�� ���� ����)
    CurrentScore = NewScore;

    // �������Ʈ �̺�Ʈ ȣ��
    BP_ScoreUpdated(TeamID, NewScore);
}

void ATrophyZone::OnScoreTimerComplete()
{
    // ���� ���� �� ��ȿ�� Ȯ��
    if (!HasAuthority() || !IsValid(PlacedTrophy))
        return;

    // Ÿ�̸� ǥ�ø� ��������� 0���� ����
    RemainingTime = 0.0f;
    UpdateTimerText();
    ForceNetUpdate();

    // Ʈ���� �� ��� ���� ��� - ���� ����
    int32 BaseScore = PlacedTrophy->TrophyValue;
    int32 ScoreToAdd = FMath::RoundToInt(BaseScore * ScoreMultiplier);

    // ���� �ν��Ͻ� ��������
    UBridgeRunGameInstance* GameInst = Cast<UBridgeRunGameInstance>(GetGameInstance());
    if (!GameInst) return;

    // Ʈ���� �� ID Ȯ��
    int32 TrophyTeamID = PlacedTrophy->OwningTeamID;

    // ���� �ο� - Ʈ������ Ÿ�Կ� ���� �޶���
    if (ZoneType == ETrophyZoneType::TeamBase)
    {
        // �� ���̽�: Ʈ�������� �� ID�� ���� �ο�
        GameInst->UpdateTeamScore(TeamID, ScoreToAdd);
        UE_LOG(LogTemp, Log, TEXT("Team Base: Team %d earned %d points (x%.1f multiplier)"),
            TeamID, ScoreToAdd, ScoreMultiplier);
    }
    else // Neutral Zone
    {
        // �߸� ����: Ʈ������ �� ID�� ���� �ο�
        if (TrophyTeamID >= 0)
        {
            GameInst->UpdateTeamScore(TrophyTeamID, ScoreToAdd);
            UE_LOG(LogTemp, Log, TEXT("Neutral Zone: Trophy Team %d earned %d points (x%.1f multiplier)"),
                TrophyTeamID, ScoreToAdd, ScoreMultiplier);
        }
    }

    // ���� ���� ������Ʈ (UI ǥ�ÿ�)
    CurrentScore += ScoreToAdd;
    MulticastOnScoreUpdated(CurrentScore);

    // Ÿ�̸� ����
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }

    // Ʈ���� ����
    PlacedTrophy->Destroy();
    PlacedTrophy = nullptr;
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
    // ���� ���� �� BP �̺�Ʈ�� ȣ�� (ScoreText ������Ʈ ����)
    BP_ScoreUpdated(TeamID, CurrentScore);
}
