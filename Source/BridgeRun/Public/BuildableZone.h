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

	// BuildableZone.h�� ��ܿ� �߰�
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Root")
	class USceneComponent* RootSceneComponent;

	// ���ö��� ������Ʈ��
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ropes|Bottom")
	class USplineComponent* LeftBottomRope;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ropes|Bottom")
	class USplineComponent* RightBottomRope;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ropes|Top")
	class USplineComponent* LeftTopRope;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ropes|Top")
	class USplineComponent* RightTopRope;

	// ���ö��� �޽� ������Ʈ �迭��
	UPROPERTY(VisibleAnywhere, Category = "Visuals")
	TArray<class USplineMeshComponent*> LeftBottomRopeMeshes;

	UPROPERTY(VisibleAnywhere, Category = "Visuals")
	TArray<class USplineMeshComponent*> RightBottomRopeMeshes;

	UPROPERTY(VisibleAnywhere, Category = "Visuals")
	TArray<class USplineMeshComponent*> LeftTopRopeMeshes;

	UPROPERTY(VisibleAnywhere, Category = "Visuals")
	TArray<class USplineMeshComponent*> RightTopRopeMeshes;

	// ���� �޽� ����
	UPROPERTY(EditAnywhere, Category = "Visuals")
	class UStaticMesh* RopeMeshAsset;

	
	// ���� �Ӽ��� �Ʒ��� �߰�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope Settings")
	FVector2D RopeScale = FVector2D(0.1f, 0.1f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope Settings")
	UMaterialInterface* RopeMaterial;

	// �� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	ETeamType TeamOwner;

	// ��ġ ��Ģ ���� ������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	float PlankWidth = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	float PlankHeight = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	float BridgeWidth = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	float MinPlankSpacing = 50.0f;

	// ��ġ ���� ���� üũ �Լ���
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool IsPlankPlacementValid(const FVector& StartPoint, const FVector& EndPoint);

	UFUNCTION(BlueprintCallable, Category = "Building")
	bool IsTentPlacementValid(const FVector& StartPoint, const FVector& EndPoint);

	// ���ö��� �޽� ������Ʈ �Լ�
	UFUNCTION(BlueprintCallable, Category = "Visuals")
	void UpdateSplineMeshes();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	// ��ƿ��Ƽ �Լ���
	float GetDistanceFromRope(const FVector& Point, class USplineComponent* Rope);
	bool IsPointNearRope(const FVector& Point, class USplineComponent* Rope, float Tolerance = 50.0f);
};