// Citizen.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InvenComponent.h"
#include "PlayerModeTypes.h"
#include "Citizen.generated.h"

UCLASS(Blueprintable)
class BRIDGERUN_API ACitizen : public ACharacter
{
    GENERATED_BODY()
public:
    ACitizen();
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    UFUNCTION()
    UCameraComponent* GetFollowCamera() const { return CameraComponent; }

protected:
    virtual void BeginPlay() override;

    UPROPERTY()
    class AItem_Trophy* HeldTrophy;

    // 기본 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USpringArmComponent* SpringArmComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCameraComponent* CameraComponent;

    // 기능별 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UInvenComponent* InvenComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPlayerModeComponent* PlayerModeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBuildingComponent* BuildingComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCombatComponent* CombatComponent;

    // 상호작용
    UPROPERTY(EditAnywhere, Category = "Interaction")
    float InteractionRange = 100.0f;

    // UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    class UUserWidget* InventoryWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UUserWidget> InventoryWidgetClass;

    // 모드 변경
    UFUNCTION()
    void OnPlayerModeChanged(EPlayerMode NewMode, EPlayerMode OldMode);

    // 입력 처리
    void MoveForward(float Value);
    void MoveRight(float Value);
    void StartJump();
    void StopJump();
    void Turn(float Value);
    void LookUp(float Value);
    void Interact();

    // 슬롯 선택
    void SelectSlot1() { SelectInventorySlot(EInventorySlot::Plank); }
    void SelectSlot2() { SelectInventorySlot(EInventorySlot::Tent); }
    void SelectSlot3() { SelectInventorySlot(EInventorySlot::Telescope); }
    void SelectSlot4() { SelectInventorySlot(EInventorySlot::Gun); }
    void SelectSlot5() { SelectInventorySlot(EInventorySlot::Trophy); }

public:
    void SelectInventorySlot(EInventorySlot Slot);
    UPlayerModeComponent* GetPlayerModeComponent() const { return PlayerModeComponent; }
    void AddItem(EInventorySlot Slot, int32 Amount = 1);
    bool UseItem(EInventorySlot Slot, int32 Amount = 1);
    UInvenComponent* GetInvenComponent() const { return InvenComponent; }
};