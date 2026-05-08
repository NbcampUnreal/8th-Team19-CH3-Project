
#include "EnemySpawnComponent.h"
#include "NavigationSystem.h" 
#include "EnemyBase.h"       
#include "TimerManager.h"
#include "Engine/DataTable.h"



UEnemySpawnComponent::UEnemySpawnComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

    static ConstructorHelpers::FObjectFinder<UDataTable> DataTableAsset(TEXT("/Script/Engine.DataTable'/Game/Enemy/DataTable/DT_EnemyWaveData.DT_EnemyWaveData'"));

    if (DataTableAsset.Succeeded())
    {
        WaveDataTable = DataTableAsset.Object;
    }
      
}

void UEnemySpawnComponent::StartWave(int32 WaveNumber)
{
    // 1. 함수 호출 확인
    UE_LOG(LogTemp, Error, TEXT("=== StartWave 시작! 넘겨받은 숫자: %d ==="), WaveNumber);

    if (!WaveDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("에러: 데이터 테이블 에셋이 연결되지 않음!"));
        return;
    }

    
    FString RowNameStr = FString::FromInt(WaveNumber);
    FName RowName = FName(*RowNameStr);
    UE_LOG(LogTemp, Warning, TEXT("찾으려는 행 이름: [%s]"), *RowNameStr);

    
    FEnemyWaveData* WaveData = WaveDataTable->FindRow<FEnemyWaveData>(RowName, TEXT(""));

    if (WaveData)
    {
        CurrentWaveInfo = *WaveData;
        RemainingMonsters = CurrentWaveInfo.TotalMonsterCount;
        UE_LOG(LogTemp, Warning, TEXT("★★★ 데이터 로드 성공! 마릿수: %d ★★★"), RemainingMonsters);
    }
    else
    {
   
        UE_LOG(LogTemp, Error, TEXT("실패: '%s' 행을 못 찾음. 테이블의 실제 행 이름들을 확인하세요!"), *RowNameStr);

        TArray<FName> AllRowNames = WaveDataTable->GetRowNames();
        for (FName Name : AllRowNames)
        {
            UE_LOG(LogTemp, Log, TEXT("테이블에 존재하는 행 이름: [%s]"), *Name.ToString());
        }
    }
}

void UEnemySpawnComponent::SpawnLogic(FVector SpawnCenter)
{
    // 1. 남은 마리수 체크
    if (RemainingMonsters <= 0)
    {
        
        UE_LOG(LogTemp, Warning, TEXT("이번 웨이브 좀비 모두 소환 완료!"));
        return;
    }

    UWorld* World = GetWorld();
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);

    if (!NavSys || !World)
    {
        UE_LOG(LogTemp, Error, TEXT("!!! 시스템(Nav/World) 오류 !!!"));
        return;
    }

    
    FNavLocation RandomLocation;

    
    if (NavSys->GetRandomReachablePointInRadius(SpawnCenter, 500.0f, RandomLocation))
    {
        if (CurrentWaveInfo.EnemyClass)
        {
            
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            AActor* SpawnedActor = World->SpawnActor<AEnemyBase>(
                CurrentWaveInfo.EnemyClass,
                RandomLocation.Location,
                FRotator::ZeroRotator,
                SpawnParams
            );

            if (SpawnedActor)
            {
                UE_LOG(LogTemp, Log, TEXT("타겟 포인트 근처 스폰 성공: %s"), *RandomLocation.Location.ToString());
                RemainingMonsters--;
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("SpawnActor 실패 (공간 부족 가능성)"));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("내비게이션 위치 찾기 실패 (타겟 포인트 위치 확인!)"));
    }
}


