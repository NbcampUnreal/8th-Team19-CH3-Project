// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"

struct FInputActionValue;
class ADefaultGun;
class AShotgun;

UCLASS()
class SPARTASURVIVAL_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMainCharacter();

	void SetEquippedGun(ADefaultGun* NewGun);

protected:

	virtual void BeginPlay() override;

	//샷건 bp 
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AShotgun> ShotgunBP;

	//현재 장착된 총기 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
	ADefaultGun* EquippedGun = nullptr;


	// 총기 액션 함수들 
	void Fire();
	void Reload();
	void StartZoom();
	void EndZoom();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
