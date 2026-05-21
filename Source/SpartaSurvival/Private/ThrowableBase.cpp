// throwable cpp
#include "ThrowableBase.h"
#include "DrawDebugHelpers.h"
#include "../SpartaSurvivalCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"


// Sets default values
AThrowableBase::AThrowableBase()
{
	PrimaryActorTick.bCanEverTick = true;

	ThrowableRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ThrowableRoot"));
	SetRootComponent(ThrowableRoot);

	ThrowableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ThrowableMesh"));
	ThrowableMesh->SetupAttachment(ThrowableRoot);
}

void AThrowableBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UE_LOG(LogTemp, Warning, TEXT("Charging Tick"));

	if (!bIsChargingThrow)  return;
	//power
	//G를 누르고 있는 동안 Tick에서 ThrowChargeTime이 계속 증가 시
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

	//character collision 회피
	FVector StartPoint = GetActorLocation() + ThrowDirection * 80.f + FVector(0.f, 0.f, 20.f);
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
			0.05f,
			0,
			3.f
		);

		PreviousPoint = CurrentPoint;
	}
}
void AThrowableBase::ThrowPressed()
{
	UE_LOG(LogTemp, Warning, TEXT("ThrowPressed Called"));
	if (bIsChargingThrow) return; //이미 던지는 중일경우 return

	bIsChargingThrow = true;
	ThrowChargeTime = 0.f; //reset
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

	//physics
	if (!SpawnedThrowable) return;

	UPrimitiveComponent* Primitive = SpawnedThrowable->ThrowableMesh;
	if (!Primitive) return;

	SpawnedThrowable->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	Primitive->SetSimulatePhysics(true);
	Primitive->SetEnableGravity(true);
	Primitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
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

	Primitive->AddImpulse(ThrowDirection * ThrowPower, NAME_None, true); //throw

	ThrowChargeTime = 0.f;
	SpawnedThrowable = nullptr;
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