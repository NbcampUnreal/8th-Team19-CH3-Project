#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
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
class AAssultRifle;
class AThrowableBase;
class AGrenade;

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

UCLASS(config = Game)
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

	/**
	 * 이동 상태에 따라 캡슐 크기를 조정합니다.
	 * 크라우치 상태는 언리얼 내장 OnStartCrouch / OnEndCrouch 콜백에서 처리하므로
	 * 여기서는 일반(기본/달리기) 상태만 담당합니다.
	 */
	void AdjustCapsuleSize();

	/** 이동 상태에 따른 최대 이동 속도를 적용합니다 */
	void ApplyMovementSpeed();

	/**
	 * 주어진 상태에 해당하는 이동 속도를 반환합니다.
	 * ApplyMovementSpeed 및 크라우치 콜백에서 공통으로 사용됩니다.
	 */
	float GetMovementSpeedForState(EMovementState State) const;

	// ─── 언리얼 내장 크라우치 콜백 오버라이드 ─────────────────────

	/**
	 * 언리얼 엔진이 Crouch() 처리를 완료한 직후 호출됩니다.
	 * CrouchCapsuleHalfHeight / CrouchCapsuleRadius 값을 여기서 적용합니다.
	 */
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	/**
	 * 언리얼 엔진이 UnCrouch() 처리를 완료한 직후 호출됩니다.
	 * 기본 캡슐 크기를 복원하고 이동 속도를 재적용합니다.
	 */
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	// ─── 발소리 ────────────────────────────────────────────────────

	/**
	 * 매 Tick에서 호출되어 이동 상태와 경과 시간을 기반으로
	 * 발소리를 자동 재생합니다.
	 */
	void TryPlayFootstep(float DeltaTime);

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

	/** 기본(걷기/정지) 상태의 캡슐 반지름 — Manny 메시 기본값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Capsule")
	float DefaultCapsuleRadius = 34.f;

	/** 기본(걷기/정지) 상태의 캡슐 절반 높이 — Manny 메시 기본값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Capsule")
	float DefaultCapsuleHalfHeight = 88.f;

	/** 달리기 상태의 캡슐 반지름 — Manny 기본보다 약간 좁게 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Capsule")
	float SprintCapsuleRadius = 34.f;

	/** 달리기 상태의 캡슐 절반 높이 — Manny 기본과 동일 (달리기 시 자세 변화 없음) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Capsule")
	float SprintCapsuleHalfHeight = 88.f;

	/**
	 * 앉기 상태의 캡슐 반지름
	 * BeginPlay에서 CharacterMovement에 주입되어 내장 Crouch()가 이 값을 사용합니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Capsule")
	float CrouchCapsuleRadius = 34.f;

	/**
	 * 앉기 상태의 캡슐 절반 높이 — Manny 기본(88)의 절반
	 * BeginPlay에서 CharacterMovement->CrouchedHalfHeight에 주입됩니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Capsule")
	float CrouchCapsuleHalfHeight = 44.f;

	/** 앉기 시 카메라 붐 Z 오프셋 — 캡슐이 낮아진 만큼 카메라도 내려옴 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera")
	float CrouchCameraZOffset = -44.f;

	/** 카메라 높이 보간 속도 (클수록 빠름, 기본값: 10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Camera")
	float CrouchCameraInterpSpeed = 10.f;

	// ─── 발소리 설정값 (블루프린트에서 조정 가능) ─────────────────

	/** 재생할 발소리 사운드 에셋 — BP_ThirdPersonCharacter에서 할당 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Footstep")
	USoundBase* FootstepSound;

	/** 걷기 발소리 재생 간격 (초, 기본값: 0.5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Footstep")
	float WalkStepInterval = 0.5f;

	/** 달리기 발소리 재생 간격 (초, 기본값: 0.3) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Footstep")
	float SprintStepInterval = 0.3f;

	/** 앉기 발소리 재생 간격 (초, 기본값: 0.75) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Footstep")
	float CrouchStepInterval = 0.75f;

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

	/** 발소리 타이머 누적값 */
	float FootstepTimer = 0.f;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE EMovementState GetMovementState() const { return MovementState; }



	//총기 관련 프로퍼티와 함수들
	//총기 액션
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
	UInputMappingContext* WeaponMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
	UInputAction* FireAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
	UInputAction* ReloadAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
	UInputAction* ZoomAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
	UInputAction* MeleeAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable")
	UInputAction* ThrowAction;
public:

	//손붙이기 왼손
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FVector LeftHandIKLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "IK")
	bool bUseLeftHandIK = false;

public:
	USceneComponent* GetWeaponSocket() const { return WeaponSocket; }
	//USceneComponent* GetSupportSocket() const { return SupportSocket; }

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Gun")
	AShotgun* StartingShotgun = nullptr;

	void SetUseLeftHandIK(bool bBlock);
	//bool bBlockLeftHandIK = false;

protected:
	//샷건 bp 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
	TSubclassOf<AShotgun> ShotgunBP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
	TSubclassOf<AAssultRifle> AssultRifleBP;

	//현재 장착된 총기 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
	ADefaultGun* EquippedGun = nullptr;

	//component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
	USceneComponent* WeaponSocket = nullptr;

	//현재 장착된 투척물
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throwable")
	AThrowableBase* EquippedThrowable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throwable")
	TSubclassOf<AGrenade> GrenadeBP;

public:
	void SetEquippedThrowable(AThrowableBase* NewThrowable);
	void SetEquippedGun(ADefaultGun* NewGun);

	// 총기 액션 함수들 
	void Fire();
	void EndFire();
	void Reload();
	void StartZoom();
	void EndZoom();
	void Melee();

	void ReadyToThrow();
	void Throw();

private:
	//bool bWeaponMovePose = false;

	FRotator WeaponBaseRot = FRotator(-5.f, 170.f, 0.f);
	//FRotator WeaponMoveOffsetRot = FRotator(13.f, -30.f, 50.f);
	//FRotator WeaponJumpOffsetRot = FRotator(10.f, -22.f, 50.f);

	//float WeaponRotInterpSpeed = 10.f;
};
