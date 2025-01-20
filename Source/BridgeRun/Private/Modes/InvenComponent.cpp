// Private/Modes/InvenComponent.cpp
#include "Modes/InvenComponent.h"
#include "Net/UnrealNetwork.h"

UInvenComponent::UInvenComponent()
{
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

    DOREPLIFETIME(UInvenComponent, CurrentSelectedSlot);
    DOREPLIFETIME(UInvenComponent, PlankData);
    DOREPLIFETIME(UInvenComponent, TentData);
    DOREPLIFETIME(UInvenComponent, TelescopeData);
    DOREPLIFETIME(UInvenComponent, GunData);
    DOREPLIFETIME(UInvenComponent, TrophyData);
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

void UInvenComponent::SetCurrentSelectedSlot_Implementation(EInventorySlot Slot)
{
    if (!GetOwner()->HasAuthority()) return;
    CurrentSelectedSlot = Slot;
}

void UInvenComponent::UpdateItemCount_Implementation(EInventorySlot Slot, int32 Amount)
{
    if (!GetOwner()->HasAuthority()) return;

    FItemData* ItemData = GetItemData(Slot);
    if (ItemData)
    {
        ItemData->Count += Amount;
        if (ItemData->Count < 0)
        {
            ItemData->Count = 0;
        }
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