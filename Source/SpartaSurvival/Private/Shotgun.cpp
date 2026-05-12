// Fill out your copyright notice in the Description page of Project Settings.
#include "Shotgun.h"
#include "../SpartaSurvivalCharacter.h"
#include "DrawDebugHelpers.h"


AShotgun::AShotgun()
{
	ZoomMultiplier = 2.f;
	ReloadDuration = 1.5f;
	MaxAmmo = 8;
	CurrentAmmo = MaxAmmo;
	CanFire = true;


	//mesh 적용하기 
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(
		TEXT("/Game/GunMeshes/ShotgunPrototype.ShotgunPrototype")
	);

	if (MeshAsset.Succeeded())
	{
		GunMesh->SetStaticMesh(MeshAsset.Object);
	}

	GunMesh->SetRelativeScale3D(FVector(.37f, .37f, .37f));
	GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GunMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	GunMesh->SetGenerateOverlapEvents(false);



}

//마지막으로 맞은 액터 반환
AActor* AShotgun::GetHitActor() const 
{
	return LastHitActor;
}

//캐릭터에게 장착
void AShotgun::EquipToCharacter(ASpartaSurvivalCharacter* Character)
{
	if (!Character || !Character->GetWeaponSocket() || !Character->GetSupportSocket() || !GripPoint || !SupportPoint || !GetRootComponent()) return;

	CurrentCharacter = Character;
	Character->SetEquippedGun(this);

	// 먼저 샷건 Actor를 WeaponSocket에 붙임
	AttachToComponent(
		Character->GetWeaponSocket(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);

	// GripPoint 위치와 WeaponSocket 위치 차이 계산
	FVector Delta =
		Character->GetWeaponSocket()->GetComponentLocation()
		- GripPoint->GetComponentLocation();

	// 샷건 Actor 전체를 이동해서 GripPoint를 WeaponSocket 위치에 맞춤
	AddActorWorldOffset(Delta);

	//support point를 SupportSocket에 붙임
	GetSupportPoint()->AttachToComponent(
		Character->GetSupportSocket(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);
}

//샷건 기본 로직 구현
void AShotgun::Fire()
{
	if (CanFire && CurrentAmmo > 0)
	{
		if (!MuzzlePoint) return;
		
		CurrentAmmo--;

		FVector StartPoint = MuzzlePoint->GetComponentLocation();
		FVector StartDirection = MuzzlePoint->GetForwardVector();

		FCollisionQueryParams CollisionParameters;
		CollisionParameters.AddIgnoredActor(this); //자신은 충돌에서 제외
		CollisionParameters.AddIgnoredActor(GetOwner()); //소유자도 충돌에서 제외


		for (int32 i = 0; i < PelletCount; i++)
		{
			FVector ShotDirection = FMath::VRandCone(
				StartDirection,
				FMath::DegreesToRadians(SpreadAngle)
			);

			FVector EndPoint = StartPoint + ShotDirection * ShotgunRange; // 발사될 방향 계산

			FHitResult HitResult;

			bool bHit = GetWorld()->LineTraceSingleByChannel( // 맞으면 HitResult 안에 충돌 정보가 저장
				HitResult,
				StartPoint,
				EndPoint,
				ECC_Visibility,
				CollisionParameters
			);

			DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Red, false, 1.f, 0, 2.f);

			if (bHit && HitResult.GetActor())
			{
				LastHitActor = HitResult.GetActor(); //Hit 액터 저장
				UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *HitResult.GetActor()->GetName()); //충돌한 액터 이름 로그
			}
			else
			{
				LastHitActor = nullptr; //충돌 없으면 null로 초기화	
				UE_LOG(LogTemp, Warning, TEXT("Shotgun fired but hit nothing."));
			}
			UE_LOG(LogTemp, Log, TEXT("Shotgun fired! Remaining ammo: %d"), CurrentAmmo);
		}



	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot fire! No ammo or gun is not ready."));
	}
}

//재장전
void AShotgun::Reload()
{
	CanFire = false;

	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&AShotgun::EndReload,
		ReloadDuration,
		false
	);
}

void AShotgun::EndReload()
{
	CurrentAmmo = MaxAmmo;
	CanFire = true;

	UE_LOG(LogTemp, Log, TEXT("Shotgun reloaded! Ammo reset to: %d"), CurrentAmmo);
}

void AShotgun::Zoom(bool bIsZoom)
{

}