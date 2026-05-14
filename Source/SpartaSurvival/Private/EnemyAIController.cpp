#include "EnemyAIController.h"
#include "EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

void AEnemyAIController::OnPossess(APawn* InPawn)
{

    Super::OnPossess(InPawn);
   
    GetWorldTimerManager().SetTimer(ChaseTimerHandle, this, &AEnemyAIController::ChasePlayer, 0.1f, true);
}

void AEnemyAIController::ChasePlayer()
{
    AEnemyBase* Enemy = Cast<AEnemyBase>(GetPawn());
    if (!Enemy || Enemy->EnemyState == EEnemyState::Death)
    {
        GetWorldTimerManager().ClearTimer(ChaseTimerHandle);
        return;
    }

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerChar) return;

    if (Enemy->bIsAttacking)
    {
        return;
    }

    float Distance = Enemy->GetDistanceTo(PlayerChar);
    

    
    if (Distance <= 130.f)
    {
        StopMovement();
        HandleAttack(Enemy);
    }
    
    else
    {
        FAIMoveRequest MoveRequest(PlayerChar);
        MoveRequest.SetAcceptanceRadius(10.f); 
        MoveRequest.SetCanStrafe(true);     

        MoveTo(MoveRequest);
    }
}

void AEnemyAIController::HandleAttack(AEnemyBase* Enemy)
{
    
    if (Enemy && !Enemy->bIsAttacking)
    {
        Enemy->Attack();
       
    }
}

void AEnemyAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);
}