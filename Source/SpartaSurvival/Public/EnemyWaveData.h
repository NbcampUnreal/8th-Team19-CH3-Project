#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EnemyWaveData.generated.h"

USTRUCT(BlueprintType)
struct FEnemyWaveData : public FTableRowBase
{
    GENERATED_BODY()

public:
    
    FEnemyWaveData()
        : TotalMonsterCount(10), SpawnInterval(1.5f)
    {
    }

    // 웨이브당 총 몬스터 수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    int32 TotalMonsterCount;

    // 스폰 간격
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    float SpawnInterval;

    // 스폰할 적 클래스 (AEnemyBase를 상속받은 블루프린트들)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    TSubclassOf<class AEnemyBase> EnemyClass;
};