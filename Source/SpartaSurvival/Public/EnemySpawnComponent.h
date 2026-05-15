#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyWaveData.h"
#include "EnemyBase.h"
#include "EnemySpawnComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPARTASURVIVAL_API UEnemySpawnComponent : public UActorComponent
{

    GENERATED_BODY()

public:
    UEnemySpawnComponent();


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    class UDataTable* WaveDataTable;


    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
    FEnemyWaveData CurrentWaveInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 RemainingMonsters;

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void StartWave(int32 WaveNumber);

    void SpawnLogic(FVector SpawnCenter);

private:
    FTimerHandle SpawnTimerHandle;
    TArray<TSubclassOf<AEnemyBase>> WaveSpawnQueue;
    FVector LastSpawnCenter;

};