// throwable cpp
#include "ThrowableBase.h"
#include "DrawDebugHelpers.h"
#include "../SpartaSurvivalCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"

#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"


// Sets default values
AThrowableBase::AThrowableBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ThrowableRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ThrowableRoot"));
	SetRootComponent(ThrowableRoot);

	ThrowableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ThrowableMesh"));
	ThrowableMesh->SetupAttachment(ThrowableRoot);

	static ConstructorHelpers::FObjectFinder<USoundBase> PinSoundAsset(
		TEXT("/Game/GunMeshes/PinPullSound.PinPullSound")
	);

	if (PinSoundAsset.Succeeded())
	{
		PinPullSound = PinSoundAsset.Object;
	}

}
void AThrowableBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AThrowableBase::StartExplosionTimer()
{
	GetWorldTimerManager().SetTimer(
		ExplosionTimerHandle,
		this,
		&AThrowableBase::Explode,
		ExplosionDelay,
		false
	);
}
void AThrowableBase::StopRolling()
{
	ThrowableMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
	ThrowableMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
}
void AThrowableBase::Charging()
{
	if (!GetWorld()) return;
	GetWorldTimerManager().ClearTimer(ChargingTimerHandle);

	GetWorld()->GetTimerManager().SetTimer(
		ChargingTimerHandle,
		[this]()
		{
			if (!bIsChargingThrow)
			{
				GetWorldTimerManager().ClearTimer(ChargingTimerHandle);
				return;
			}
			
			const float DeltaTime = GetWorld()->GetDeltaSeconds();
			ThrowChargeTime += DeltaTime;

			float ChargeRatio = FMath::Clamp(ThrowChargeTime / MaxChargeTime, 0.f, 1.f);
			float CurrentThrowPower = FMath::Lerp(MinThrowPower, MaxThrowPower, ChargeRatio);

			APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
			if (!PlayerController) return;

			FVector CharacterLocation;
			FRotator CharacterRotation;
			PlayerController->GetPlayerViewPoint(CharacterLocation, CharacterRotation);

			//debugline
			FVector ThrowDirection = CharacterRotation.Vector();

			ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
			if (!OwnerCharacter || !OwnerCharacter->GetMesh()) return;

			FVector StartPoint =
				OwnerCharacter->GetMesh()->GetSocketLocation(TEXT("hand_r"));

			FVector LaunchVelocity = ThrowDirection * CurrentThrowPower; //velocity vector
			float GravityZ = GetWorld()->GetGravityZ();

			const int32 SegmentCount = 30;
			const float TimeStep = 0.07f;

			FVector PreviousPoint = StartPoint;

			for (int32 i = 1; i <= SegmentCount; i++)
			{
				float Time = i * TimeStep;

				FVector CurrentPoint =
					StartPoint
					+ LaunchVelocity * Time
					+ FVector(0.f, 0.f, 0.5f * GravityZ * Time * Time);

				DrawDebugLine(
					GetWorld(),
					PreviousPoint,
					CurrentPoint,
					FColor::Green,
					false,
					0.0f,
					0,
					3.f
				);

				PreviousPoint = CurrentPoint;
			}
		},
		.016f,
		true
	);
}
void AThrowableBase::ThrowPressed()
{
	if (bIsChargingThrow) return; //이미 던지는 중일경우 return

	if (PinPullSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PinPullSound,
			GetActorLocation()
		);
	}

	bIsChargingThrow = true;
	ThrowChargeTime = 0.f; //reset
	Charging();
}
//던지는 힘 물리 계산
void AThrowableBase::ThrowReleased()
{
	if (!bIsChargingThrow) return;

	bIsChargingThrow = false;
	/*Clamp = 값이 범위 밖으로 못 나가게 막음
	Lerp = 두 값 사이에서 중간값을 계산함*/

	float ChargeRatio = FMath::Clamp(ThrowChargeTime / MaxChargeTime, 0.f, 1.f);
	float ThrowPower = FMath::Lerp(MinThrowPower, MaxThrowPower, ChargeRatio);

	UPrimitiveComponent* Primitive = ThrowableMesh;
	if (!Primitive) return;

	ThrowableMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	Primitive->SetMobility(EComponentMobility::Movable);
	Primitive->SetCollisionProfileName(TEXT("PhysicsActor"));
	Primitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Primitive->SetSimulatePhysics(true); 
	Primitive->SetEnableGravity(true);
	Primitive->WakeRigidBody();

	FVector ThrowDirection = GetActorForwardVector(); //pc 실패전 보험

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		FVector CharacterLocation;
		FRotator CharacterRotation;

		PlayerController->GetPlayerViewPoint(CharacterLocation, CharacterRotation);
		ThrowDirection = CharacterRotation.Vector();
	}

	// 빠른 물체 충돌 관통 방지
	Primitive->SetUseCCD(true);
	Primitive->WakeAllRigidBodies();
	Primitive->AddImpulse(ThrowDirection * ThrowPower, NAME_None, true); //throw
	
	StartExplosionTimer();

	GetWorldTimerManager().SetTimer(
		StopTimerHandle,
		this,
		&AThrowableBase::StopRolling,
		0.2f,
		false
	);

	ThrowChargeTime = 0.f;

}

void AThrowableBase::Throw(bool bReadyToThrow)
{
	if (bReadyToThrow)
	{
		ThrowPressed();
	}
	else
	{
		ThrowReleased();
	}
}
//damage
void AThrowableBase::OnHit()
{
}
void AThrowableBase::Explode()
{
}
//장착했을때 필요한 것 적기
void AThrowableBase::EquipToCharacter(ASpartaSurvivalCharacter* Character)
{
}