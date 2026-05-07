// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGun.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ADefaultGun::ADefaultGun()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GunRoot = CreateDefaultSubobject<USceneComponent>(TEXT("GunRoot"));
	RootComponent = GunRoot;

	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunMesh"));
	GunMesh->SetupAttachment(GunRoot);

	MuzzleRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleRoot"));
	MuzzleRoot->SetupAttachment(GunMesh);

	GripRoot = CreateDefaultSubobject<USceneComponent>(TEXT("GripRoot"));
	GripRoot->SetupAttachment(GunMesh);

}

void ADefaultGun::Fire()
{
}
void ADefaultGun::Reload()
{
}
void ADefaultGun::Aim()
{
}