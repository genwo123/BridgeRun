// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item/Item.h"
#include "Modes/InvenComponent.h" 
#include "Core/BridgeRunPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Item/Item_Plank.h"
#include "Item/Item_Tent.h"
#include "BuildingComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BRIDGERUN_API UBuildingComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    // 기본 함수
    UBuildingComponent();
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 네트워크 RPC
    UFUNCTION(Server, Reliable)
    void OnBuildModeEntered();
    UFUNCTION(Server, Reliable)
    void DeactivateBuildMode();
    UFUNCTION(Server, Reliable)
    void RotateBuildPreview();
    UFUNCTION(Server, Reliable)
    void AttemptBuild();
    UFUNCTION(NetMulticast, Reliable)
    void MulticastOnBuildComplete();

    // 건설 함수
    void UpdateBuildPreview();
    void StartBuildTimer(float BuildTime);
    void CancelBuild();
    bool ValidateBuildLocation(const FVector& Location);
    void FinishBuild();

    // 아이템 물리 설정 및 인벤토리 체크
    void ConfigureBuildingItemPhysics(UStaticMeshComponent* MeshComp, const FVector& Location, const FRotator& Rotation);
    void CheckInventoryAfterBuilding(AItem* BuiltItem);

protected:
    // 구현 함수
    virtual void AttemptBuild_Implementation();
    virtual void OnBuildModeEntered_Implementation();
    virtual void DeactivateBuildMode_Implementation();
    virtual void RotateBuildPreview_Implementation();
    virtual void MulticastOnBuildComplete_Implementation();

    // 컴포넌트
    UPROPERTY(ReplicatedUsing = OnRep_BuildPreviewMesh)
    class UStaticMeshComponent* BuildPreviewMesh;

    // 재질
    UPROPERTY(EditAnywhere, Category = "Building|Preview")
    UMaterialInterface* ValidPlacementMaterial;
    UPROPERTY(EditAnywhere, Category = "Building|Preview")
    UMaterialInterface* InvalidPlacementMaterial;

    // 메시
    UPROPERTY()
    UStaticMesh* PlankMesh;
    UPROPERTY()
    UStaticMesh* TentMesh;

    // 아이템 클래스
    UPROPERTY(EditDefaultsOnly, Category = "Building|Items")
    TSubclassOf<AItem_Plank> PlankClass;
    UPROPERTY(EditDefaultsOnly, Category = "Building|Items")
    TSubclassOf<AItem_Tent> TentClass;

    // 설정
    UPROPERTY(EditAnywhere, Category = "Building|Settings")
    float MaxBuildDistance = 300.0f;
    UPROPERTY(EditAnywhere, Category = "Building|Settings")
    float BuildRotationStep = 15.0f;

    // 널빤지 설정
    UPROPERTY(EditAnywhere, Category = "Building|Plank")
    float PlankPlacementDistance = 50.0f;
    UPROPERTY(EditAnywhere, Category = "Building|Plank")
    float PlankBuildTime = 2.0f;

    // 텐트 설정
    UPROPERTY(EditAnywhere, Category = "Building|Tent")
    float TentPlacementDistance = 50.0f;
    UPROPERTY(EditAnywhere, Category = "Building|Tent")
    float TentBuildTime = 2.0f;


    UPROPERTY(ReplicatedUsing = OnRep_BuildProgress, BlueprintReadOnly, Category = "Building|UI")
    float CurrentBuildProgress = 0.0f;

    UFUNCTION(BlueprintPure, Category = "Building")
    float GetCurrentBuildTime() const
    {
        return (CurrentBuildingItem == EInventorySlot::Plank) ? PlankBuildTime : TentBuildTime;
    }

    UFUNCTION(BlueprintPure, Category = "Building")
    bool IsBuilding() const { return bIsBuilding; }

    UFUNCTION(BlueprintPure, Category = "Building")
    float GetCurrentBuildProgress() const { return CurrentBuildProgress; }


    // 복제 함수 (OnRep_BuildState 등 옆에 추가)
    UFUNCTION()
    void OnRep_BuildProgress();

    // 복제 상태
    UPROPERTY(Replicated)
    EInventorySlot CurrentBuildingItem;

    // 시각 피드백
    UPROPERTY(EditAnywhere, Category = "Building|Feedback")
    class USoundCue* BuildingBlockedSound;
    UPROPERTY(EditAnywhere, Category = "Building|Feedback")
    class UParticleSystem* BuildingBlockedEffect;

private:
    // 참조
    UPROPERTY()
    class ACitizen* OwnerCitizen;
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // 건설 상태
    UPROPERTY(ReplicatedUsing = OnRep_BuildState)
    bool bCanBuildNow = true;
    UPROPERTY(ReplicatedUsing = OnRep_BuildState)
    bool bIsBuilding = false;
    UPROPERTY(ReplicatedUsing = OnRep_ValidPlacement)
    bool bIsValidPlacement = false;

    // 타이머
    FTimerHandle BuildDelayTimerHandle;
    FTimerHandle BuildTimerHandle;

    // 위치 추적 변수
    FVector PreviewLocation;
    FRotator PreviewRotation;

    // 초기화 및 설정 함수
    void InitializeBuildPreviewMesh();
    void SetupPreviewMeshForCurrentItem();
    void SetupPlankPreviewMesh();
    void SetupTentPreviewMesh();

    // 배치 검증 함수
    bool DetermineValidPlacement(FVector& PreviewLocation, FRotator& PreviewRotation);
    bool ValidatePlankZonePlacement(class ABuildableZone* Zone, FVector& PreviewLocation);
    bool ValidateTentZonePlacement(class ABuildableZone* Zone, FVector& PreviewLocation, FRotator& PreviewRotation);
    bool ValidatePlankPlacement(const FVector& Location);
    bool ValidateTentPlacement(const FVector& Location);

    // 건설 관련 함수
    void UpdatePreviewVisuals(bool bValid);
    void UpdateOwnerBuildState();
    void ResetBuildDelay();

    // 복제 함수
    UFUNCTION()
    void OnRep_BuildState();
    UFUNCTION()
    void OnRep_BuildPreviewMesh();
    UFUNCTION()
    void OnRep_ValidPlacement();

    template<class T>
    T* SpawnBuildingItem(TSubclassOf<T> ItemClass, const FVector& Location, const FRotator& Rotation)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = OwnerCitizen;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        T* SpawnedItem = GetWorld()->SpawnActor<T>(
            ItemClass,
            Location,
            Rotation,
            SpawnParams
        );

        if (SpawnedItem)
        {
            // 공통 설정
            SpawnedItem->SetReplicates(true);
            SpawnedItem->SetReplicateMovement(true);
            SpawnedItem->bIsBuiltItem = true;

            SpawnedItem->Tags.Add(TEXT("BuiltItem"));

            if (SpawnedItem->MeshComponent)
            {
                // 기본 객체에서 트랜스폼 설정 가져오기
                T* DefaultItem = Cast<T>(ItemClass.GetDefaultObject());
                if (DefaultItem && DefaultItem->MeshComponent)
                {
                    // 원본 스케일 저장
                    FVector OriginalScale = DefaultItem->MeshComponent->GetRelativeScale3D();

                    SpawnedItem->MeshComponent->SetStaticMesh(DefaultItem->MeshComponent->GetStaticMesh());
                    SpawnedItem->MeshComponent->SetWorldScale3D(OriginalScale);

                    FTransform NewTransform = DefaultItem->MeshComponent->GetRelativeTransform();
                    NewTransform.SetLocation(Location);
                    NewTransform.SetRotation(Rotation.Quaternion());
                    SpawnedItem->SetActorTransform(NewTransform);
                }

                // 🆕 물리 설정을 여기서 먼저 적용
                ConfigureBuildingItemPhysics(SpawnedItem->MeshComponent, Location, Rotation);
            }

            // 🆕 중요: OnPlaced를 반드시 호출하여 서버에서 상태 설정
            if (GetOwner()->HasAuthority())
            {
                SpawnedItem->OnPlaced();
                UE_LOG(LogTemp, Warning, TEXT("OnPlaced called for spawned item: %s"), *SpawnedItem->GetName());
            }

            // =====================================
            // 스코어보드 통계 수집
            // =====================================
            if (OwnerCitizen && OwnerCitizen->GetController())
            {
                APlayerController* PC = Cast<APlayerController>(OwnerCitizen->GetController());
                if (PC && PC->PlayerState)
                {
                    ABridgeRunPlayerState* BridgeRunPS = Cast<ABridgeRunPlayerState>(PC->PlayerState);
                    if (BridgeRunPS)
                    {
                        // 아이템 타입에 따라 다른 통계 업데이트
                        if (Cast<AItem_Plank>(SpawnedItem))
                        {
                            BridgeRunPS->ServerAddPlankBuilt();
                            UE_LOG(LogTemp, Log, TEXT("Player %s built a plank - stats updated"),
                                *OwnerCitizen->GetName());
                        }
                        else if (Cast<AItem_Tent>(SpawnedItem))
                        {
                            BridgeRunPS->ServerAddTentBuilt();
                            UE_LOG(LogTemp, Log, TEXT("Player %s built a tent - stats updated"),
                                *OwnerCitizen->GetName());
                        }
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Failed to cast PlayerState to BridgeRunPlayerState"));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("PlayerController or PlayerState is null"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("OwnerCitizen or Controller is null"));
            }

            // 인벤토리 상태 체크 (기존 코드)
            CheckInventoryAfterBuilding(SpawnedItem);

            return SpawnedItem;
        }

        return nullptr;
    }
};