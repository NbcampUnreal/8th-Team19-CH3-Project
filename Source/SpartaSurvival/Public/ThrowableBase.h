//throwablebase h 

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "ThrowableBase.generated.h"

class ASpartaSurvivalCharacter;
class UProjectileMovementComponent;
class StaticMeshCompoent;
class USceneComponent;

UCLASS()
class SPARTASURVIVAL_API AThrowableBase : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throwable")
	USceneComponent* ThrowableRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Throwable")
	UStaticMeshComponent* ThrowableMesh;

	float DamageRadius;
	float ThrowForce;
	float ProjectileRadius; //발사체의 반지름
	float ProjectileLength; //포물선을 몇초 앞까지 보여줄지
	float ProjectileFrequency; // 포물선의 밀도정도

protected:
	bool bIsChargingThrow = false;
	float ThrowChargeTime = 0.f;
	float MaxChargeTime = 2.f;
	float MinThrowPower = 600.f;
	float MaxThrowPower = 1600.f;


public:

	AThrowableBase();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY()
	AThrowableBase* SpawnedThrowable = nullptr;
	UPROPERTY()
	AThrowableBase* EquippedThrowable = nullptr;

	virtual void Throw(bool bReadyToThrow);
	virtual void ThrowPressed();
	virtual void ThrowReleased();

	virtual void OnHit();
	virtual void Explode();
	virtual void EquipToCharacter(ASpartaSurvivalCharacter* Character);
	ASpartaSurvivalCharacter* CurrentCharacter;

	void Charging();
	FTimerHandle ChargingTimerHandle;


	//explosion
	void StartExplosionTimer();

	UPROPERTY(EditAnywhere, Category = "Explosion")
	float ExplosionDelay = 3.f;

	FTimerHandle ExplosionTimerHandle;

};
