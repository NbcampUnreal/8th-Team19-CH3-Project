#pragma once

#include "EnemyBase.h"
#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "EnemyAIController.generated.h"

UCLASS()
class SPARTASURVIVAL_API AEnemyAIController : public AAIController
{

	GENERATED_BODY()

protected:
	virtual void OnPossess(APawn* InPawn) override;

	
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

public:
	
	void ChasePlayer();
	void HandleAttack(AEnemyBase* Enemy);

private:
	FTimerHandle ChaseTimerHandle;
};