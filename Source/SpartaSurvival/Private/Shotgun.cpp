// Fill out your copyright notice in the Description page of Project Settings.
#include "Shotgun.h"
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



AShotgun::AShotgun()
{
	ZoomMultiplier = 1.2f;
	ReloadDuration = 1.5f;
	MaxAmmo = 8;
	CurrentAmmo = MaxAmmo;
	CanFire = true;

	ShotgunRange = 1000.f;
	PelletCount = 8;
	SpreadAngle = 8.f;
	DamagePerPellet = 10.f;

	MeleeDuration = .8f;
	MeleeRange = 150.f;

	//mesh 적용하기 
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
		TEXT("/Game/GunMeshes/ShotgunFinal.ShotgunFinal")
	);

	if (MeshAsset.Succeeded())
	{
		GunMesh->SetSkeletalMesh(MeshAsset.Object);
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

	//reload animation
	static ConstructorHelpers::FObjectFinder<UAnimationAsset> ReloadAnimAsset(
		TEXT("/Game/GunMeshes/ShotgunFinal_Anim.ShotgunFinal_Anim")
	);

	if (ReloadAnimAsset.Succeeded())
	{
		GunReloadAnimation = ReloadAnimAsset.Object;
	}

	//shell spawn point
	ShellSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ShellSpawnPoint"));
	ShellSpawnPoint->SetupAttachment(GunMesh);
}
void AShotgun::ShowCrosshair()
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

void AShotgun::HideCrosshair()
{
	if (CrosshairWidget)
	{
		CrosshairWidget->RemoveFromParent();
		CrosshairWidget = nullptr;
	}
}
//캐릭터에게 장착
void AShotgun::EquipToCharacter(ASpartaSurvivalCharacter* Character)
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

}

void AShotgun::SpawnShotgunShells()
{
	if (!ShellMesh || !ShellSpawnPoint) return;

	for (int32 i = 0; i < 2; i++)
	{
		FVector SpawnLoc =
			ShellSpawnPoint->GetComponentLocation()
			+ GetActorRightVector() * (15.f + i * 8.f)
			+ GetActorUpVector() * 5.f;

		FRotator SpawnRot = FRotator::ZeroRotator;

		//즉 월드에 실제로 생성됩니다, 보이려면 생성 후에 Static Mesh를 넣어야 합니다.

		AStaticMeshActor* ShellActor =
			GetWorld()->SpawnActor<AStaticMeshActor>(
				AStaticMeshActor::StaticClass(),
				SpawnLoc,
				SpawnRot
			);

		if (!ShellActor) return;
		//component 
		UStaticMeshComponent* ShellComp = ShellActor->GetStaticMeshComponent();
		
		//staitc to movable
		ShellComp->SetMobility(EComponentMobility::Movable);
		ShellComp->SetStaticMesh(ShellMesh);
		ShellComp->SetWorldScale3D(FVector(0.0004f));

		//no collision
		ShellComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ShellComp->SetCollisionObjectType(ECC_PhysicsBody);

		// 4. 플레이어랑은 안 부딪히게, 바닥은 부딪히게
		ShellComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		ShellComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		ShellComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

		//무게 가볍게
		ShellComp->SetMassOverrideInKg(NAME_None, 0.02f, true);

		//gravity physics
		ShellComp->SetSimulatePhysics(true);
		ShellComp->SetEnableGravity(true);

		//살짝 튀어나가게
		FVector Impulse =
			GetActorRightVector() * 40.f
			+ GetActorUpVector() * 30.f
			- GetActorForwardVector() * 10.f;
		 
		ShellComp->AddImpulse(Impulse, NAME_None, true);

		// delete after few secs
		ShellActor->SetLifeSpan(ShellLifeTime);

	}
}


//샷건 기본 로직 구현
void AShotgun::Fire()
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
					PointDamageEvent.Damage = DamagePerPellet;
					PointDamageEvent.HitInfo = HitResult;
					PointDamageEvent.ShotDirection = ShotDirection;
					PointDamageEvent.ShotDirection = ShotDirection;

					AController* InstigatorController =
						CurrentCharacter ? CurrentCharacter->GetController() : nullptr;

					HitEnemy->TakeDamage(DamagePerPellet, PointDamageEvent, InstigatorController, this);
				}
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
	if (bIsReloading || CurrentAmmo == MaxAmmo) return;

	bIsReloading = true;
	CanFire = false;

	if (CurrentCharacter)
	{
		CurrentCharacter->SetBlockLeftHandIK(true);
	}

	if (GunMesh && GunReloadAnimation)
	{
		GunMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		GunMesh->PlayAnimation(GunReloadAnimation, false);
	}

	//shells dropped
	SpawnShotgunShells();

	if (ReloadSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
	}

	if (CurrentCharacter && CurrentCharacter->GetMesh())
	{
		if (UAnimInstance* AnimInstance = CurrentCharacter->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(ReloadMontage, 1.0f);
		}
	}

	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimer,
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

	if (CurrentCharacter)
	{
		CurrentCharacter->SetBlockLeftHandIK(false);
	}


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

void AShotgun::Melee()
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
		&AShotgun::EndMelee,
		MeleeDuration,
		false
	);
}

void AShotgun::EndMelee()
{
	bIsMeleeing = false;
	CanFire = true;
	UE_LOG(LogTemp, Warning, TEXT("Melee attack ended."));
}
