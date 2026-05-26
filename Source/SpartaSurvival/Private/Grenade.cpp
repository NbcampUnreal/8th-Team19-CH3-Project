// Fill out your copyright notice in the Description page of Project Settings.
#include "Grenade.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "../SpartaSurvivalCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGrenade::AGrenade()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(
		TEXT("/Game/GunMeshes/GrenadeMesh.GrenadeMesh")
	);

	if (MeshAsset.Succeeded())
	{
		ThrowableMesh->SetStaticMesh(MeshAsset.Object);
	}

	ThrowableMesh->SetSimulatePhysics(false);
	ThrowableMesh->SetEnableGravity(false);
	ThrowableMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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