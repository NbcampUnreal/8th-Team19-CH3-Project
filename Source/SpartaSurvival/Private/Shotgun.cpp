// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"

AShotgun::AShotgun()
{
	AimFov = 50.f;
	AimDuration = 0.25f;
	ReloadDuration = 1.5f;
	MaxAmmo = 8;
	CurrentAmmo = MaxAmmo;
	CanFire = true;
}	

void AShotgun::Fire()
{
	if (CanFire && CurrentAmmo > 0)
	{
		// 총알 발사 로직 구현
		CurrentAmmo--;
		UE_LOG(LogTemp, Log, TEXT("Shotgun fired! Remaining ammo: %d"), CurrentAmmo);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot fire! No ammo or gun is not ready."));
	}
}

void AShotgun::Reload()
{
	// 재장전 로직 구현
	CurrentAmmo = MaxAmmo;
	UE_LOG(LogTemp, Log, TEXT("Shotgun reloaded! Ammo reset to: %d"), CurrentAmmo);
}

void AShotgun::Aim()
{
	// 조준 로직 구현
	UE_LOG(LogTemp, Log, TEXT("Shotgun aiming! FOV: %f, Duration: %f"), AimFov, AimDuration);
}