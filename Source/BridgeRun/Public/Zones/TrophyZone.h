// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/Item_Trophy.h"
#include "Engine/World.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "TrophyZone.generated.h"

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

protected:
    // ������Ʈ
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* TriggerBox;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UTextRenderComponent* TimerText;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UTextRenderComponent* ScoreText;

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

    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void UpdateScoreText();

    // ���� �Լ�
    UFUNCTION(Server, Reliable)
    void ServerUpdateScore(int32 NewScore);

    UFUNCTION(Server, Reliable)
    void ServerHandleTrophyPlacement(AItem_Trophy* Trophy);

    UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
    void MulticastOnScoreUpdated(int32 NewScore);

private:
    // �ʱ�ȭ �Լ�
    void InitializeComponents();
    void SetupTriggerBox();
    void SetupTextComponents();
    void UpdateTimerText();
};