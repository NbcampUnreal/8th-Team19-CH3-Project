// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

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

	UFUNCTION(BlueprintCallable, Category = "GameSystem")
	void GameClear(int32 inCurrentStage);

	//점수
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	//int32 CurrentScore = 0;

	//UFUNCTION(BlueprintCallable, Category = "GameSystem")
	//void AddScore(int32 Amount);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	class UEnemySpawnComponent* EnemySpawnComp;

	UPROPERTY()
	TArray<AActor*> SpawnPoints;


	/*// 보스가 등장할 특정 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelDesign|Boss")
	FVector BossSpawnLocation;*/

	//void SpawnBoss();

protected:

	virtual void BeginPlay() override;

	//virtual void Tick(float DeltaSeconds) override;

	// --- UI 관련 변수 (나중에 UI 담당자가 사용할 슬롯) ---
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	//TSubclassOf<class UUserWidget> GameOverWidgetClass;

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



