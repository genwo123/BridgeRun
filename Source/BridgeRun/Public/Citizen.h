// Citizen.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InvenComponent.h"  // EInventorySlot�� ���⼭ ������
#include "Citizen.generated.h"

// EInventorySlot �������� ���� (InvenComponent.h���� ���ǵ� �� ���)

USTRUCT(BlueprintType)
struct FInventoryItem
{
    GENERATED_USTRUCT_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count;
    FInventoryItem() : Count(0) {}
};

UCLASS(Blueprintable)
class BRIDGERUN_API ACitizen : public ACharacter
{
    GENERATED_BODY()

public:
    ACitizen();
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    virtual void BeginPlay() override;
    // ��ȣ�ۿ� �Լ�
    void Interact();

    // ��ȣ�ۿ� ����
    UPROPERTY(EditAnywhere, Category = "Interaction")
    float InteractionRange = 500.0f;


    // ī�޶� ������Ʈ (���� ����)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    class USpringArmComponent* SpringArmComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    class UCameraComponent* CameraComponent;

    // �κ��丮 ������Ʈ �߰�
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UInvenComponent* InvenComponent;

    // ���� ���� ���� �Լ��� ����
    void SelectSlot1() { SelectInventorySlot(EInventorySlot::Plank); }
    void SelectSlot2() { SelectInventorySlot(EInventorySlot::Tent); }
    void SelectSlot3() { SelectInventorySlot(EInventorySlot::Telescope); }
    void SelectSlot4() { SelectInventorySlot(EInventorySlot::Gun); }
    void SelectSlot5() { SelectInventorySlot(EInventorySlot::Trophy); }

    // �⺻ �̵� �Լ��� (���� ����)
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveForward(float Value);
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveRight(float Value);
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartJump();
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopJump();
    void Turn(float Value);
    void LookUp(float Value);

    // �Ǽ� �ý��� (���� ����)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    class UStaticMeshComponent* BuildPreviewMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float MaxBuildDistance = 300.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float BuildRotationStep = 15.0f;

    // �Ǽ� �Լ���
    void UpdateBuildPreview();
    void RotateBuildPreview();
    void AttemptBuild();
    bool IsValidBuildLocation(const FVector& Location) const;


    // ������ ��Ƽ����
    UPROPERTY(EditAnywhere, Category = "Building")
    UMaterialInterface* ValidPlacementMaterial;   // M_PlaceValid �Ҵ�

    UPROPERTY(EditAnywhere, Category = "Building")
    UMaterialInterface* InvalidPlacementMaterial; // M_PlaceInvalid �Ҵ�


    // UI �ý��� (���� ����)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    class UUserWidget* InventoryWidget;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UUserWidget> InventoryWidgetClass;

public:
    // �κ��丮 �Լ��� (InvenComponent�� ����)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SelectInventorySlot(EInventorySlot Slot);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddItem(EInventorySlot Slot, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseItem(EInventorySlot Slot, int32 Amount = 1);
};