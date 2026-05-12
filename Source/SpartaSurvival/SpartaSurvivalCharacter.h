// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "SpartaSurvivalCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

//총기관련 클래스 전방선언
class USceneComponent;
class ADefaultGun;
class AShotgun;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 * 캐릭터 이동 상태를 나타내는 열거형
 * 애니메이션 블루프린트와 이동 속도 제어에 활용됩니다.
 */
UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Idle       UMETA(DisplayName = "Idle"),
	Walking    UMETA(DisplayName = "Walking"),
	Crouching  UMETA(DisplayName = "Crouching"),
	Sprinting  UMETA(DisplayName = "Sprinting"),
	Jumping    UMETA(DisplayName = "Jumping"),
	Falling    UMETA(DisplayName = "Falling"),
};

UCLASS(config=Game)
class ASpartaSurvivalCharacter : public ACharacter
{
	GENERATED_BODY()

	/** 카메라 붐 (캐릭터 뒤에 카메라 위치) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** 3인칭 카메라 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** 기본 입력 매핑 컨텍스트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** 점프 입력 액션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** 이동 입력 액션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** 시점 조작 입력 액션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** 달리기 입력 액션 (Shift) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** 앉기 입력 액션 (Ctrl) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

public:
	ASpartaSurvivalCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ─── 입력 핸들러 ───────────────────────────────────────────────

	/** WASD 이동 */
	void Move(const FInputActionValue& Value);

	/** 이동 입력 종료 */
	void StopMove();

	/** 마우스 시점 회전 */
	void Look(const FInputActionValue& Value);

	/** 달리기 시작 (Shift 누름) */
	void StartSprint();

	/** 달리기 종료 (Shift 뗌) */
	void StopSprint();

	/** 앉기 시작 (Ctrl 누름) */
	void StartCrouch();

	/** 앉기 종료 (Ctrl 뗌) */
	void StopCrouch();

	// ─── 이동 상태 관리 ────────────────────────────────────────────

	/** 현재 이동 상태를 갱신하고 관련 수치를 적용합니다 */
	void UpdateMovementState();

	/** 이동 상태에 따라 캡슐 크기를 조정합니다 */
	void AdjustCapsuleSize();

	/** 이동 상태에 따른 최대 이동 속도를 적용합니다 */
	void ApplyMovementSpeed();

protected:
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** 착지 이벤트 (점프/낙하 종료 시 호출) */
	virtual void Landed(const FHitResult& Hit) override;

public:
	// ─── 이동 속도 설정값 (블루프린트에서 조정 가능) ──────────────

	/** 걷기 속도 (기본값: 500) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed")
	float WalkSpeed = 500.f;

	/** 달리기 속도 (기본값: 900) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed")
	float SprintSpeed = 900.f;

	/** 앉기 속도 (기본값: 250) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed")
	float CrouchSpeed = 250.f;

	// ─── 캡슐 크기 설정값 (블루프린트에서 조정 가능) ──────────────

	/** 기본(걷기/정지) 상태의 캡슐 반지름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Capsule")
	float DefaultCapsuleRadius = 42.f;

	/** 기본(걷기/정지) 상태의 캡슐 절반 높이 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Capsule")
	float DefaultCapsuleHalfHeight = 96.f;

	/** 달리기 상태의 캡슐 반지름 (약간 좁아져 기동성 향상) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Capsule")
	float SprintCapsuleRadius = 38.f;

	/** 달리기 상태의 캡슐 절반 높이 (약간 낮아져 기동성 향상) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Capsule")
	float SprintCapsuleHalfHeight = 90.f;

	/** 앉기 상태의 캡슐 반지름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Capsule")
	float CrouchCapsuleRadius = 42.f;

	/** 앉기 상태의 캡슐 절반 높이 (절반으로 낮아짐) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Capsule")
	float CrouchCapsuleHalfHeight = 55.f;

	/** 앉기 시 카메라 붐 Z 오프셋 (기본값: -40) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera")
	float CrouchCameraZOffset = -40.f;

	/** 카메라 높이 보간 속도 (클수록 빠름, 기본값: 10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera")
	float CrouchCameraInterpSpeed = 10.f;

	// ─── 애니메이션용 읽기 전용 상태 프로퍼티 ─────────────────────

	/** 현재 이동 상태 (애니메이션 블루프린트에서 참조) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	EMovementState MovementState = EMovementState::Idle;

	/** 달리기 중 여부 (애니메이션 블루프린트에서 참조) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsSprinting = false;

	/** 앉기 중 여부 (애니메이션 블루프린트에서 참조) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsCrouching = false;

	/** 이동 중 여부 (애니메이션 블루프린트에서 참조) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsMoving = false;

	/** 공중에 있는지 여부 (애니메이션 블루프린트에서 참조) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsInAir = false;

	/** 수평 이동 속력 (애니메이션 블루프린트 BlendSpace에서 참조) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	float CurrentSpeed = 0.f;

private:
	/** 달리기 키 입력 중 여부 */
	bool bSprintKeyHeld = false;

	/** 앉기 키 입력 중 여부 */
	bool bCrouchKeyHeld = false;

	/** 이동 입력이 들어오고 있는지 여부 */
	bool bHasMovementInput = false;

	/** 앞 방향(+Y) 이동 입력이 들어오고 있는지 여부 */
	bool bIsMovingForward = false;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE EMovementState GetMovementState() const { return MovementState; }



//총기 관련 프로퍼티와 함수들

public:
	void SetEquippedGun(ADefaultGun* NewGun);

	USceneComponent* GetWeaponSocket() const { return WeaponSocket; }
	USceneComponent* GetSupportSocket() const { return SupportSocket; }

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Gun")
	AShotgun* StartingShotgun = nullptr;


protected:
	//샷건 bp 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
	TSubclassOf<AShotgun> ShotgunBP;

	//현재 장착된 총기 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
	ADefaultGun* EquippedGun = nullptr;

	//component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
	USceneComponent* WeaponSocket = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
	USceneComponent* SupportSocket = nullptr;


	// 총기 액션 함수들 
	void Fire();
	void Reload();
	void StartZoom();
	void EndZoom();

private:
	bool bWeaponMovePose = false;

	FRotator WeaponBaseRot = FRotator(-5.f, 170.f, 0.f);
	FRotator WeaponMoveOffsetRot = FRotator(15.f, -22.f, 0.f);
	FRotator WeaponJumpOffsetRot = FRotator(15.f, -22.f, 0.f); 

	float WeaponRotInterpSpeed = 10.f;
};
