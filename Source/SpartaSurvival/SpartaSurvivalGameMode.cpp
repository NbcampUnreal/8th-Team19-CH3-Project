// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpartaSurvivalGameMode.h"
#include "SpartaSurvivalCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h" 
#include "Kismet/KismetSystemLibrary.h"
#include "EnemySpawnComponent.h"
#include "Engine/TargetPoint.h"
#include "SpartaSurvivalGameState.h"
#include "SpartaSurvivalPlayerController.h"
#include "BossZombie.h"
#include "Engine/World.h"
#include "EnemySpawnComponent.h"


ASpartaSurvivalGameMode::ASpartaSurvivalGameMode()
{
	// set default pawn class to our Blueprinted character
	/*static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}*/
	PrimaryActorTick.bCanEverTick = true;

	EnemySpawnComp = CreateDefaultSubobject<UEnemySpawnComponent>(TEXT("EnemySpawnComp"));


	AccumulatedSeconds = 0;
	CurrentStage = 1;
	
	//스테이지 타임 나중에 수정//일단 1분
	StageTime = 10;


}



void ASpartaSurvivalGameMode::BeginPlay()
{
	
	Super::BeginPlay();

	//메인메뉴때 게임 시작 방지
	FString CurrentLevelName = GetWorld()->GetMapName();
	CurrentLevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);


	if (CurrentLevelName.Equals(TEXT("MainMenuLevel"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogTemp, Error, TEXT("게임 시작 전!"));
		return; 
	}

	UE_LOG(LogTemp, Error, TEXT("게임 시작!"));

	if (!EnemySpawnComp)
	{
		UE_LOG(LogTemp, Error, TEXT("심각: EnemySpawnComp가 생성되지 않았습니다!"));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("성공: EnemySpawnComp를 찾았습니다!"));
		
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (Actor && Actor->ActorHasTag(FName("ZombieSpawn")))
		{
			SpawnPoints.Add(Actor);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("좀비 스폰 포인트 %d개 등록 완료!"), SpawnPoints.Num());

	EnemySpawnComp->StartWave(CurrentStage);


	GetWorld()->GetTimerManager().SetTimer(
		MainTimerHandle,
		this,
		&ThisClass::HandleMainTimerElapsed,
		1.f,
		true
	);
}

//실시간 정보 관리 (놔둿다 캐릭터에 붙히는게 좋을듯)
/*void ASpartaSurvivalGameMode::Tick(float DeltaSeconds)
{

	Super::Tick(DeltaSeconds);

	
	ASpartaSurvivalCharacter* MyChar = Cast<ASpartaSurvivalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (MyChar)
	{
		// 2. HP가 0 이하인지 체크 HP변수명맞추기
		if (MyChar->HP <= 0.0f)
		{
			GameOver();

		
			PrimaryActorTick.bCanEverTick = false;
		}
	}
}*/


//게임시작(나중에 수정)
void ASpartaSurvivalGameMode::StartGame(FName LevelName)
{
	//시작 위젯연결
	GetWorldTimerManager().SetTimer(MainTimerHandle, this, &ASpartaSurvivalGameMode::HandleMainTimerElapsed, 1.0f, true);

	/* // 적 소환 로직
		if (EnemyClass) 
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();

			// 적을 특정 위치에 소환
			AActor* SpawnedEnemy = GetWorld()->SpawnActor<AActor>(
				EnemyClass,
				SpawnLocation,
				FRotator::ZeroRotator,
				SpawnParams
			);

			if (SpawnedEnemy)
			{
				UE_LOG(LogTemp, Warning, TEXT("적 !"));
			}
		}*/

}
//캐릭터 사망
void ASpartaSurvivalGameMode::GameOver()
{
    if (GetWorld())
    {
        // 1. 플레이어 컨트롤러를 가져옵니다.
        ASpartaSurvivalPlayerController* PC = Cast<ASpartaSurvivalPlayerController>(GetWorld()->GetFirstPlayerController());

       
        if (PC)
        {
            PC->ShowGameOver();
            UE_LOG(LogTemp, Warning, TEXT("게임 오버! 플레이어가 사망했습니다."));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerController를 가져오지 못했습니다!"));
        }
    }
}
//클리어 수정중 노드로 만듬
/*void ASpartaSurvivalGameMode::GameClear(int32 inCurrentStage)
{

	if (inCurrentStage == 7)
	{
		if (GetWorld())
		{
			// 1. 플레이어 컨트롤러를 가져옵니다.
			ASpartaSurvivalPlayerController* PC = Cast<ASpartaSurvivalPlayerController>(GetWorld()->GetFirstPlayerController());

			if (PC)
			{
				PC->ShowGameClear();

			}

		}
	}
}*/


//나중에 수정
void ASpartaSurvivalGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (EndPlayReason == EEndPlayReason::Quit)
	{
		UE_LOG(LogTemp, Warning, TEXT("게임을 종료합니다."));
	}
}



//시간
void ASpartaSurvivalGameMode::HandleMainTimerElapsed()
{
	AccumulatedSeconds += 1;
	//테스트용
	if (GetWorld())
	{
		ASpartaSurvivalGameState* GS = Cast<ASpartaSurvivalGameState>(GetWorld()->GetGameState());
		if (GS)
		{

			int32 BeforeLevel = GS->PlayerLevel;
			GS->AddPlayerEXP(10.f);
			if (GS->PlayerLevel > BeforeLevel)
			{
				ASpartaSurvivalPlayerController* PC = Cast<ASpartaSurvivalPlayerController>(GetWorld()->GetFirstPlayerController());
				if (PC)
				{

					PC->ShowLevelUp();
				}
			}


		}
	}
	if (AccumulatedSeconds > 0)
	{
		
		if (EnemySpawnComp && SpawnPoints.Num() > 0)
		{
			int32 CurrentQueueCount = EnemySpawnComp->WaveSpawnQueue.Num();
			for (int32 i = 0; i < CurrentQueueCount; ++i)
			{
				int32 RandIdx = FMath::RandRange(0, SpawnPoints.Num() - 1);
				if (SpawnPoints[RandIdx] != nullptr)
				{
					FVector SpawnPos = SpawnPoints[RandIdx]->GetActorLocation();
					EnemySpawnComp->SpawnLogic(SpawnPos);
				}
			}
		}
		



		if (AccumulatedSeconds % StageTime == 0)
		{
			if (CurrentStage < 3)
			{
				CurrentStage += 1;

				UE_LOG(LogTemp, Error, TEXT("⏰ [시간 경과] %d 웨이브 전환 신호 발생!"), CurrentStage);

				// 변경된 CurrentStage 숫자를 그대로 토스
				Gamelevel(CurrentStage);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[최종 웨이브] 현재 5스테이지 보스전 유지 중 (시간: %d초)"), AccumulatedSeconds);
			}
		}
	}
}

//게임 난이도( 적스폰이랑 맞춰보겠습니다)
void ASpartaSurvivalGameMode::Gamelevel(int32 inCurrentStage)
{
	CurrentStage = inCurrentStage;

	
	if (!EnemySpawnComp) return;

	
	EnemySpawnComp->StartWave(inCurrentStage);

	UE_LOG(LogTemp, Warning, TEXT("📢 GameMode가 %d 스테이지 명령을 Component로 성공적으로 전달했습니다!"), inCurrentStage);
}




/*	if (EnemySpawnComp && EnemySpawnComp->RemainingMonsters > 0 && SpawnPoints.Num() > 0)
	{
		int32 RandIdx = FMath::RandRange(0, SpawnPoints.Num() - 1);
		FVector SpawnPos = SpawnPoints[RandIdx]->GetActorLocation();


		EnemySpawnComp->SpawnLogic(SpawnPos);
	}*/