// Private/Core/BridgeRunGameMode.cpp
#include "Core/BridgeRunGameMode.h"
#include "Characters/BridgeRunCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "Core/BridgeRunGameState.h"

ABridgeRunGameMode::ABridgeRunGameMode()
{
    // ��Ʈ��ũ Ȱ��ȭ
    bReplicates = true;

    // �⺻ ĳ���� Ŭ���� ����
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }

    // GameState ����
    GameStateClass = ABridgeRunGameState::StaticClass();

    // �⺻ ���� ��ġ ����
    PlayerStartLocations.Add(FVector(0.0f, -200.0f, 100.0f));
    PlayerStartLocations.Add(FVector(0.0f, 200.0f, 100.0f));
    PlayerStartLocations.Add(FVector(200.0f, 0.0f, 100.0f));
    PlayerStartLocations.Add(FVector(-200.0f, 0.0f, 100.0f));
}

void ABridgeRunGameMode::BeginPlay()
{
    Super::BeginPlay();

    // ���� ����
    if (GetWorld()->GetNetMode() == NM_ListenServer)
    {
        GetWorld()->GetAuthGameMode()->bUseSeamlessTravel = true;
    }
}

void ABridgeRunGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (NewPlayer)
    {
        // �� ����
        int32 TeamId = GetNumPlayers() % 2;  // 0 �Ǵ� 1

        // �ش� ���� NetworkPlayerStart ã��
        AActor* StartSpot = FindPlayerStart(NewPlayer, FString::FromInt(TeamId));
        if (StartSpot)
        {
            // ���� �� ����
            if (NewPlayer->GetPawn())
            {
                NewPlayer->GetPawn()->Destroy();
            }

            // �� ��ġ�� ����
            FRotator StartRotation = StartSpot->GetActorRotation();
            FVector StartLocation = StartSpot->GetActorLocation();

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            APawn* NewPawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, StartLocation, StartRotation, SpawnParams);
            if (NewPawn)
            {
                NewPlayer->Possess(NewPawn);
            }
        }
    }
}