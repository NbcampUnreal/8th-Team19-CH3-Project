// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DefaultGun.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS(Abstract)
class SPARTASURVIVAL_API ADefaultGun : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADefaultGun();

	virtual void Fire();
	virtual void Reload();
	virtual void Aim();

	float AimFov;
	float AimDuration;
	float ReloadDuration;
	
	int32 MaxAmmo;      // 최대 총알 수
	int32 CurrentAmmo;  // 현재/남은 총알 수

	bool CanFire;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
	USceneComponent* GunRoot;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
	USceneComponent* MuzzleRoot;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
	USceneComponent* GripRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
	UStaticMeshComponent* GunMesh;


};
