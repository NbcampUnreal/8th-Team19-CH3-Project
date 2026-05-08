// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpartaSurvivalGameMode.h"
#include "SpartaSurvivalCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h" 
#include "Kismet/KismetSystemLibrary.h"


ASpartaSurvivalGameMode::ASpartaSurvivalGameMode()
{
	// set default pawn class to our Blueprinted character
	/*static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}*/
	PrimaryActorTick.bCanEverTick = true;

	AccumulatedSeconds = 0;
	CurrentStage = 1;
	
	//스테이지 타임 나중에 수정
	StageTime = 10;


}



void ASpartaSurvivalGameMode::BeginPlay()
{
	
	Super::BeginPlay();
	UE_LOG(LogTemp, Error, TEXT("게임 시작!"));
	//메인 메뉴 위젯
	//ShowMainMenu();

	//위젯전시간 체크
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
	GetWorldTimerManager().PauseTimer(MainTimerHandle);


	/*APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC){
		PC->bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	// 위젯추가(InputMode.SetWidgetToFocus(YourWidget))
	PC->SetInputMode(InputMode);
}*/
//게임일시정지
//UGameplayStatics::SetGamePaused(GetWorld(), true);
	
}
//점수
void ASpartaSurvivalGameMode::AddScore(int32 Amount)
{
	CurrentScore += Amount;
	
	UE_LOG(LogTemp, Warning, TEXT("점수 획득: +%d | 현재 총점: %d"), Amount, CurrentScore);
	//적 코드에 넣어서 실행이 깔끔할듯 안되면 모드에서 다시처리
	/*ASpartaSurvivalGameMode* GM = Cast<ASpartaSurvivalGameMode>(GetWorld()->GetAuthGameMode());
if (GM)
{
    GM->AddScore(100); // 100점 추가
}*/

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

	AddScore(10);

	UE_LOG(LogTemp, Warning, TEXT("Main timer elapsed... [Stage-%02d][%03d초]"), CurrentStage, AccumulatedSeconds);

	if (AccumulatedSeconds % StageTime == 0)
	{
		CurrentStage += 1;
		Gamelevel(CurrentStage);

		UE_LOG(LogTemp, Warning, TEXT("Stage-%02d started!"), CurrentStage);
	}
}

//게임 난이도( 적스폰이랑 맞춰보겠습니다)
void ASpartaSurvivalGameMode::Gamelevel(int32 inCurrentStage)
{

	if (inCurrentStage == 3)
	{
		GetWorldTimerManager().PauseTimer(MainTimerHandle);


		SpawnBoss();

		return;
	}
	/*//소환주기 단축
	float NewInterval = FMath::Max(0.5f, 2.0f - (CurrentStage * 0.2f));
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ASpartaSurvivalGameMode::SpawnEnemy, NewInterval, true);
	
	//소환량 증가
	int32 SpawnCount = 3 + (CurrentStage * 2);

	for (int32 i = 0; i < SpawnCount; i++)
	{
		SpawnEnemy();
	}*/

	UE_LOG(LogTemp, Warning, TEXT("난이도 상승!"));

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

void ASpartaSurvivalGameMode::SpawnBoss()
{
	// 아무 기능 없이 로그만 찍어서 호출 확인!
	UE_LOG(LogTemp, Error, TEXT("보스 소환 함수가 정상적으로 호출되었습니다!"));
}