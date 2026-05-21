// Fill out your copyright notice in the Description page of Project Settings.
#include "Grenade.h"
#include "../SpartaSurvivalCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGrenade::AGrenade()
{
	CurrentGrenadeCount = 3;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(
		TEXT("Game/GunMeshes/GrenadeMesh.GrenadeMesh")
	);
}

void AGrenade::Explode()
{
}

//캐릭터에게 장착 
void AGrenade::EquipToCharacter(ASpartaSurvivalCharacter* Character)
{
	if (!Character || !GetRootComponent()) return;
	Character->SetEquippedThrowable(this);
}