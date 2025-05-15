// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/Item_Trophy.h"
#include "Engine/World.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "TrophyZone.generated.h"

// Ʈ������ Ÿ�� ������ �߰�
UENUM(BlueprintType)
enum class ETrophyZoneType : uint8
{
    TeamBase UMETA(DisplayName = "Team Base"),    // �� ���̽� (Ʈ�������� �� ID�� ���� �ο�)
    Neutral  UMETA(DisplayName = "Neutral Zone")  // �߸� ���� (Ʈ������ �� ID�� ���� �ο�)
};

UCLASS()
class BRIDGERUN_API ATrophyZone : public AActor
{
    GENERATED_BODY()
public:
    // ������ �� �⺻ �Լ�
    ATrophyZone();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;

    // �� ���� �Ӽ�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    int32 TeamID = 0;  // �⺻���� 0 (ù ��° ��)

    // Ʈ������ Ÿ�� �߰�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    ETrophyZoneType ZoneType = ETrophyZoneType::TeamBase;

    // ���� ���� �ý��� �߰�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Gameplay|Multiplier")
    float ScoreMultiplier = 1.0f;  // �⺻ ������ 1.0

    // ���� ���� �Լ� �� �Ӽ�
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Gameplay")
    int32 GetCurrentScore() const { return CurrentScore; }
    UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentScore, Category = "Gameplay")
    int32 CurrentScore;

    // Ÿ�̸� ���� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void OnScoreTimerComplete();

    // �������Ʈ �̺�Ʈ
    UFUNCTION(BlueprintImplementableEvent, Category = "Gameplay")
    void BP_ScoreUpdated(int32 InTeamID, int32 InNewScore);

    // Ʈ���� ��ġ �̺�Ʈ �߰�
    UFUNCTION(BlueprintImplementableEvent, Category = "Gameplay")
    void BP_TrophyPlaced(AItem_Trophy* Trophy);

    // ���� ���� �̺�Ʈ �߰�
    UFUNCTION(BlueprintImplementableEvent, Category = "Gameplay|Multiplier")
    void BP_MultiplierChanged(float NewMultiplier);

    // ���� ���� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Gameplay|Multiplier")
    void SetScoreMultiplier(float NewMultiplier);

protected:
    // ������Ʈ
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* TriggerBox;
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UTextRenderComponent* TimerText;
    // ScoreText ���ŵ� - UI�� ��ü

    // �����÷��� �Ӽ�
    UPROPERTY(ReplicatedUsing = OnRep_PlacedTrophy)
    AItem_Trophy* PlacedTrophy;
    UPROPERTY(EditAnywhere, Category = "Gameplay")
    float ScoreTime;
    UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
    float RemainingTime;

    // Ÿ�̸� �ڵ�
    FTimerHandle ScoreTimerHandle;
    FTimerHandle UpdateTimerHandle;

    // ���� �̺�Ʈ �Լ�
    UFUNCTION()
    void OnRep_RemainingTime();
    UFUNCTION()
    void OnRep_PlacedTrophy();
    UFUNCTION()
    void OnRep_CurrentScore();

    // ���� ���� �̺�Ʈ �߰�
    UFUNCTION()
    void OnRep_ScoreMultiplier();

    // ������ �̺�Ʈ
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);
    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

    // ������Ʈ �Լ�
    UFUNCTION()
    void UpdateTimer();

    // ���� �Լ�
    UFUNCTION(Server, Reliable)
    void ServerUpdateScore(int32 NewScore);
    UFUNCTION(Server, Reliable)
    void ServerHandleTrophyPlacement(AItem_Trophy* Trophy);
    UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
    void MulticastOnScoreUpdated(int32 NewScore);

    // ���� ���� ���� �Լ� �߰�
    UFUNCTION(Server, Reliable)
    void ServerSetScoreMultiplier(float NewMultiplier);
    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnMultiplierChanged(float NewMultiplier);

private:
    // �ʱ�ȭ �Լ�
    void InitializeComponents();
    void SetupTriggerBox();
    void SetupTextComponents();
    void UpdateTimerText();
};