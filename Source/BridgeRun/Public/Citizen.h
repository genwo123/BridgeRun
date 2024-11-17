// Citizen.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InvenComponent.h"  // EInventorySlot을 여기서 가져옴
#include "Citizen.generated.h"

// EInventorySlot 열거형은 제거 (InvenComponent.h에서 정의된 것 사용)

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
    // 상호작용 함수
    void Interact();

    // 상호작용 범위
    UPROPERTY(EditAnywhere, Category = "Interaction")
    float InteractionRange = 500.0f;


    // 카메라 컴포넌트 (기존 유지)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    class USpringArmComponent* SpringArmComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    class UCameraComponent* CameraComponent;

    // 인벤토리 컴포넌트 추가
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UInvenComponent* InvenComponent;

    // 기존 슬롯 선택 함수들 유지
    void SelectSlot1() { SelectInventorySlot(EInventorySlot::Plank); }
    void SelectSlot2() { SelectInventorySlot(EInventorySlot::Tent); }
    void SelectSlot3() { SelectInventorySlot(EInventorySlot::Telescope); }
    void SelectSlot4() { SelectInventorySlot(EInventorySlot::Gun); }
    void SelectSlot5() { SelectInventorySlot(EInventorySlot::Trophy); }

    // 기본 이동 함수들 (기존 유지)
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

    // 건설 시스템 (기존 유지)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    class UStaticMeshComponent* BuildPreviewMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float MaxBuildDistance = 300.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float BuildRotationStep = 15.0f;

    // 건설 함수들
    void UpdateBuildPreview();
    void RotateBuildPreview();
    void AttemptBuild();
    bool IsValidBuildLocation(const FVector& Location) const;


    // 프리뷰 머티리얼
    UPROPERTY(EditAnywhere, Category = "Building")
    UMaterialInterface* ValidPlacementMaterial;   // M_PlaceValid 할당

    UPROPERTY(EditAnywhere, Category = "Building")
    UMaterialInterface* InvalidPlacementMaterial; // M_PlaceInvalid 할당


    // UI 시스템 (기존 유지)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    class UUserWidget* InventoryWidget;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UUserWidget> InventoryWidgetClass;

public:
    // 인벤토리 함수들 (InvenComponent와 연동)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SelectInventorySlot(EInventorySlot Slot);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddItem(EInventorySlot Slot, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseItem(EInventorySlot Slot, int32 Amount = 1);
};