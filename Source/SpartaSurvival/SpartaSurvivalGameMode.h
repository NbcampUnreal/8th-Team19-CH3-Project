// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpartaSurvivalGameMode.generated.h"

UCLASS(minimalapi)
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

	//점수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 CurrentScore = 0;

	UFUNCTION(BlueprintCallable, Category = "GameSystem")
	void AddScore(int32 Amount);

	//	몬스터 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<AActor> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	FVector SpawnLocation;


	/*// 보스가 등장할 특정 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelDesign|Boss")
	FVector BossSpawnLocation;*/

	void SpawnBoss();

protected:

	virtual void BeginPlay() override;

	//virtual void Tick(float DeltaSeconds) override;

	// --- UI 관련 변수 (나중에 UI 담당자가 사용할 슬롯) ---
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	//TSubclassOf<class UUserWidget> GameOverWidgetClass;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void Gamelevel(int32 CurrentStage);

private:
	
//시간관련	
void HandleMainTimerElapsed();
	int32 AccumulatedSeconds;
	FTimerHandle MainTimerHandle;
	int32 StageTime;
	
	int32 CurrentStage;

};



