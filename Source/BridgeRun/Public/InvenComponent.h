#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "InvenComponent.generated.h"


UENUM(BlueprintType)
enum class EInventorySlot : uint8
{
    None = 0,
    Plank = 1,
    Tent = 2,
    Telescope = 3,
    Gun = 4,
    Trophy = 5
};

// ��������Ʈ ���� (UCLASS ���� ���� ��ġ)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemCountChanged, EInventorySlot, Slot, int32, NewCount);



USTRUCT(BlueprintType)
struct FItemData
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* NormalIcon;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* FocusedIcon;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count;
    FItemData() : NormalIcon(nullptr), FocusedIcon(nullptr), Count(0) {}
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BRIDGERUN_API UInvenComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInvenComponent();

    // ��������Ʈ �ν��Ͻ�
    UPROPERTY(BlueprintAssignable, Category = "Inventory")

    FOnItemCountChanged OnItemCountChanged;

    // ���� ���õ� ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    EInventorySlot CurrentSelectedSlot;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    EInventorySlot GetCurrentSelectedSlot() const { return CurrentSelectedSlot; }

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetCurrentSelectedSlot(EInventorySlot Slot) { CurrentSelectedSlot = Slot; }

    // ���� ������ ������
    UPROPERTY(EditDefaultsOnly, Category = "Items")
    FItemData PlankData;

    UPROPERTY(EditDefaultsOnly, Category = "Items")
    FItemData TentData;

    UPROPERTY(EditDefaultsOnly, Category = "Items")
    FItemData TelescopeData;

    UPROPERTY(EditDefaultsOnly, Category = "Items")
    FItemData GunData;

    UPROPERTY(EditDefaultsOnly, Category = "Items")
    FItemData TrophyData;

    // C++ ���� �Լ�
    FItemData* GetItemData(EInventorySlot Slot);

    // BP ���ٿ� �Լ�
    UFUNCTION(BlueprintPure, Category = "Inventory")
    FItemData GetItemDataForBP(EInventorySlot Slot);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UpdateItemCount(EInventorySlot Slot, int32 Amount);

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};