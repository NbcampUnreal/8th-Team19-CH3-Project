// Fill out your copyright notice in the Description page of Project Settings.
#include "Shotgun.h"
#include "../SpartaSurvivalCharacter.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"


AShotgun::AShotgun()
{
	ZoomMultiplier = 1.2f;
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

	GunMesh->SetRelativeScale3D(FVector(.38f, .38f, .38f));
	GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GunMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	GunMesh->SetGenerateOverlapEvents(false);

	MuzzlePoint->SetRelativeLocation(FVector(0.f, 90.f, 0.f));

	//muzzleflash
	MuzzleFlash = CreateDefaultSubobject<UBillboardComponent>(TEXT("MuzzleFlash"));
	MuzzleFlash->SetupAttachment(MuzzlePoint);
	MuzzleFlash->SetRelativeLocation(FVector::ZeroVector);
	MuzzleFlash->SetRelativeRotation(FRotator::ZeroRotator);
	MuzzleFlash->SetHiddenInGame(false);
	MuzzleFlash->SetRelativeScale3D(FVector(3.f));
	MuzzleFlash->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));

	static ConstructorHelpers::FObjectFinder<UTexture2D> Flash01(
		TEXT("/Game/GunMeshes/MF1.MF1")
	);
	static ConstructorHelpers::FObjectFinder<UTexture2D> Flash02(
		TEXT("/Game/GunMeshes/MF2.MF2")
	);
	static ConstructorHelpers::FObjectFinder<UTexture2D> Flash03(
		TEXT("/Game/GunMeshes/MF3.MF3")
	);

	if (Flash01.Succeeded()) MuzzleFlashTextures.Add(Flash01.Object);
	if (Flash02.Succeeded()) MuzzleFlashTextures.Add(Flash02.Object);
	if (Flash03.Succeeded()) MuzzleFlashTextures.Add(Flash03.Object);
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

		UCameraComponent* CurrentCam = CurrentCharacter->GetFollowCamera(); 
		if (!CurrentCam) return;

		FVector StartPoint = CurrentCam->GetComponentLocation();
		FVector StartDirection = CurrentCam->GetForwardVector();

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

			DrawDebugLine(GetWorld(), MuzzlePoint->GetComponentLocation(), EndPoint, FColor::Red, false, 1.f, 0, 2.f);

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

		//muzzle flash
		if (MuzzleFlash && MuzzleFlashTextures.Num() > 0)
		{
			MuzzleFlash->SetSprite(MuzzleFlashTextures[MuzzleFlashIndex]);

			MuzzleFlashIndex = (MuzzleFlashIndex + 1) % MuzzleFlashTextures.Num();

			MuzzleFlash->SetHiddenInGame(false);

			GetWorld()->GetTimerManager().SetTimer(
				MuzzleFlashTimer,
				[this]()
				{
					if (MuzzleFlash)
					{
						MuzzleFlash->SetHiddenInGame(true);
					}
				},
				0.05f,
				false
			);
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
	if (!CanFire || CurrentAmmo == MaxAmmo) return;
	if (bIsReloading) return;

	bIsReloading = true;
	CanFire = false;

	// 재장전 애니메이션 재생
	if (CurrentCharacter && ReloadMontage)
	{
		if (UAnimInstance* Anim = CurrentCharacter->GetMesh()->GetAnimInstance())
		{
			Anim->Montage_Play(ReloadMontage, 1.0f);
		}
	}

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
	bIsReloading = false;
	UE_LOG(LogTemp, Warning, TEXT("Shotgun reloaded! Ammo reset to: %d"), CurrentAmmo);
}

void AShotgun::Zoom(bool bIsZoom)
{
	if (!CurrentCharacter || !CurrentCharacter->GetFollowCamera())
	{
		UE_LOG(LogTemp, Warning, TEXT("Zoom failed: CurrentCharacter or Camera is NULL"));
		return;
	}

	if (bIsZoom)
	{
		CurrentCharacter->GetFollowCamera()->SetFieldOfView(55.f);
		UE_LOG(LogTemp, Warning, TEXT("Zoom In"));
	}
	else
	{
		CurrentCharacter->GetFollowCamera()->SetFieldOfView(90.f);
		UE_LOG(LogTemp, Warning, TEXT("Zoom Out"));
	}
}