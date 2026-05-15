// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Engine/StaticMeshActor.h"
#include "Camera/CameraShakeBase.h"
#include "Animation/AnimationAsset.h"
#include "Sound/SoundBase.h"
#include "Components/BillboardComponent.h"
#include "Animation/AnimMontage.h"
#include "CoreMinimal.h"
#include "DefaultGun.h"
#include "AssultRifle.generated.h"



class EnemyBase;
class ASpartaSurvivalCharacter;
class TimerManager;

UCLASS()
class SPARTASURVIVAL_API AAssultRifle : public ADefaultGun
{
	GENERATED_BODY()
private:
	AActor* LastHitActor; // 마지막으로 충돌한 액터를 저장하는 변수

	FTimerHandle ReloadTimer;// 재장전 타이머 핸들
	FTimerHandle MeleeTimer; // 근접 공격 타이머 핸들

	void EndReload();
	void EndMelee();

	float AssultRifleRange;
	float DamagePerBullet;
	float SpreadAngle;

protected:
	ASpartaSurvivalCharacter* CurrentCharacter;

	// muzzleflash
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	UBillboardComponent* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TArray<UTexture2D*> MuzzleFlashTextures;

	FTimerHandle MuzzleFlashTimer;
	int32 MuzzleFlashIndex = 0;

	// animation reload
	bool bIsReloading = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* ReloadMontage;

	//animation melee
	bool bIsMeleeing = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* MeleeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	USoundBase* ReloadSound;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TSubclassOf<UCameraShakeBase> FireCameraShake;

	UPROPERTY(EditAnywhere, Category = "Magazine")
	UStaticMesh* MagazineMesh;

	UPROPERTY(EditAnywhere, Category = "Magazine")
	float MagazineLifeTime = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Magazine")
	USceneComponent* MagazineSpawnPoint;

	UPROPERTY()
	AStaticMeshActor* CurrentMagazineActor;

	void SpawnMagazine();
	void DropMagazine();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BulletHit")
	UMaterialInterface* BulletHitMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BulletHit")
	FVector BulletHitSize = FVector(8.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BulletHit")
	float BulletHitLifeTime = 5.f;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	UUserWidget* CrosshairWidget;

	void ShowCrosshair();
	void HideCrosshair();

public:
	AAssultRifle();

	USceneComponent* GetGripPoint() const { return GripPoint; }
	USceneComponent* GetSupportPoint() const { return SupportPoint; }
	USceneComponent* GetSupportPointMoving() const { return SupportPointMoving; }
	USkeletalMeshComponent* GetGunMesh() const { return GunMesh; }

public:
	virtual void Fire() override;
	virtual void Reload() override;
	virtual void Zoom(bool bIsZoom) override;
	virtual void EquipToCharacter(ASpartaSurvivalCharacter* Character) override;
	virtual void Melee() override;

	//ui나 외부에서 필요한 함수!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
public:
	int32 GetCurrentAmmo() const { return CurrentAmmo; }
	AActor* GetHitActor() const { return LastHitActor; };

	bool IsReloading() const { return bIsReloading; }
	bool IsCanFire() const { return CanFire; }

};