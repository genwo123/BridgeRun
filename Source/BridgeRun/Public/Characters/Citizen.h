// Public/Characters/Citizen.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Modes/InvenComponent.h"
#include "Modes/PlayerModeTypes.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Citizen.generated.h"


UCLASS(Blueprintable)
class BRIDGERUN_API ACitizen : public ACharacter
{
    GENERATED_BODY()

public:
    ACitizen();
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team Materials")
    UMaterialInterface* M_Team_Blue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team Materials")
    UMaterialInterface* M_Team_Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team Materials")
    UMaterialInterface* M_Team_Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team Materials")
    UMaterialInterface* M_Team_Yellow;

    // TeamID 속성 추가 (복제 필요)
    UPROPERTY(ReplicatedUsing = OnRep_TeamID)
    int32 TeamID = -1;

    UPROPERTY(ReplicatedUsing = OnRep_SkinIndex)
    int32 SkinIndex = 0;

    UFUNCTION(NetMulticast, Reliable)
    void MulticastHandleRespawn();

    // 블루프린트에서 구현 가능한 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Character")
    void ApplySkin();

    // TeamID 변경 이벤트 처리 함수
    UFUNCTION()
    void OnRep_TeamID();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastSetTeamMaterial(int32 InTeamID);

    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetTeamMaterial(int32 InTeamID);

    UFUNCTION()
    UCameraComponent* GetFollowCamera() const { return CameraComponent; }

    // 복제되어야 하는 속성
    UPROPERTY(Replicated)
    class AItem_Trophy* HeldTrophy;

    // 사망/리스폰 처리
    UFUNCTION(NetMulticast, Reliable)
    void MulticastHandleDeath();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRespawn(const FVector& RespawnLocation);

    UFUNCTION()
    bool ServerRespawn_Validate(const FVector& RespawnLocation);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(ReplicatedUsing = OnRep_IsDead)
    bool bIsDead;

    UFUNCTION()
    void OnRep_IsDead();

    // 서버 RPC
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerInteract();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSelectInventorySlot(EInventorySlot Slot);

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

    // 상호작용 헬퍼 함수들
    bool HandleHeldTrophyInteraction();
    void InteractWithNearbyItems();
    AActor* FindClosestInteractableItem(TArray<AActor*>& OutOverlappedActors);
    void ProcessItemInteraction(AActor* Item);
    void VisualizeInteractionRadius();
    void VisualizeItemConnection(AActor* Item, bool IsClosest);

    void InitializeTeamFromPlayerState();

private:
    // private 섹션으로 이동하여 블루프린트에서 보이지 않게 함
    UFUNCTION()
    void OnRep_SkinIndex();

public:
    void SelectInventorySlot(EInventorySlot Slot);
    UPlayerModeComponent* GetPlayerModeComponent() const { return PlayerModeComponent; }
    void AddItem(EInventorySlot Slot, int32 Amount = 1);
    bool UseItem(EInventorySlot Slot, int32 Amount = 1);
    UInvenComponent* GetInvenComponent() const { return InvenComponent; }
};