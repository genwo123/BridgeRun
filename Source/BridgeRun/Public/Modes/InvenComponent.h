// Public/Modes/Components/InvenComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "InvenComponent.generated.h"
/**
 * 인벤토리 슬롯 타입
 * 게임에서 사용 가능한 아이템 유형을 정의합니다.
 */
UENUM(BlueprintType)
enum class EInventorySlot : uint8
{
    None = 0,     // 선택 없음
    Plank = 1,    // 판자
    Tent = 2,     // 텐트
    Telescope = 3, // 망원경
    Gun = 4,      // 총
    Trophy = 5    // 트로피
};
// 아이템 수량 변경 시 이벤트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemCountChanged, EInventorySlot, Slot, int32, NewCount);
/**
 * 아이템 데이터 구조체
 * 인벤토리 아이템의 기본 속성을 정의합니다.
 */
USTRUCT(BlueprintType)
struct FItemData
{
    GENERATED_BODY()
    // 아이템 아이콘
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* NormalIcon;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* FocusedIcon;
    // 아이템 수량
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count;
    // 기본 생성자
    FItemData() : NormalIcon(nullptr), FocusedIcon(nullptr), Count(0) {}
};
/**
 * 인벤토리 컴포넌트
 * 플레이어의 아이템 보유 상태와 선택을 관리합니다.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BRIDGERUN_API UInvenComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    // 생성자 및 기본 함수
    UInvenComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    // 이벤트 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnItemCountChanged OnItemCountChanged;
    // 현재 선택된 슬롯
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    EInventorySlot CurrentSelectedSlot;
    // 슬롯 접근 함수
    UFUNCTION(BlueprintPure, Category = "Inventory")
    EInventorySlot GetCurrentSelectedSlot() const { return CurrentSelectedSlot; }
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory")
    void SetCurrentSelectedSlot(EInventorySlot Slot);
    // 아이템 데이터
    UPROPERTY(Replicated, EditDefaultsOnly, Category = "Items")
    FItemData PlankData;
    UPROPERTY(Replicated, EditDefaultsOnly, Category = "Items")
    FItemData TentData;
    UPROPERTY(Replicated, EditDefaultsOnly, Category = "Items")
    FItemData TelescopeData;
    UPROPERTY(Replicated, EditDefaultsOnly, Category = "Items")
    FItemData GunData;
    UPROPERTY(Replicated, EditDefaultsOnly, Category = "Items")
    FItemData TrophyData;
    // 아이템 데이터 접근 함수
    FItemData* GetItemData(EInventorySlot Slot);
    UFUNCTION(BlueprintPure, Category = "Inventory")
    FItemData GetItemDataForBP(EInventorySlot Slot);
    // 아이템 수량 업데이트
    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory")
    void UpdateItemCount(EInventorySlot Slot, int32 Amount);
protected:
    // 오버라이드 함수
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};