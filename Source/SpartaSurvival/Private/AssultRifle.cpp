// assult rifle cpp
#include "AssultRifle.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "../SpartaSurvivalCharacter.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "EnemyBase.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Controller.h"
#include "GameFramework/DamageType.h"
#include "EnemyBase.h"
#include "Kismet/GameplayStatics.h"



AAssultRifle::AAssultRifle()
{
	PrimaryActorTick.bCanEverTick = true;

	ZoomMultiplier = 1.4f;
	ReloadDuration = 2.f;
	MaxAmmo = 32;
	MaxRecoil = static_cast<float>(MaxAmmo) * RecoilPerShot;
	CurrentAmmo = MaxAmmo;
	CanFire = true;

	AssultRifleRange = 2000.f;
	DamagePerBullet = 10.f;
	SpreadAngle = 1.f;

	MeleeDuration = .8f;
	MeleeRange = 150.f;

	//mesh 적용하기 
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
		TEXT("/Game/GunMeshes/AssultRifleMesh.AssultRifleMesh")
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

	//magazinePoint
	MagazineSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("MagazinePoint"));
	MagazineSpawnPoint->SetupAttachment(GunMesh);

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

	//zoom cam

	ScopeCamPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ScopeCamPoint"));
	ScopeCamPoint->SetupAttachment(GunMesh);

	ScopeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScopeMesh"));
	ScopeMesh->SetupAttachment(GunMesh);
	ScopeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ScopeMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ScopeMesh->SetGenerateOverlapEvents(false);

}
void AAssultRifle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CurrentCharacter) return;

	APlayerController* PlayerController =
		Cast<APlayerController>(CurrentCharacter->GetController());

	if (!PlayerController) return;

	if (CurrentCharacter && CurrentCharacter->GetFollowCamera() && ScopeCamPoint)
	{
		UCameraComponent* FollowCamera = CurrentCharacter->GetFollowCamera();
		USceneComponent* CameraParent = FollowCamera->GetAttachParent();

		if (CameraParent)
		{
			if (bIsScoped && !bIsReloading)
			{
				FTransform ParentSocketTransform = CameraParent->GetSocketTransform(
					FollowCamera->GetAttachSocketName(),
					ERelativeTransformSpace::RTS_World
				);
				FVector ScopeLocation = ScopeCamPoint->GetComponentLocation();
				//scope위치를 camera parent 기준 Relative 위치로 변환
				FVector TargetRelativeLocation = ParentSocketTransform.InverseTransformPosition(ScopeLocation);
				//보간
				FVector NewRelativeLocation = FMath::VInterpTo(
					FollowCamera->GetRelativeLocation(),
					TargetRelativeLocation,
					DeltaTime,
					ScopeCameraInterpSpeed
				);
				//지정
				FollowCamera->SetRelativeLocation(NewRelativeLocation);
				FRotator CameraRot = FollowCamera->GetComponentRotation();
				FRotator OffsetRot = FRotator(0.f, 90.f, 0.f);

				GunMesh->SetWorldRotation((FQuat(CameraRot) * FQuat(OffsetRot)).Rotator());
				SupportPoint->SetRelativeLocation(FVector(-4.362456f, -7.89851f, -5.0));
			}
			else if (bSavedScopeCamera)
			{
				GunMesh->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
				FVector NewRelativeLocation = FMath::VInterpTo(
					FollowCamera->GetRelativeLocation(),
					DefaultCameraRelativeLocation,
					DeltaTime,
					ScopeCameraInterpSpeed
				);
				FollowCamera->SetRelativeLocation(NewRelativeLocation);
				SupportPoint->SetRelativeLocation(FVector(-4.362456f, -7.89851f, 4.017956f));

				//원래 위치에 거의 도달했는지 
				if (FVector::Dist(NewRelativeLocation, DefaultCameraRelativeLocation) < 1.f)
				{
					FollowCamera->SetRelativeLocation(DefaultCameraRelativeLocation);
					bSavedScopeCamera = false;
				}
			}
		}
	}

	//recoil 복구하기
	if (CurrentRecoil > 0.f && !bIsTriggerHeld)
	{
		float RecoverRecoil = FMath::Min(CurrentRecoil, RecoilRecoverySpeed * DeltaTime);
		CurrentRecoil -= RecoverRecoil;
		PlayerController->AddPitchInput(RecoverRecoil);
	}
}
void AAssultRifle::ShowCrosshair()
{
	if (CrosshairWidget || !CrosshairWidgetClass || !CurrentCharacter) return;

	APlayerController* PlayerController =
		Cast<APlayerController>(CurrentCharacter->GetController());

	if (!PlayerController || !PlayerController->IsLocalController()) return;

	CrosshairWidget = CreateWidget<UUserWidget>(PlayerController, CrosshairWidgetClass);

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
void AAssultRifle::ApplyRecoil(APlayerController* PlayerController)
{
	float Kick = FMath::Min(RecoilPerShot, MaxRecoil - CurrentRecoil);
	if (Kick <= 0.f) return;

	CurrentRecoil += Kick;

	PlayerController->AddPitchInput(-Kick);
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

void AAssultRifle::FireOnce()
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

		APlayerController* PlayerController = Cast<APlayerController>(CurrentCharacter->GetController());
		if (!PlayerController) return;

		int32 ViewportX = 0;
		int32 ViewportY = 0;
		PlayerController->GetViewportSize(ViewportX, ViewportY);

		FVector StartPoint;
		FVector ShotDirection;

		bool bDeprojected = PlayerController->DeprojectScreenPositionToWorld(
			ViewportX * 0.5f,
			ViewportY * 0.5f,
			StartPoint,
			ShotDirection
		);

		if (!bDeprojected) return;

		SpreadAngle = bIsScoped ? 0.f : 1.f;

		FVector FinalShotDirection = FMath::VRandCone(
			ShotDirection.GetSafeNormal(),
			FMath::DegreesToRadians(SpreadAngle)
		);

		FVector EndPoint = StartPoint + FinalShotDirection * AssultRifleRange;

		FCollisionQueryParams CollisionParameters;
		CollisionParameters.AddIgnoredActor(this);
		CollisionParameters.AddIgnoredActor(GetOwner());
		CollisionParameters.AddIgnoredActor(CurrentCharacter);


		FHitResult HitResult;

		bool bHit = GetWorld()->LineTraceSingleByChannel( // 맞으면 HitResult 안에 충돌 정보가 저장
			HitResult,
			StartPoint,
			EndPoint,
			ECC_Visibility,
			CollisionParameters
		);

		//DrawDebugLine(GetWorld(), MuzzlePoint->GetComponentLocation(), EndPoint, FColor::Red, false, 1.f, 0, 2.f);

		ApplyRecoil(PlayerController);

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
				UE_LOG(LogTemp, Warning, TEXT("[3] EnemyBase 캐스팅 성공! 데미지를 줍니다. 데미지 양: %f"), DamagePerBullet);
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
			UE_LOG(LogTemp, Error, TEXT("❌ [레이 충돌 실패] 아무것도 맞추지 못했습니다. 허공이거나 콜리전 통과됨."));
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

void AAssultRifle::Fire()
{
	APlayerController* PlayerController = Cast<APlayerController>(CurrentCharacter->GetController());
	if (!PlayerController) return;

	if (!bIsTriggerHeld)
	{
		bRecoveringRecoil = false;          // 복구 중단
		SavedCameraRot = PlayerController->GetControlRotation(); // 현재 위치 저장
	}

	bIsTriggerHeld = true;

	if (bIsFiring) return;
	bIsFiring = false;
	FireOnce();

	GetWorldTimerManager().SetTimer(
		FullAutoTimer,
		this,
		&AAssultRifle::Fire,
		FireRate, //약 600RPM
		true);
}

void AAssultRifle::EndFire()
{
	bIsTriggerHeld = false;
	bIsFiring = false;
	bRecoveringRecoil = true;

	GetWorldTimerManager().ClearTimer(FullAutoTimer);
}

//재장전
void AAssultRifle::Reload()
{
	if (bIsReloading || CurrentAmmo == MaxAmmo) return;

	bIsReloading = true;
	CanFire = false;
	bIsScoped = false;
	CurrentCharacter->GetFollowCamera()->SetFieldOfView(90.f);

	if (CurrentCharacter)
	{
		CurrentCharacter->SetUseLeftHandIK(false);
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

	if (bIsHoldingScope)
	{
		DefaultCameraRelativeLocation = CurrentCharacter->GetFollowCamera()->GetRelativeLocation();
		bSavedScopeCamera = true;
		bIsScoped = true;
		CurrentCharacter->GetFollowCamera()->SetFieldOfView(55.f);
	}
	else
	{
		bIsScoped = false;
		CurrentCharacter->GetFollowCamera()->SetFieldOfView(90.f);
	}

	if (CurrentCharacter)
	{
		CurrentCharacter->SetUseLeftHandIK(true);
	}

	SpawnMagazine();
	UE_LOG(LogTemp, Warning, TEXT("Shotgun reloaded! Ammo reset to: %d"), CurrentAmmo);
}

void AAssultRifle::Zoom(bool bIsZoom)
{
	bRecoveringRecoil = false;

	if (!CurrentCharacter || !CurrentCharacter->GetFollowCamera()) return;

	UCameraComponent* FollowCamera = CurrentCharacter->GetFollowCamera();

	bIsHoldingScope = bIsZoom;

	if (bIsReloading)
	{
		bIsScoped = false;
		FollowCamera->SetFieldOfView(90.f);
		return;
	}


	if (bIsZoom)
	{
		if (!bSavedScopeCamera)
		{
			DefaultCameraRelativeLocation = FollowCamera->GetRelativeLocation();
			bSavedScopeCamera = true;
		}
		bIsHoldingScope = true;
		bIsScoped = true;
		CurrentCharacter->GetFollowCamera()->SetFieldOfView(55.f);
	}
	else
	{
		bIsHoldingScope = false;
		bIsScoped = false;
		CurrentCharacter->GetFollowCamera()->SetFieldOfView(90.f);
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