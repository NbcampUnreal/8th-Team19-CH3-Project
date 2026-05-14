// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DefaultGun.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class ASpartaSurvivalCharacter;

UCLASS(Abstract)
class SPARTASURVIVAL_API ADefaultGun : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADefaultGun();

	//총기 함수
	virtual void Fire();
	virtual void Reload();
	virtual void Zoom(bool bIsZoom);
	virtual void EquipToCharacter(ASpartaSurvivalCharacter* Character);
	virtual void Melee();

	//총기 정보
	float ZoomMultiplier;
	float CurrentFov;
	float ReloadDuration;

	//근접 공격 정보
	float MeleeDuration;
	float MeleeRange;
	
	int32 MaxAmmo;      // 최대 총알 수
	int32 CurrentAmmo;  // 현재/남은 총알 수

	bool CanFire;
	
protected:
	//총기 블루프린트 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
	USceneComponent* GunRoot;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
	USceneComponent* MuzzlePoint;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
	USceneComponent* GripPoint;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun")
	USceneComponent* SupportPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
	UStaticMeshComponent* GunMesh;

	ADefaultGun* EquippedGun;


};
