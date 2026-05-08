// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyWaveData.h"
#include "EnemySpawnComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPARTASURVIVAL_API UEnemySpawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UEnemySpawnComponent();

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void StartWave(int32 WaveNumber);

protected:

	void SpawnLogic();


	UPROPERTY(EditAnywhere, Category = "Spawning")
	class UDataTable* WaveDataTable;

private:

	FEnemyWaveData CurrentWaveInfo;

	int32 RemainingMonsters;

	FTimerHandle SpawnTimerHandle;
		
};
