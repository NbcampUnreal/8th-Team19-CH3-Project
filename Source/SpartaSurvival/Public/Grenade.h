// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ThrowableBase.h"
#include "Grenade.generated.h"

class ASpartaSurvivalCharacter;
class UNiagaraSystem;
class USoundBase;

UCLASS()
class SPARTASURVIVAL_API AGrenade : public AThrowableBase
{
	GENERATED_BODY()
public:
	AGrenade();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throwable")
	TSubclassOf<AGrenade> GrenadeBP;

	virtual void Explode() override;
	virtual void EquipToCharacter(ASpartaSurvivalCharacter* Character) override;
	//	virtual void OnHit() override;

	UPROPERTY(EditAnywhere, Category = "Explosion")
	UNiagaraSystem* ExplosionEffect;

	UPROPERTY(EditAnywhere, Category = "Explosion")
	USoundBase* ExplosionSound;
};
