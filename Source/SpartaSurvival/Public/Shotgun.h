// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DefaultGun.h"
#include "Shotgun.generated.h"


class ASpartaSurvivalCharacter;
class TimerManager;

UCLASS()
class SPARTASURVIVAL_API AShotgun : public ADefaultGun
{
	GENERATED_BODY()
private:
	AActor* LastHitActor; // 마지막으로 충돌한 액터를 저장하는 변수

	FTimerHandle ReloadTimerHandle;// 재장전 타이머 핸들
	void EndReload();

	float ShotgunRange = 1000.f;
	int32 PelletCount = 8;
	float SpreadAngle = 8.f;
	float DamagePerPellet = 10.f;
	
protected:
	ASpartaSurvivalCharacter* CurrentCharacter;

public:
	AShotgun();

	AActor* GetHitActor() const;
	USceneComponent* GetGripPoint() const { return GripPoint; }
	UStaticMeshComponent* GetGunMesh() const { return GunMesh; }

	virtual void Fire() override;
	virtual void Reload() override;
	virtual void Zoom(bool bIsZoom) override;
	virtual void EquipToCharacter(ASpartaSurvivalCharacter* Character) override;
};
