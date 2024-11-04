// BuildableZone.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildableZone.generated.h"

UENUM(BlueprintType)
enum class ETeamType : uint8
{
	Team1 UMETA(DisplayName = "Team 1"),
	Team2 UMETA(DisplayName = "Team 2"),
	Team3 UMETA(DisplayName = "Team 3")
};

UCLASS()
class BRIDGERUN_API ABuildableZone : public AActor
{
	GENERATED_BODY()

public:
	ABuildableZone();

	// BuildableZone.h의 상단에 추가
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Root")
	class USceneComponent* RootSceneComponent;

	// 스플라인 컴포넌트들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ropes|Bottom")
	class USplineComponent* LeftBottomRope;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ropes|Bottom")
	class USplineComponent* RightBottomRope;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ropes|Top")
	class USplineComponent* LeftTopRope;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ropes|Top")
	class USplineComponent* RightTopRope;

	// 스플라인 메시 컴포넌트 배열들
	UPROPERTY(VisibleAnywhere, Category = "Visuals")
	TArray<class USplineMeshComponent*> LeftBottomRopeMeshes;

	UPROPERTY(VisibleAnywhere, Category = "Visuals")
	TArray<class USplineMeshComponent*> RightBottomRopeMeshes;

	UPROPERTY(VisibleAnywhere, Category = "Visuals")
	TArray<class USplineMeshComponent*> LeftTopRopeMeshes;

	UPROPERTY(VisibleAnywhere, Category = "Visuals")
	TArray<class USplineMeshComponent*> RightTopRopeMeshes;

	// 로프 메시 에셋
	UPROPERTY(EditAnywhere, Category = "Visuals")
	class UStaticMesh* RopeMeshAsset;

	
	// 기존 속성들 아래에 추가
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope Settings")
	FVector2D RopeScale = FVector2D(0.1f, 0.1f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope Settings")
	UMaterialInterface* RopeMaterial;

	// 팀 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	ETeamType TeamOwner;

	// 설치 규칙 관련 변수들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	float PlankWidth = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	float PlankHeight = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	float BridgeWidth = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	float MinPlankSpacing = 50.0f;

	// 설치 가능 여부 체크 함수들
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool IsPlankPlacementValid(const FVector& StartPoint, const FVector& EndPoint);

	UFUNCTION(BlueprintCallable, Category = "Building")
	bool IsTentPlacementValid(const FVector& StartPoint, const FVector& EndPoint);

	// 스플라인 메시 업데이트 함수
	UFUNCTION(BlueprintCallable, Category = "Visuals")
	void UpdateSplineMeshes();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	// 유틸리티 함수들
	float GetDistanceFromRope(const FVector& Point, class USplineComponent* Rope);
	bool IsPointNearRope(const FVector& Point, class USplineComponent* Rope, float Tolerance = 50.0f);
};