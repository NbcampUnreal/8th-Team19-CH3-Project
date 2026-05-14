
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
    if (!WaveDataTable) return;

    FString RowNameStr = FString::FromInt(WaveNumber);
    FEnemyWaveData* WaveData = WaveDataTable->FindRow<FEnemyWaveData>(FName(*RowNameStr), TEXT(""));

    if (WaveData)
    {

        WaveSpawnQueue.Empty();


        for (const FEnemySpawnInfo& Info : WaveData->EnemyList)
        {
            for (int32 i = 0; i < Info.SpawnCount; i++)
            {
                WaveSpawnQueue.Add(Info.EnemyClass);
            }
        }


        int32 LastIndex = WaveSpawnQueue.Num() - 1;
        for (int32 i = 0; i <= LastIndex; ++i)
        {
            int32 Index = FMath::RandRange(i, LastIndex);
            if (i != Index) WaveSpawnQueue.Swap(i, Index);
        }


        RemainingMonsters = WaveSpawnQueue.Num();

        UE_LOG(LogTemp, Warning, TEXT("웨이브 %d 로드 성공! 총 %d마리 스폰 예정"), WaveNumber, RemainingMonsters);
    }
}

void UEnemySpawnComponent::SpawnLogic(FVector SpawnCenter)
{

    if (WaveSpawnQueue.Num() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("스폰 대기열이 비었습니다."));
        return;
    }

    UWorld* World = GetWorld();
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!NavSys || !World) return;

    FNavLocation RandomLocation;
    if (NavSys->GetRandomReachablePointInRadius(SpawnCenter, 500.0f, RandomLocation))
    {

        TSubclassOf<AEnemyBase> TargetClass = WaveSpawnQueue.Pop();

        if (TargetClass)
        {

            FRotator SpawnRotation = FRotator::ZeroRotator;
            APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
            if (PlayerPawn) {
                FVector LookAtDir = PlayerPawn->GetActorLocation() - RandomLocation.Location;
                SpawnRotation = LookAtDir.Rotation();
                SpawnRotation.Pitch = 0.f; SpawnRotation.Roll = 0.f;
            }

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;


            AActor* SpawnedActor = World->SpawnActor<AEnemyBase>(TargetClass, RandomLocation.Location, SpawnRotation, SpawnParams);

            if (SpawnedActor)
            {
                RemainingMonsters--;
                UE_LOG(LogTemp, Log, TEXT("스폰 성공! 남은 좀비: %d"), RemainingMonsters);
            }
            else
            {

                WaveSpawnQueue.Add(TargetClass);
            }
        }
    }
}

