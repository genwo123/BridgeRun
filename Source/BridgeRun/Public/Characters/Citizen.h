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

    // TeamID �Ӽ� �߰� (���� �ʿ�)
    UPROPERTY(ReplicatedUsing = OnRep_TeamID)
    int32 TeamID = -1;

    UPROPERTY(ReplicatedUsing = OnRep_SkinIndex)
    int32 SkinIndex = 0;

    UFUNCTION(NetMulticast, Reliable)
    void MulticastHandleRespawn();

    // �������Ʈ���� ���� ������ �̺�Ʈ
    UFUNCTION(BlueprintImplementableEvent, Category = "Character")
    void ApplySkin();

    // TeamID ���� �̺�Ʈ ó�� �Լ�
    UFUNCTION()
    void OnRep_TeamID();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastSetTeamMaterial(int32 InTeamID);

    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetTeamMaterial(int32 InTeamID);

    UFUNCTION()
    UCameraComponent* GetFollowCamera() const { return CameraComponent; }

    // �����Ǿ�� �ϴ� �Ӽ�
    UPROPERTY(Replicated)
    class AItem_Trophy* HeldTrophy;

    // ���/������ ó��
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

    // ���� RPC
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerInteract();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSelectInventorySlot(EInventorySlot Slot);

    // �⺻ ������Ʈ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USpringArmComponent* SpringArmComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCameraComponent* CameraComponent;

    // ��ɺ� ������Ʈ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UInvenComponent* InvenComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPlayerModeComponent* PlayerModeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBuildingComponent* BuildingComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCombatComponent* CombatComponent;

    // ��ȣ�ۿ�
    UPROPERTY(EditAnywhere, Category = "Interaction")
    float InteractionRange = 100.0f;

    // UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    class UUserWidget* InventoryWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UUserWidget> InventoryWidgetClass;

    // ��� ����
    UFUNCTION()
    void OnPlayerModeChanged(EPlayerMode NewMode, EPlayerMode OldMode);

    // �Է� ó��
    void MoveForward(float Value);
    void MoveRight(float Value);
    void StartJump();
    void StopJump();
    void Turn(float Value);
    void LookUp(float Value);
    void Interact();

    // ���� ����
    void SelectSlot1() { SelectInventorySlot(EInventorySlot::Plank); }
    void SelectSlot2() { SelectInventorySlot(EInventorySlot::Tent); }
    void SelectSlot3() { SelectInventorySlot(EInventorySlot::Telescope); }
    void SelectSlot4() { SelectInventorySlot(EInventorySlot::Gun); }
    void SelectSlot5() { SelectInventorySlot(EInventorySlot::Trophy); }

    // ��ȣ�ۿ� ���� �Լ���
    bool HandleHeldTrophyInteraction();
    void InteractWithNearbyItems();
    AActor* FindClosestInteractableItem(TArray<AActor*>& OutOverlappedActors);
    void ProcessItemInteraction(AActor* Item);
    void VisualizeInteractionRadius();
    void VisualizeItemConnection(AActor* Item, bool IsClosest);

    void InitializeTeamFromPlayerState();

private:
    // private �������� �̵��Ͽ� �������Ʈ���� ������ �ʰ� ��
    UFUNCTION()
    void OnRep_SkinIndex();

public:
    void SelectInventorySlot(EInventorySlot Slot);
    UPlayerModeComponent* GetPlayerModeComponent() const { return PlayerModeComponent; }
    void AddItem(EInventorySlot Slot, int32 Amount = 1);
    bool UseItem(EInventorySlot Slot, int32 Amount = 1);
    UInvenComponent* GetInvenComponent() const { return InvenComponent; }
};