// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "EnemyWaveData.h"
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpartaSurvivalGameMode.generated.h"

UCLASS()
class ASpartaSurvivalGameMode : public AGameModeBase
{

	GENERATED_BODY()

public:
	ASpartaSurvivalGameMode();

public:
	UFUNCTION(BlueprintCallable, Category = "GameSystem")
	void StartGame(FName LevelName);

	UFUNCTION(BlueprintCallable, Category = "GameSystem")
	void GameOver();

	//UFUNCTION(BlueprintCallable, Category = "GameSystem")
	//void GameClear(int32 inCurrentStage);

	//점수
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	//int32 CurrentScore = 0;

	//UFUNCTION(BlueprintCallable, Category = "GameSystem")
	//void AddScore(int32 Amount);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	class UEnemySpawnComponent* EnemySpawnComp;

	UPROPERTY()
	TArray<AActor*> SpawnPoints;

protected:

	virtual void BeginPlay() override;

	//virtual void Tick(float DeltaSeconds) override;

	//난이도 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	UDataTable* EnemyWaveTable;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void Gamelevel(int32 CurrentStage);

public:
	
//시간관련	
	UFUNCTION(BlueprintCallable, Category = "GameSystem")
	void HandleMainTimerElapsed();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameSystem")
    int32 AccumulatedSeconds;

      UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameSystem")
      FTimerHandle MainTimerHandle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameSystem")
	int32 StageTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameSystem")
	int32 CurrentStage;

};



