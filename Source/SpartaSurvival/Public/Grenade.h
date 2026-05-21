// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ThrowableBase.h"
#include "Grenade.generated.h"

class ASpartaSurvivalCharacter;


UCLASS()
class SPARTASURVIVAL_API AGrenade : public AThrowableBase
{
	GENERATED_BODY()
public:
	AGrenade();

	int32 CurrentGrenadeCount;
	int32 GetCurrentGrenadeCount() { return CurrentGrenadeCount; }

	void AddGrenade() { CurrentGrenadeCount += 1; }
	void UseGrenade() { CurrentGrenadeCount -= 1; }

	virtual void Explode() override;
	virtual void EquipToCharacter(ASpartaSurvivalCharacter* Character) override;
	//	virtual void OnHit() override;
};
