// Copyright Epic Games, Inc. All Rights Reserved.

#include "BridgeRunGameMode.h"
#include "BridgeRunCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABridgeRunGameMode::ABridgeRunGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
