// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"

#include "GunController.h"
#include "DefaultGun.h"
#include "Shotgun.h"

#include "UObject/ConstructorHelpers.h"

#include "EnhancedInputComponent.h"


// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AGunController* GunController = Cast<AGunController>(GetController()))
		{
			if (GunController->FireAction)
			{
				EnhancedInput->BindAction(GunController->FireAction, ETriggerEvent::Triggered, this, &AMainCharacter::Fire);
			}
			if (GunController->ReloadAction)
			{
				EnhancedInput->BindAction(GunController->ReloadAction, ETriggerEvent::Triggered, this, &AMainCharacter::Reload);
			}
			if (GunController->ZoomAction)
			{
				EnhancedInput->BindAction(GunController->ZoomAction, ETriggerEvent::Triggered, this, &AMainCharacter::StartZoom);
				EnhancedInput->BindAction(GunController->ZoomAction, ETriggerEvent::Completed, this, &AMainCharacter::EndZoom	);
			}

		}	
	}

}

void AMainCharacter::SetEquippedGun(ADefaultGun* ToBeEquippedGun)
{
	EquippedGun = ToBeEquippedGun;
}


void AMainCharacter::Fire()
{
	if (EquippedGun)
	{
		EquippedGun->Fire();
	}
}


void AMainCharacter::Reload()
{
	if (EquippedGun)
	{
		EquippedGun->Reload();
	}
}

void AMainCharacter::StartZoom()
{
	if (EquippedGun)
	{
		EquippedGun->Zoom(true);
	}
}

void AMainCharacter::EndZoom()
{
	if (EquippedGun)
	{
		EquippedGun->Zoom(false);
	}	
}
