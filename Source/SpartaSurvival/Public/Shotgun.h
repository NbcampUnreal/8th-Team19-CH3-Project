// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DefaultGun.h"
#include "Shotgun.generated.h"

UCLASS()
class SPARTASURVIVAL_API AShotgun : public ADefaultGun
{
	GENERATED_BODY()

public:
	AShotgun();

	virtual void Fire() override;
	virtual void Reload() override;
	virtual void Aim() override;
	
};
