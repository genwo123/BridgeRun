// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TeamManagerComponent.generated.h"

// �� ���� ����ü
USTRUCT(BlueprintType)
struct BRIDGERUN_API FTeamInfo
{
    GENERATED_BODY()

    // �� �⺻ ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 TeamID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    FString TeamName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    FLinearColor TeamColor;

    // ���� �� �ο� ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 PlayerCount;

    FTeamInfo()
        : TeamID(0)
        , TeamName(TEXT(""))
        , TeamColor(FLinearColor::White)
        , PlayerCount(0)
    {
    }
};

/**
 * �� ������ ����ϴ� ������Ʈ
 * ����: �ڵ� �� ���� ���� ��� (�ӽ�)
 * ����: �κ� �ý��۰� �����Ͽ� �÷��̾ �� ����
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class BRIDGERUN_API UTeamManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // �⺻ �Լ�
    UTeamManagerComponent();
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // �� �ʱ�ȭ �� ���� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Team")
    void InitializeTeamColors();

    UFUNCTION(BlueprintCallable, Category = "Team")
    void UpdateTeamActiveStatus();

    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetActiveTeamCount(int32 Count);

    // �� ���� ���� �Լ�
    UFUNCTION(BlueprintPure, Category = "Team")
    int32 GetActiveTeamCount() const { return ActiveTeamCount; }

    UFUNCTION(BlueprintPure, Category = "Team")
    FLinearColor GetTeamColor(int32 TeamID) const;

    UFUNCTION(BlueprintPure, Category = "Team")
    FString GetTeamName(int32 TeamID) const;

    UFUNCTION(BlueprintCallable, Category = "Team")
    TArray<FTeamInfo> GetActiveTeams() const;

    // �÷��̾� �� ���� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Team")
    void AssignPlayerToTeam(AController* PlayerController);

    UFUNCTION(BlueprintPure, Category = "Team")
    int32 GetPlayerTeamID(AController* PlayerController) const;

    UFUNCTION(BlueprintCallable, Category = "Team|Future")
    void ReallocatePlayersToTeams();

    // ���� �κ� �ý��ۿ� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Team|Future")
    bool RequestTeamChange(AController* PlayerController, int32 RequestedTeamID);

protected:
    // �� ����
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team")
    int32 MaxTeams = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Team", meta = (ClampMin = "2", ClampMax = "4"))
    int32 ActiveTeamCount = 4;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team")
    int32 MaxPlayersPerTeam = 3;

    // �� ������
    UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere, Category = "Team")
    TArray<FTeamInfo> TeamInfo;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
    TArray<bool> TeamActive;

private:
    // �÷��̾�-�� ����
    UPROPERTY()
    TMap<AController*, int32> PlayerTeamMap;

    // ���� ��ƿ��Ƽ �Լ�
    int32 GetOptimalTeamForTeam() const; // ���� �Լ� (4�� ��� ���)

    // �� ���� �߰�: Ȱ��ȭ�� ���� ����ϴ� �Լ� ��
    int32 GetOptimalActiveTeam() const;

    void RespawnPlayerInTeam(AController* PlayerController, int32 TeamID);
    AActor* FindPlayerStartForTeam(AController* Controller, const FString& TeamTag);
};