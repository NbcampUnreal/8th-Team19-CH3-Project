// Fill out your copyright notice in the Description page of Project Settings.
#include "Grenade.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "../SpartaSurvivalCharacter.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include "EnemyBase.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Controller.h"
#include "Engine/DamageEvents.h"

#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

AGrenade::AGrenade()
{
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> ExplosionAsset(
		TEXT("/Game/GunMeshes/NS_GrenadeExplosion.NS_GrenadeExplosion")
	);

	if (ExplosionAsset.Succeeded())
	{
		ExplosionEffect = ExplosionAsset.Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Explosion Niagara load failed"));
	}

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

	static ConstructorHelpers::FObjectFinder<USoundBase> ExplosionSoundAsset(
		TEXT("/Game/GunMeshes/ExlpodedSound.ExlpodedSound")
	);

	if (ExplosionSoundAsset.Succeeded())
	{
		ExplosionSound = ExplosionSoundAsset.Object;
	}
}

void AGrenade::Explode()
{
	FVector Loc = GetActorLocation();

	if (ThrowableMesh)
	{
		Loc = ThrowableMesh->GetComponentLocation();
	}

	UE_LOG(LogTemp, Error, TEXT("Explosion Loc: %s"), *Loc.ToString());

	if (ExplosionEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ExplosionEffect,
			Loc,
			FRotator::ZeroRotator,
			FVector(0.1f)
		);
	}

	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ExplosionSound,
			Loc
		);
	}


	TArray<FOverlapResult> Overlaps;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->OverlapMultiByChannel(Overlaps, Loc, FQuat::Identity, ECC_Pawn,
	FCollisionShape::MakeSphere(ExplosionRadius), QueryParams);

	if (bHit)
	{
		//데미지를 이미 받은 Actor 목록
		TSet<AActor*> DamagedActors;
		for (const FOverlapResult& Result : Overlaps)
		{
			AEnemyBase* HitEnemy = Cast<AEnemyBase>(Result.GetActor());
			if (!HitEnemy) continue;
			if (DamagedActors.Contains(HitEnemy)) continue;

			DamagedActors.Add(HitEnemy);

			FDamageEvent DamageEvent;

			AController* InstigatorController =
				GetInstigator() ? GetInstigator()->GetController() : nullptr;

			HitEnemy->TakeDamage(ExplosionDamage, DamageEvent, InstigatorController, this);
		}
	}

	Destroy();
}

//캐릭터에게 장착 
void AGrenade::EquipToCharacter(ASpartaSurvivalCharacter* Character)
{
	if (!Character || !GetRootComponent()) return;
	Character->SetEquippedThrowable(this);
}