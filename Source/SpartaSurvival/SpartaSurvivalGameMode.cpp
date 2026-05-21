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
	
	//스테이지 타임 나중에 수정
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
//클리어 수정중
void ASpartaSurvivalGameMode::GameClear(int32 inCurrentStage)
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
}


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
			// 1. [1초마다] 체력 10 감소, 경험치 10 증가
			//GS->AddPlayerHP(-10.f);
			
			int32 BeforeLevel = GS->PlayerLevel;
			//GS->AddPlayerEXP(10.f);
			if (GS->PlayerLevel > BeforeLevel)
			{
				ASpartaSurvivalPlayerController* PC = Cast<ASpartaSurvivalPlayerController>(GetWorld()->GetFirstPlayerController());
				if (PC)
				{
					
					PC->ShowLevelUp();
				}
			}

			// 2. [3초마다] 체력 50 회복, 점수 1점 증가 (3의 배수 초 일 때)
			if (AccumulatedSeconds % 3 == 0)
			{
			//	GS->AddPlayerHP(50.f);
				GS->AddScore(1);

				UE_LOG(LogTemp, Warning, TEXT("[3초 주기] HP 50 회복 및 1점 획득! 현재 총점: %d"), GS->CurrentScore);
			}
		}
	}

	if (AccumulatedSeconds % 2 == 0)
	{
		if (EnemySpawnComp)
		{
			UE_LOG(LogTemp, Warning, TEXT("체크 - 남은 마릿수: %d, 포인트 개수: %d"),
				EnemySpawnComp->RemainingMonsters, SpawnPoints.Num());
		}
		
		if (EnemySpawnComp && EnemySpawnComp->RemainingMonsters > 0 && SpawnPoints.Num() > 0)
		{
			int32 RandIdx = FMath::RandRange(0, SpawnPoints.Num() - 1);
			FVector SpawnPos = SpawnPoints[RandIdx]->GetActorLocation();

			
			EnemySpawnComp->SpawnLogic(SpawnPos);
		}
	}

	if (AccumulatedSeconds % StageTime == 0)
	{
		CurrentStage += 1;
		Gamelevel(CurrentStage);
		GameClear(CurrentStage);
	}
}

//게임 난이도( 적스폰이랑 맞춰보겠습니다)
void ASpartaSurvivalGameMode::Gamelevel(int32 inCurrentStage)
{

	if (inCurrentStage == 9)
	{
		GetWorldTimerManager().PauseTimer(MainTimerHandle);


		//SpawnBoss();

		return;
	}
	CurrentStage = inCurrentStage;

	if (EnemySpawnComp)
	{
		
		EnemySpawnComp->StartWave(CurrentStage);

		
		int32 DifficultyMultiplier = CurrentStage;

		
		EnemySpawnComp->RemainingMonsters = EnemySpawnComp->RemainingMonsters * DifficultyMultiplier;

		UE_LOG(LogTemp, Warning, TEXT("난이도 상승! %d 스테이지 좀비 물량 자동으로 %d배 증가 완료 (총 %d마리)"),
			CurrentStage, DifficultyMultiplier, EnemySpawnComp->RemainingMonsters);
	}

}

//서브
/*int32 AmountPerSpawn = 1 + (InCurrentStage / 3);

// 3. 타이머 재설정 
GetWorldTimerManager().SetTimer(SpawnTimerHandle, [this, AmountPerSpawn]()
	{
		// 정해진 양만큼 반복 소환
		for (int32 i = 0; i < AmountPerSpawn; i++)
		{
			SpawnEnemy();
		}
	}, NewInterval, true);

UE_LOG(LogTemp, Warning, TEXT("난이도 상승! 주기: %.2f초 | 한 번에 %d마리 소환"), NewInterval, AmountPerSpawn);
}*/

/*void ASpartaSurvivalGameMode::SpawnBoss()
{
	UWorld* World = GetWorld();
	if (World)
	{
		// 스폰할 위치와 회전값 설정
		FVector SpawnLocation(0.0f, 0.0f, 100.0f); // 원하는 좌표로 변경하세요
		FRotator SpawnRotation = FRotator::ZeroRotator;

		// 강제 소환 끼임방지
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		// ABossZombie 스폰 실행
		ABossZombie* SpawnBoss = World->SpawnActor<ABossZombie>(ABossZombie::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);

		if (SpawnBoss)
		{
		//	SpawnBoss->OnBossDeath.AddDynamic(this, &ASpartaSurvivalGameMode::GameClear);
			// 스폰 성공 후 처리할 로직 (예: UI 표시, 로그 출력 등)
			UE_LOG(LogTemp, Warning, TEXT("보스 좀비가 성공적으로 호출되었습니다!"));
		}
	}
}*/

