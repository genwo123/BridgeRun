// TrophyZone.cpp
#include "TrophyZone.h"
#include "Item_Trophy.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ATrophyZone::ATrophyZone()
{
    PrimaryActorTick.bCanEverTick = true;

    // TriggerBox ���� �� �⺻ ����
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    if (TriggerBox)
    {
        RootComponent = TriggerBox;
        TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ATrophyZone::OnOverlapBegin);
        TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ATrophyZone::OnOverlapEnd);
        TriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
    }

    // TimerText ����
    TimerText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TimerText"));
    if (TimerText)
    {
        TimerText->SetupAttachment(RootComponent);
        TimerText->SetHorizontalAlignment(EHTA_Center);
        TimerText->SetVerticalAlignment(EVRTA_TextCenter);
        TimerText->SetWorldSize(70.0f);
        // �ʱ� �ؽ�Ʈ�� �����
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
        // �ʱ� �ؽ�Ʈ�� ����� - BP���� ó��
        ScoreText->SetText(FText::FromString(TEXT("")));
    }

    // �ʱⰪ ����
    PlacedTrophy = nullptr;
    // ScoreTime�� BP���� ������ �� �ֵ��� ��
    ScoreTime = 0.0f;
    RemainingTime = 0.0f;
    CurrentScore = 0;
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
    if (!IsValid(OtherActor))
        return;

    // ������ ������ ������ Ȯ��
    UE_LOG(LogTemp, Warning, TEXT("TrophyZone receiving: %s of class %s"),
        *OtherActor->GetName(),
        *OtherActor->GetClass()->GetName());

    if (!IsValid(TimerText))
        return;

    if (OtherActor->IsA(AItem_Trophy::StaticClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("TrophyZone detected Trophy"));

        AItem_Trophy* Trophy = Cast<AItem_Trophy>(OtherActor);
        if (Trophy)
        {
            UE_LOG(LogTemp, Warning, TEXT("Cast successful, setting up timer"));

            PlacedTrophy = Trophy;
            RemainingTime = ScoreTime;

            FString InitialTimerString = FString::Printf(TEXT("%.1f"), ScoreTime);
            TimerText->SetText(FText::FromString(InitialTimerString));

            // ���� Ÿ�̸� ����
            GetWorld()->GetTimerManager().SetTimer(
                ScoreTimerHandle,
                this,
                &ATrophyZone::OnScoreTimerComplete,
                ScoreTime,
                false
            );

            // ������Ʈ Ÿ�̸� ����
            GetWorld()->GetTimerManager().SetTimer(
                UpdateTimerHandle,
                this,
                &ATrophyZone::UpdateTimer,
                0.1f,
                true
            );

            UE_LOG(LogTemp, Warning, TEXT("Timers started. Score time: %.1f"), ScoreTime);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor is not a Trophy"));
    }
}


void ATrophyZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("TrophyZone: OnOverlapEnd Called"));

    if (!IsValid(OtherActor) || !IsValid(TimerText))
    {
        UE_LOG(LogTemp, Warning, TEXT("TrophyZone: OnOverlapEnd - Invalid actor or timer text"));
        return;
    }

    AItem_Trophy* Trophy = Cast<AItem_Trophy>(OtherActor);
    if (!IsValid(Trophy))
    {
        UE_LOG(LogTemp, Warning, TEXT("TrophyZone: OnOverlapEnd - Failed to cast to Trophy"));
        return;
    }

    if (Trophy == PlacedTrophy)
    {
        UE_LOG(LogTemp, Warning, TEXT("TrophyZone: Trophy removed, resetting timer"));
        GetWorld()->GetTimerManager().ClearTimer(ScoreTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
        PlacedTrophy = nullptr;
        TimerText->SetText(FText::FromString(TEXT("")));
    }
}

void ATrophyZone::UpdateTimer()
{
    UE_LOG(LogTemp, Warning, TEXT("UpdateTimer Called"));

    if (!IsValid(PlacedTrophy))
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateTimer: PlacedTrophy is not valid"));
        return;
    }

    if (!IsValid(TimerText))
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateTimer: TimerText is not valid"));
        return;
    }

    RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(ScoreTimerHandle);
    UE_LOG(LogTemp, Warning, TEXT("Remaining Time: %.1f"), RemainingTime);

    if (RemainingTime > 0.0f)
    {
        FString TimerString = FString::Printf(TEXT("%.1f"), RemainingTime);
        TimerText->SetText(FText::FromString(TimerString));
    }
}

void ATrophyZone::OnScoreTimerComplete()
{
    UE_LOG(LogTemp, Warning, TEXT("OnScoreTimerComplete Called"));

    if (!IsValid(PlacedTrophy))
    {
        UE_LOG(LogTemp, Warning, TEXT("OnScoreTimerComplete: PlacedTrophy is not valid"));
        return;
    }

    if (!IsValid(TimerText) || !IsValid(ScoreText))
    {
        UE_LOG(LogTemp, Warning, TEXT("OnScoreTimerComplete: Text components are not valid"));
        return;
    }

    CurrentScore += ScoreAmount;
    UE_LOG(LogTemp, Warning, TEXT("Score increased. Current Score: %d"), CurrentScore);

    FString ScoreString = FString::Printf(TEXT("Score: %d"), CurrentScore);
    ScoreText->SetText(FText::FromString(ScoreString));

    TimerText->SetText(FText::FromString(TEXT("")));

    PlacedTrophy->Destroy();
    PlacedTrophy = nullptr;

    GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);

    UE_LOG(LogTemp, Warning, TEXT("Score timer complete. Trophy destroyed"));
}


void ATrophyZone::UpdateScoreText()
{
    if (ScoreText)
    {
        // BP���� �������� ���̹Ƿ� ���⼭�� �⺻ ������
        FString ScoreString = FString::Printf(TEXT("Score: %d"), CurrentScore);
        ScoreText->SetText(FText::FromString(ScoreString));
    }
}