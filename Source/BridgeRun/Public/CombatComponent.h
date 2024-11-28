// CombatComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BRIDGERUN_API UCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatComponent();
    virtual void BeginPlay() override;  // virtual Ű���� �߰�

    void OnCombatModeEntered();
};