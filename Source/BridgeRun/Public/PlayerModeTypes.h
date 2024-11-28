// PlayerModeTypes.h

#pragma once
#include "CoreMinimal.h"
#include "PlayerModeTypes.generated.h"

UENUM(BlueprintType)
enum class EPlayerMode : uint8
{
    Normal  UMETA(DisplayName = "Normal Mode"),
    Build   UMETA(DisplayName = "Build Mode"),
    Combat  UMETA(DisplayName = "Combat Mode")
};
