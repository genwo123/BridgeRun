// Copyright BridgeRun Game, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "TeamManagerComponent.generated.h"

// �� ���� ����ü
USTRUCT(BlueprintType)
struct BRIDGERUN_API FTeamInfo
{
    GENERATED_BODY()

    // �� ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 TeamID;

    // �� �̸�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    FString TeamName;

    // �� ����
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
 * ���Ӹ�忡 �߰��Ͽ� ���
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class BRIDGERUN_API UTeamManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // ������ �� �ʱ�ȭ
    UTeamManagerComponent();
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // �� ���� �ʱ�ȭ �Լ�
    UFUNCTION(BlueprintCallable, Category = "Team")
    void InitializeTeamColors();

    // �÷��̾ ���� �Ҵ��ϴ� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Team")
    void AssignPlayerToTeam(AController* PlayerController);

    // Ȱ�� �� �� ���� (2-4)
    UFUNCTION(BlueprintCallable, Category = "Team")
    void SetActiveTeamCount(int32 Count);

    // ���� Ȱ��ȭ�� �� �� ��������
    UFUNCTION(BlueprintPure, Category = "Team")
    int32 GetActiveTeamCount() const { return ActiveTeamCount; }

    // Ư�� ���� ���� ��������
    UFUNCTION(BlueprintPure, Category = "Team")
    FLinearColor GetTeamColor(int32 TeamID) const;

    // Ư�� ���� �̸� ��������
    UFUNCTION(BlueprintPure, Category = "Team")
    FString GetTeamName(int32 TeamID) const;

    // �÷��̾��� �� ID ��������
    UFUNCTION(BlueprintPure, Category = "Team")
    int32 GetPlayerTeamID(AController* PlayerController) const;

    // Ȱ��ȭ�� �� ���� ��������
    UFUNCTION(BlueprintCallable, Category = "Team")
    TArray<FTeamInfo> GetActiveTeams() const;

    // �÷��̾ ������ϴ� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Team")
    void ReallocatePlayersToTeams();

    // �÷��̾ ���� �����ϴ� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Team")
    bool RequestTeamChange(AController* PlayerController, int32 RequestedTeamID);

    // �� Ȱ��ȭ ���� ������Ʈ
    UFUNCTION(BlueprintCallable, Category = "Team")
    void UpdateTeamActiveStatus();







protected:
    // �ִ� �� ��
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team")
    int32 MaxTeams = 4;

    // Ȱ��ȭ�� �� �� (2-4)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Team", meta = (ClampMin = "2", ClampMax = "4"))
    int32 ActiveTeamCount = 2;

    // �� ���� �迭
    UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere, Category = "Team")
    TArray<FTeamInfo> TeamInfo;

    // �� Ȱ��ȭ ���� �迭
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
    TArray<bool> TeamActive;

    // �� �ο� ����
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team")
    int32 MaxPlayersPerTeam = 8;

private:
    // �÷��̾�-�� ������ ������ ����
    UPROPERTY()
    TMap<AController*, int32> PlayerTeamMap;

    // ������ �� ã�� (���� �ο��� ���� ��)
    int32 GetOptimalTeamForTeam() const;

    // �� ���� �� �ʿ��� ��� �÷��̾� ������ ó��
    void RespawnPlayerInTeam(AController* PlayerController, int32 TeamID);


    AActor* FindPlayerStartForTeam(AController* Controller, const FString& TeamTag);
};