// Fill out your copyright notice in the Description page of Project Settings.
#include "AssultRifle.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "../SpartaSurvivalCharacter.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "EnemyBase.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"



AAssultRifle::AAssultRifle()
{
	ZoomMultiplier = 1.4f;
	ReloadDuration = 1.5f;
	MaxAmmo = 32;
	CurrentAmmo = MaxAmmo;
	CanFire = true;

	AssultRifleRange = 2000.f;
	DamagePerBullet = 10.f;
	SpreadAngle = .8;

	MeleeDuration = .8f;
	MeleeRange = 150.f;

	//mesh 적용하기 
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
		TEXT("/Game/GunMeshes/AssultRifleFinal.AssultRifleFinal")
	);

	if (MeshAsset.Succeeded())
	{
		GunMesh->SetSkeletalMesh(MeshAsset.Object);
	}

	GunMesh->SetRelativeScale3D(FVector(1.1f));
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
void AAssultRifle::ShowCrosshair()
{
	if (CrosshairWidget || !CrosshairWidgetClass || !CurrentCharacter) return;

	APlayerController* PC =
		Cast<APlayerController>(CurrentCharacter->GetController());

	if (!PC || !PC->IsLocalController()) return;

	CrosshairWidget = CreateWidget<UUserWidget>(PC, CrosshairWidgetClass);

	if (CrosshairWidget)
	{
		CrosshairWidget->AddToViewport();
	}
}

void AAssultRifle::HideCrosshair()
{
	if (CrosshairWidget)
	{
		CrosshairWidget->RemoveFromParent();
		CrosshairWidget = nullptr;
	}
}
void AAssultRifle::SpawnMagazine()
{
	if (!MagazineMesh || !MagazineSpawnPoint) return;

	CurrentMagazineActor = GetWorld()->SpawnActor<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(),
		MagazineSpawnPoint->GetComponentTransform()
	);

	if (!CurrentMagazineActor) return;

	UStaticMeshComponent* MagazineComp =
		CurrentMagazineActor->GetStaticMeshComponent();

	MagazineComp->SetMobility(EComponentMobility::Movable);
	MagazineComp->SetStaticMesh(MagazineMesh);
	MagazineComp->SetSimulatePhysics(false);
	MagazineComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CurrentMagazineActor->AttachToComponent(
		MagazineSpawnPoint,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);
}
void AAssultRifle::DropMagazine()
{
	if (!CurrentMagazineActor) return;

	AStaticMeshActor* DroppedMagazine = CurrentMagazineActor;
	CurrentMagazineActor = nullptr;

	DroppedMagazine->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	UStaticMeshComponent* MagComp =
		DroppedMagazine->GetStaticMeshComponent();

	MagComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MagComp->SetCollisionResponseToAllChannels(ECR_Block);
	MagComp->SetSimulatePhysics(true);

	MagComp->AddImpulse(-GetActorRightVector() * 200.f, NAME_None, true);

	DroppedMagazine->SetLifeSpan(3.0f);
}
//캐릭터에게 장착
void AAssultRifle::EquipToCharacter(ASpartaSurvivalCharacter* Character)
{
	if (!Character || !Character->GetWeaponSocket() || !GripPoint || !GetRootComponent()) return;

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

	ShowCrosshair();
	SpawnMagazine();

}

//assult rifle
void AAssultRifle::Fire()
{
	UE_LOG(LogTemp, Warning, TEXT("Firning"));
	if (CanFire && CurrentAmmo > 0)
	{
		if (!MuzzlePoint) return;

		CurrentAmmo--;

		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				MuzzlePoint->GetComponentLocation()
			);
		}

		if (CurrentCharacter && FireCameraShake)
		{
			APlayerController* PC = Cast<APlayerController>(CurrentCharacter->GetController());

			if (PC && PC->PlayerCameraManager)
			{
				PC->PlayerCameraManager->StartCameraShake(FireCameraShake, 10.f);
			}
		}

		UCameraComponent* CurrentCam = CurrentCharacter->GetFollowCamera();
		if (!CurrentCam) return;

		FVector StartPoint = CurrentCam->GetComponentLocation();
		FVector StartDirection = CurrentCam->GetForwardVector();

		FCollisionQueryParams CollisionParameters;
		CollisionParameters.AddIgnoredActor(this); //자신은 충돌에서 제외
		CollisionParameters.AddIgnoredActor(GetOwner()); //소유자도 충돌에서 제외


		FVector ShotDirection = FMath::VRandCone(
			StartDirection,
			FMath::DegreesToRadians(SpreadAngle)
		);

		FVector EndPoint = StartPoint + ShotDirection * AssultRifleRange; // 발사될 방향 계산

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

			//총알 구멍
			if (BulletHitMaterial)
			{
				FVector DecalLocation =
					HitResult.ImpactPoint + HitResult.ImpactNormal * 1.f;

				FRotator DecalRotation =
					HitResult.ImpactNormal.Rotation();

				UGameplayStatics::SpawnDecalAtLocation(
					GetWorld(),
					BulletHitMaterial,
					BulletHitSize,
					DecalLocation,
					DecalRotation,
					BulletHitLifeTime
				);
			}

			AEnemyBase* HitEnemy = Cast<AEnemyBase>(LastHitActor);

			//데미지 처리 총기 line
			if (HitEnemy)
			{
				FPointDamageEvent PointDamageEvent;
				PointDamageEvent.Damage = DamagePerBullet;
				PointDamageEvent.HitInfo = HitResult;
				PointDamageEvent.ShotDirection = ShotDirection;
				PointDamageEvent.ShotDirection = ShotDirection;

				AController* InstigatorController =
					CurrentCharacter ? CurrentCharacter->GetController() : nullptr;

				HitEnemy->TakeDamage(DamagePerBullet, PointDamageEvent, InstigatorController, this);
			}
		}
		else
		{
			LastHitActor = nullptr; //충돌 없으면 null로 초기화	
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
void AAssultRifle::Reload()
{
	if (bIsReloading || CurrentAmmo == MaxAmmo) return;

	bIsReloading = true;
	CanFire = false;

	if (CurrentCharacter)
	{
		CurrentCharacter->SetBlockLeftHandIK(true);
	}

	if (ReloadSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
	}

	//reload anim
	if (!ReloadMontage)
	{
		UE_LOG(LogTemp, Error, TEXT("ReloadMontage is NULL"));
		return;
	}

	if (CurrentCharacter && CurrentCharacter->GetMesh())
	{
		if (UAnimInstance* AnimInstance = CurrentCharacter->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(ReloadMontage, 1.0f);
		}
	}

	DropMagazine();

	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimer,
		this,
		&AAssultRifle::EndReload,
		ReloadDuration,
		false
	);
}

void AAssultRifle::EndReload()
{
	CurrentAmmo = MaxAmmo;
	CanFire = true;
	bIsReloading = false;

	if (CurrentCharacter)
	{
		CurrentCharacter->SetBlockLeftHandIK(false);
	}

	SpawnMagazine();
	UE_LOG(LogTemp, Warning, TEXT("Shotgun reloaded! Ammo reset to: %d"), CurrentAmmo);
}

void AAssultRifle::Zoom(bool bIsZoom)
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

void AAssultRifle::Melee()
{
	if (bIsMeleeing) return;

	bIsMeleeing = true;
	CanFire = false;

	//ANIMATION 
	UAnimInstance* AnimInstance = CurrentCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reload failed: AnimInstance NULL"));
		return;
	}

	float PlayResult = AnimInstance->Montage_Play(MeleeMontage, 1.0f);
	GetWorld()->GetTimerManager().SetTimer(
		MeleeTimer,
		this,
		&AAssultRifle::EndMelee,
		MeleeDuration,
		false
	);
}

void AAssultRifle::EndMelee()
{
	bIsMeleeing = false;
	CanFire = true;
	UE_LOG(LogTemp, Warning, TEXT("Melee attack ended."));
}
