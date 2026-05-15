// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGun.h"
#include "../SpartaSurvivalCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
// Sets default values
ADefaultGun::ADefaultGun()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GunRoot = CreateDefaultSubobject<USceneComponent>(TEXT("GunRoot"));
	RootComponent = GunRoot;

	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
	GunMesh->SetupAttachment(GunRoot);

	MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
	MuzzlePoint->SetupAttachment(GunMesh);

	GripPoint = CreateDefaultSubobject<USceneComponent>(TEXT("GripPoint"));
	GripPoint->SetupAttachment(GunMesh);

	SupportPoint = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHandGrip"));
	SupportPoint->SetupAttachment(GunMesh);

}
void ADefaultGun::EquipToCharacter(ASpartaSurvivalCharacter* Character)
{
}
void ADefaultGun::Fire()
{
}
void ADefaultGun::Reload()
{
}
void ADefaultGun::Zoom(bool bIsZoom)
{
}
void ADefaultGun::Melee()
{
}