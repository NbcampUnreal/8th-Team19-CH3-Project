// Fill out your copyright notice in the Description page of Project Settings.
#include "Shotgun.h"
#include "MainCharacter.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"


AShotgun::AShotgun()
{
	ZoomMultiplier = 2.f;
	ReloadDuration = 1.5f;
	MaxAmmo = 8;
	CurrentAmmo = MaxAmmo;
	CanFire = true;

	//샷건 bp 클래스 로드
	static ConstructorHelpers::FClassFinder<AShotgun> ShotgunBPClass(
		TEXT("/Game/Blueprints/BP_Shotgun")
	);

	if (ShotgunBPClass.Succeeded())
	{
		ShotgunBP = ShotgunBPClass.Class;
	}
}

//마지막으로 맞은 액터 반환
AActor* AShotgun::GetHitActor() const 
{
	return LastHitActor;
}

//샷건 장착
void AShotgun::EquipToCharacter(AMainCharacter* Character)
{
	if (!Character || !ShotgunBP) return;

	AShotgun* SpawnedShotgun = GetWorld()->SpawnActor<AShotgun>(
		ShotgunBP,
		GetActorLocation(),
		GetActorRotation()
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