
#include "EnemySpawnComponent.h"
#include "NavigationSystem.h" 
#include "EnemyBase.h"       
#include "TimerManager.h"
#include "Engine/DataTable.h"


UEnemySpawnComponent::UEnemySpawnComponent()
{

	PrimaryComponentTick.bCanEverTick = false;


}

void UEnemySpawnComponent::StartWave(int32 WaveNumber)
{
	UE_LOG(LogTemp, Warning, TEXT("StartWave Called!"));
	if (!WaveDataTable) return;

	// 1. 데이터 테이블에서 웨이브 정보 가져오기
	FString RowName = FString::FromInt(WaveNumber);
	FEnemyWaveData* WaveData = WaveDataTable->FindRow<FEnemyWaveData>(FName(*RowName), TEXT(""));

	if (WaveData)
	{
		CurrentWaveInfo = *WaveData;
		RemainingMonsters = CurrentWaveInfo.TotalMonsterCount;

		// 2. 타이머 시작 (설정된 간격마다 SpawnLogic 실행)
		GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &UEnemySpawnComponent::SpawnLogic, CurrentWaveInfo.SpawnInterval, true);
	}
}

void UEnemySpawnComponent::SpawnLogic()
{
    UE_LOG(LogTemp, Warning, TEXT("SpawnLogic Tick - 남은 마리수: %d"), RemainingMonsters);

    if (RemainingMonsters <= 0)
    {
        GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
        return;
    }

    UWorld* World = GetWorld();
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);

    if (!NavSys)
    {
        UE_LOG(LogTemp, Error, TEXT("!!! NavSys를 찾을 수 없음 !!! (Build.cs 확인 필요)"));
        return;
    }

    FVector Center = GetOwner()->GetActorLocation();
    UE_LOG(LogTemp, Error, TEXT("현재 탐색 중심점: %s"), *Center.ToString());
    FNavLocation RandomLocation;

    
    if (NavSys->GetRandomReachablePointInRadius(Center, 10000.0f, RandomLocation))
    {
        if (CurrentWaveInfo.EnemyClass)
        {
            AActor* SpawnedActor = World->SpawnActor<AEnemyBase>(
                CurrentWaveInfo.EnemyClass,
                RandomLocation.Location,
                FRotator::ZeroRotator
            );

            if (SpawnedActor)
            {
                UE_LOG(LogTemp, Log, TEXT("좀비 스폰 성공! 위치: %s"), *RandomLocation.Location.ToString());
                RemainingMonsters--;
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("SpawnActor가 NULL을 반환함 (충돌 문제일 가능성)"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("EnemyClass가 비어있음 (데이터 테이블 확인!)"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("내비게이션 위치 찾기 실패 (P 눌러서 초록색인지 확인, Center 위치 확인)"));
    }
}


