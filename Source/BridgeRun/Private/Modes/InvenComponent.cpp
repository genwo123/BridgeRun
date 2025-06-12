// Private/Modes/InvenComponent.cpp
#include "Modes/InvenComponent.h"
#include "Net/UnrealNetwork.h"
UInvenComponent::UInvenComponent()
{
    // 기본 설정
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
    // 현재 선택된 슬롯 초기화
    CurrentSelectedSlot = EInventorySlot::None;
    // 아이템 데이터 초기화
    PlankData = FItemData();
    TentData = FItemData();
    TelescopeData = FItemData();
    GunData = FItemData();
    TrophyData = FItemData();
}
void UInvenComponent::BeginPlay()
{
    Super::BeginPlay();
}
void UInvenComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
void UInvenComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // 모든 인벤토리 변수 복제
    DOREPLIFETIME(UInvenComponent, CurrentSelectedSlot);
    DOREPLIFETIME(UInvenComponent, PlankData);
    DOREPLIFETIME(UInvenComponent, TentData);
    DOREPLIFETIME(UInvenComponent, TelescopeData);
    DOREPLIFETIME(UInvenComponent, GunData);
    DOREPLIFETIME(UInvenComponent, TrophyData);
}
FItemData* UInvenComponent::GetItemData(EInventorySlot Slot)
{
    // 슬롯에 해당하는 아이템 데이터 반환
    switch (Slot)
    {
    case EInventorySlot::Plank:
        return &PlankData;
    case EInventorySlot::Tent:
        return &TentData;
    case EInventorySlot::Telescope:
        return &TelescopeData;
    case EInventorySlot::Gun:
        return &GunData;
    case EInventorySlot::Trophy:
        return &TrophyData;
    default:
        return nullptr;
    }
}
FItemData UInvenComponent::GetItemDataForBP(EInventorySlot Slot)
{
    // 블루프린트용 아이템 데이터 반환
    if (FItemData* Data = GetItemData(Slot))
    {
        return *Data;
    }
    return FItemData();
}
void UInvenComponent::SetCurrentSelectedSlot_Implementation(EInventorySlot Slot)
{
    // 서버에서만 실행
    if (!GetOwner()->HasAuthority()) return;
    // 현재 선택된 슬롯 업데이트
    CurrentSelectedSlot = Slot;
}
void UInvenComponent::UpdateItemCount_Implementation(EInventorySlot Slot, int32 Amount)
{
    // 서버에서만 실행
    if (!GetOwner()->HasAuthority()) return;
    // 대상 아이템 데이터 찾기
    FItemData* ItemData = GetItemData(Slot);
    if (ItemData)
    {
        // 수량 업데이트
        ItemData->Count += Amount;
        // 음수 방지
        if (ItemData->Count < 0)
        {
            ItemData->Count = 0;
        }
        // 디버그 메시지
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
            FString::Printf(TEXT("After Update: Count = %d"), ItemData->Count));
        // 이벤트 발생
        OnItemCountChanged.Broadcast(Slot, ItemData->Count);
    }
    else
    {
        // 데이터 없음 경고
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
            TEXT("ItemData is null!"));
    }
}