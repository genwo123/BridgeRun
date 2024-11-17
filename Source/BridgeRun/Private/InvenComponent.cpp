// InvenComponent.cpp
#include "InvenComponent.h"

UInvenComponent::UInvenComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

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

FItemData* UInvenComponent::GetItemData(EInventorySlot Slot)
{
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
    if (FItemData* Data = GetItemData(Slot))
    {
        return *Data;
    }
    return FItemData();
}

void UInvenComponent::UpdateItemCount(EInventorySlot Slot, int32 Amount)
{
    // 함수 호출 확인
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
        FString::Printf(TEXT("UpdateItemCount Called: Slot %d, Amount %d"), (int32)Slot, Amount));

    FItemData* ItemData = GetItemData(Slot);
    if (ItemData)
    {

        ItemData->Count += Amount;
        if (ItemData->Count < 0)
        {
            ItemData->Count = 0;
        }

        // 업데이트 후 값
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
            FString::Printf(TEXT("After Update: Count = %d"), ItemData->Count));

        OnItemCountChanged.Broadcast(Slot, ItemData->Count);
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
            TEXT("ItemData is null!"));
    }
}