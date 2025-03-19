// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeathVolume.generated.h"

// 전방 선언
class UBoxComponent;
class ACitizen;

/**
 * 낙사 처리를 위한 볼륨 클래스
 * 맵 아래에 배치하여 캐릭터나 아이템이 맵 밖으로 떨어지는 것 감지
 */
UCLASS()
class BRIDGERUN_API ADeathVolume : public AActor
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    ADeathVolume();

protected:
    /** 게임 시작시 초기화 */
    virtual void BeginPlay() override;

    /** 충돌 감지용 박스 컴포넌트 */
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UBoxComponent* DeathBox;

    /** 오버랩 이벤트 처리 함수 */
    UFUNCTION()
    void HandleOverlap(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    /** 플레이어 리스폰 대기 시간 (초) */
    UPROPERTY(EditAnywhere, Category = "Respawn Settings")
    float PlayerRespawnDelay = 5.0f;

    /** 트로피 리스폰 시 사용할 기본 높이 */
    UPROPERTY(EditAnywhere, Category = "Respawn Settings")
    float TrophyRespawnHeight = 500.0f;

    /** 트로피 리스폰 시 안전한 위치 탐색 반경 */
    UPROPERTY(EditAnywhere, Category = "Respawn Settings")
    float SafeRespawnRadius = 300.0f;

    /** 타겟 포인트가 없을 경우 사용할 기본 리스폰 위치 */
    UPROPERTY(EditAnywhere, Category = "Respawn Settings")
    FVector DefaultRespawnLocation = FVector(-970.0f, -146.44342f, 222.000671f);

private:
    /** 낙사한 위치 근처에서 안전한 위치 찾기 */
    FVector FindSafeLocationNearby(const FVector& SearchOrigin) const;

    /** 플레이어 리스폰 위치 찾기 */
    FVector GetRespawnLocation() const;

    /** 트로피 리스폰 위치 찾기 */
    FVector GetTrophyRespawnLocation(const FVector& DeathLocation) const;

    /** 트로피를 든 시민 캐릭터 처리 */
    void HandleCitizenWithTrophy(ACitizen* Citizen);

    /** 타이머 기반 리스폰 처리 */
    void ScheduleRespawn(ACitizen* Citizen);
};