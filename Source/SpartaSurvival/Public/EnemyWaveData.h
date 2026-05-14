#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EnemyWaveData.generated.h"


USTRUCT(BlueprintType)
struct FEnemySpawnInfo
{
    GENERATED_BODY()

    

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    TSubclassOf<class AEnemyBase> EnemyClass;

    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    int32 SpawnCount = 0;
};


USTRUCT(BlueprintType)
struct FEnemyWaveData : public FTableRowBase
{

    GENERATED_BODY()

public:
    FEnemyWaveData() : SpawnInterval(1.5f) {}

    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    float SpawnInterval;

    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    TArray<FEnemySpawnInfo> EnemyList;
};