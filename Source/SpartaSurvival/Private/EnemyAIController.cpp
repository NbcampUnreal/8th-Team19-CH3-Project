#include "EnemyAIController.h"
#include "EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ChasePlayer();
}

void AEnemyAIController::ChasePlayer()
{
	
	AEnemyBase* Enemy = Cast<AEnemyBase>(GetPawn());
	if (!Enemy || Enemy->EnemyState == EEnemyState::Death || Enemy->bIsAttacking) return;

	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerChar)
	{
		
		EPathFollowingRequestResult::Type MoveResult = MoveToActor(PlayerChar, 100.f);

		
		if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			HandleAttack(Enemy);
		}
	}
}

void AEnemyAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	AEnemyBase* Enemy = Cast<AEnemyBase>(GetPawn());
	if (!Enemy || Enemy->EnemyState == EEnemyState::Death) return;

	
	if (Result.IsSuccess())
	{
		HandleAttack(Enemy);
	}
	else
	{
		
		GetWorldTimerManager().SetTimer(ChaseTimerHandle, this, &AEnemyAIController::ChasePlayer, 0.2f, false);
	}
}


void AEnemyAIController::HandleAttack(AEnemyBase* Enemy)
{
	if (Enemy && !Enemy->bIsAttacking)
	{
		Enemy->Attack();

		
		GetWorldTimerManager().SetTimer(ChaseTimerHandle, this, &AEnemyAIController::ChasePlayer, 1.5f, false);
	}
}