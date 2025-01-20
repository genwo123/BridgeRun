// Private/Core/BridgeRunGameMode.cpp
#include "Core/BridgeRunGameMode.h"
#include "Characters/BridgeRunCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "Core/BridgeRunGameState.h"

ABridgeRunGameMode::ABridgeRunGameMode()
{
    // 네트워크 활성화
    bReplicates = true;

    // 기본 캐릭터 클래스 설정
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }

    // GameState 설정
    GameStateClass = ABridgeRunGameState::StaticClass();

    // 기본 스폰 위치 설정
    PlayerStartLocations.Add(FVector(0.0f, -200.0f, 100.0f));
    PlayerStartLocations.Add(FVector(0.0f, 200.0f, 100.0f));
    PlayerStartLocations.Add(FVector(200.0f, 0.0f, 100.0f));
    PlayerStartLocations.Add(FVector(-200.0f, 0.0f, 100.0f));
}

void ABridgeRunGameMode::BeginPlay()
{
    Super::BeginPlay();

    // 서버 설정
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
        // 팀 배정
        int32 TeamId = GetNumPlayers() % 2;  // 0 또는 1

        // 해당 팀의 NetworkPlayerStart 찾기
        AActor* StartSpot = FindPlayerStart(NewPlayer, FString::FromInt(TeamId));
        if (StartSpot)
        {
            // 기존 폰 제거
            if (NewPlayer->GetPawn())
            {
                NewPlayer->GetPawn()->Destroy();
            }

            // 새 위치에 스폰
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