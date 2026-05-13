// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpartaSurvivalCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"  // 발소리 재생에 사용

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ASpartaSurvivalCharacter

ASpartaSurvivalCharacter::ASpartaSurvivalCharacter()
{
	// Tick 활성화 (매 프레임 상태 갱신에 필요)
	PrimaryActorTick.bCanEverTick = true;

	// ─── 콜리전 캡슐 ───────────────────────────────────────────────
	// Manny 메시 기본값: 반지름 34, 절반 높이 88
	GetCapsuleComponent()->InitCapsuleSize(DefaultCapsuleRadius, DefaultCapsuleHalfHeight);

	// ─── 컨트롤러 회전 설정 ────────────────────────────────────────
	// 컨트롤러 회전을 캐릭터 회전에 직접 반영하지 않습니다.
	// 카메라 붐만 컨트롤러를 따라 회전하고, 캐릭터는 이동 방향을 향합니다.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	// ─── 이동 컴포넌트 설정 ────────────────────────────────────────
	UCharacterMovementComponent* MovComp = GetCharacterMovement();
	MovComp->bOrientRotationToMovement = true;           // 이동 방향으로 캐릭터 회전
	MovComp->RotationRate               = FRotator(0.f, 500.f, 0.f);
	MovComp->JumpZVelocity              = 700.f;
	MovComp->AirControl                 = 0.35f;
	MovComp->MaxWalkSpeed               = WalkSpeed;     // 걷기 속도로 초기화
	MovComp->MinAnalogWalkSpeed         = 20.f;
	MovComp->BrakingDecelerationWalking = 2000.f;
	MovComp->BrakingDecelerationFalling = 1500.f;
	MovComp->NavAgentProps.bCanCrouch   = true;          // 언리얼 내장 크라우치 활성화

	// ─── 카메라 붐 ─────────────────────────────────────────────────
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength         = 400.f;
	CameraBoom->bUsePawnControlRotation = true; // 컨트롤러(마우스)에 따라 붐 회전

	// ─── 팔로우 카메라 ─────────────────────────────────────────────
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // 붐 기준으로 고정, 별도 회전 없음
}

//////////////////////////////////////////////////////////////////////////
// 라이프사이클

void ASpartaSurvivalCharacter::BeginPlay()
{
	Super::BeginPlay();

	// ── 앉기 캡슐 크기를 CharacterMovement에 주입 ─────────────────
	// 언리얼 내장 Crouch()는 CharacterMovement->CrouchedHalfHeight 값을 기준으로
	// 캡슐을 자동 조정합니다. UPROPERTY 값을 여기서 덮어써 커스텀 크기가 적용되도록 합니다.
	if (UCharacterMovementComponent* MovComp = GetCharacterMovement())
	{
		MovComp->CrouchedHalfHeight = CrouchCapsuleHalfHeight;
	}

	// 초기 캡슐 크기 및 이동 속도 적용
	AdjustCapsuleSize();
	ApplyMovementSpeed();
}

void ASpartaSurvivalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 매 프레임 이동 상태를 갱신합니다.
	UpdateMovementState();

	// ── 발소리 재생 ───────────────────────────────────────────────
	TryPlayFootstep(DeltaTime);

	// ── 앉기 시 카메라 높이 부드럽게 보간 ────────────────────────
	// 목표 Z 오프셋: 앉으면 CrouchCameraZOffset, 일어서면 0
	float TargetCameraZ = bIsCrouched ? CrouchCameraZOffset : 0.f;
	FVector CurrentOffset = CameraBoom->GetRelativeLocation();
	float SmoothedZ = FMath::FInterpTo(CurrentOffset.Z, TargetCameraZ, DeltaTime, CrouchCameraInterpSpeed);
	CameraBoom->SetRelativeLocation(FVector(CurrentOffset.X, CurrentOffset.Y, SmoothedZ));
}

//////////////////////////////////////////////////////////////////////////
// 입력 설정

void ASpartaSurvivalCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ASpartaSurvivalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// ── 점프 ──────────────────────────────────────────────────
		EIC->BindAction(JumpAction, ETriggerEvent::Started,   this, &ACharacter::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// ── 이동 ──────────────────────────────────────────────────
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpartaSurvivalCharacter::Move);
		EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &ASpartaSurvivalCharacter::StopMove);

		// ── 시점 조작 ─────────────────────────────────────────────
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpartaSurvivalCharacter::Look);

		// ── 달리기 (Shift) ────────────────────────────────────────
		if (SprintAction)
		{
			EIC->BindAction(SprintAction, ETriggerEvent::Started,   this, &ASpartaSurvivalCharacter::StartSprint);
			EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASpartaSurvivalCharacter::StopSprint);
		}
		else
		{
			UE_LOG(LogTemplateCharacter, Warning,
				TEXT("[%s] SprintAction이 설정되지 않았습니다. "
				     "에디터에서 SprintAction Input Action 에셋을 할당해 주세요."),
				*GetNameSafe(this));
		}

		// ── 앉기 (Ctrl) ───────────────────────────────────────────
		if (CrouchAction)
		{
			EIC->BindAction(CrouchAction, ETriggerEvent::Started,   this, &ASpartaSurvivalCharacter::StartCrouch);
			EIC->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ASpartaSurvivalCharacter::StopCrouch);
		}
		else
		{
			UE_LOG(LogTemplateCharacter, Warning,
				TEXT("[%s] CrouchAction이 설정되지 않았습니다. "
				     "에디터에서 CrouchAction Input Action 에셋을 할당해 주세요."),
				*GetNameSafe(this));
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error,
			TEXT("[%s] EnhancedInputComponent를 찾을 수 없습니다!"), *GetNameSafe(this));
	}
}

//////////////////////////////////////////////////////////////////////////
// 입력 핸들러

void ASpartaSurvivalCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	bHasMovementInput = !MovementVector.IsNearlyZero();

	// 앞 방향(+Y)으로 이동 중인 경우에만 달리기를 허용합니다.
	bIsMovingForward = MovementVector.Y > 0.f;

	if (Controller && bHasMovementInput)
	{
		// 컨트롤러 Yaw 방향 기준으로 전/우 벡터를 계산합니다.
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector  ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector  RightDir   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDir, MovementVector.Y);
		AddMovementInput(RightDir,   MovementVector.X);
	}
}

void ASpartaSurvivalCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxis = Value.Get<FVector2D>();
	if (Controller)
	{
		AddControllerYawInput(LookAxis.X);
		AddControllerPitchInput(LookAxis.Y);
	}
}

void ASpartaSurvivalCharacter::StopMove()
{
	bHasMovementInput = false;
	bIsMovingForward  = false;
}

void ASpartaSurvivalCharacter::StartSprint()
{
	bSprintKeyHeld = true;
}

void ASpartaSurvivalCharacter::StopSprint()
{
	bSprintKeyHeld = false;
}

void ASpartaSurvivalCharacter::StartCrouch()
{
	bCrouchKeyHeld = true;
	bSprintKeyHeld = false; // 앉기 시작 시 달리기 강제 해제
	Crouch();               // 언리얼 내장 크라우치 → OnStartCrouch 콜백 트리거
}

void ASpartaSurvivalCharacter::StopCrouch()
{
	bCrouchKeyHeld = false;
	UnCrouch();             // 언리얼 내장 크라우치 해제 → OnEndCrouch 콜백 트리거
}

//////////////////////////////////////////////////////////////////////////
// 언리얼 내장 크라우치 콜백

void ASpartaSurvivalCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	// 부모 호출: 엔진이 캡슐을 CrouchedHalfHeight(= CrouchCapsuleHalfHeight)로 줄이고
	// 메시 오프셋 등을 자동 보정합니다.
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	// ── 캡슐 반지름 커스텀 적용 ─────────────────────────────────
	// 엔진은 반지름을 건드리지 않으므로 직접 설정합니다.
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		// HalfHeight는 이미 엔진이 CrouchedHalfHeight로 설정했으므로 덮어쓰지 않습니다.
		// Radius만 커스텀 값으로 변경합니다.
		Capsule->SetCapsuleRadius(CrouchCapsuleRadius, true);
	}

	// ── 크라우치 이동 속도 적용 ──────────────────────────────────
	if (UCharacterMovementComponent* MovComp = GetCharacterMovement())
	{
		MovComp->MaxWalkSpeedCrouched = CrouchSpeed;
	}

	UE_LOG(LogTemplateCharacter, Verbose,
		TEXT("[%s] OnStartCrouch: Capsule(R=%.1f, HH=%.1f) Speed=%.1f"),
		*GetNameSafe(this), CrouchCapsuleRadius, CrouchCapsuleHalfHeight, CrouchSpeed);
}

void ASpartaSurvivalCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	// 부모 호출: 엔진이 캡슐을 원래 크기(HalfHeight)로 복원하고 메시 오프셋을 되돌립니다.
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	// ── 캡슐 반지름 복원 ─────────────────────────────────────────
	// 엔진은 반지름을 복원하지 않으므로 직접 기본값으로 되돌립니다.
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->SetCapsuleRadius(DefaultCapsuleRadius, true);
	}

	// ── 일어선 뒤 이동 상태 즉시 재갱신 ─────────────────────────
	// UpdateMovementState()가 다음 Tick에 호출되기 전에 속도가 올바르게 적용되도록
	// 즉시 갱신합니다.
	UpdateMovementState();

	UE_LOG(LogTemplateCharacter, Verbose,
		TEXT("[%s] OnEndCrouch: Capsule(R=%.1f, HH=%.1f) 복원"),
		*GetNameSafe(this), DefaultCapsuleRadius, DefaultCapsuleHalfHeight);
}

//////////////////////////////////////////////////////////////////////////
// 발소리

void ASpartaSurvivalCharacter::TryPlayFootstep(float DeltaTime)
{
	// 공중이거나 이동 중이 아니면 타이머를 리셋하고 종료합니다.
	if (bIsInAir || !bIsMoving)
	{
		FootstepTimer = 0.f;
		return;
	}

	FootstepTimer += DeltaTime;

	// 이동 상태에 따라 발소리 간격을 결정합니다.
	float Interval = WalkStepInterval;
	if (MovementState == EMovementState::Sprinting) Interval = SprintStepInterval;
	if (MovementState == EMovementState::Crouching) Interval = CrouchStepInterval;

	if (FootstepTimer >= Interval)
	{
		FootstepTimer = 0.f;

		if (FootstepSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FootstepSound, GetActorLocation());
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// 이동 상태 관리

void ASpartaSurvivalCharacter::UpdateMovementState()
{
	// ── 공중 여부 확인 ────────────────────────────────────────────
	bIsInAir = GetCharacterMovement()->IsFalling();

	// ── 수평 이동 속력 계산 ───────────────────────────────────────
	FVector Velocity = GetVelocity();
	Velocity.Z       = 0.f;
	CurrentSpeed     = Velocity.Size();
	bIsMoving        = CurrentSpeed > 10.f;

	// ── 언리얼 내장 크라우치 상태를 직접 읽습니다 ─────────────────
	bIsCrouching = bIsCrouched;

	// ── 달리기 가능 여부 ──────────────────────────────────────────
	bIsSprinting = bSprintKeyHeld && bIsMovingForward && !bIsInAir && !bIsCrouching;

	// ── 상태 결정 ─────────────────────────────────────────────────
	EMovementState PrevState = MovementState;

	if (bIsInAir)
	{
		MovementState = (GetVelocity().Z > 0.f)
			? EMovementState::Jumping
			: EMovementState::Falling;
	}
	else if (bIsCrouching)
	{
		MovementState = EMovementState::Crouching;
	}
	else if (bIsSprinting)
	{
		MovementState = EMovementState::Sprinting;
	}
	else if (bIsMoving)
	{
		MovementState = EMovementState::Walking;
	}
	else
	{
		MovementState = EMovementState::Idle;
	}

	// 상태가 바뀐 경우에만 속도·캡슐을 재적용합니다.
	if (MovementState != PrevState)
	{
		ApplyMovementSpeed();
		AdjustCapsuleSize();
	}
}

float ASpartaSurvivalCharacter::GetMovementSpeedForState(EMovementState State) const
{
	switch (State)
	{
	case EMovementState::Sprinting:  return SprintSpeed;
	case EMovementState::Crouching:  return CrouchSpeed;
	default:                         return WalkSpeed;
	}
}

void ASpartaSurvivalCharacter::ApplyMovementSpeed()
{
	// ── 크라우치 상태는 MaxWalkSpeedCrouched로 별도 관리됩니다 ───
	// OnStartCrouch에서 이미 설정하므로 여기서 중복 적용하지 않습니다.
	if (MovementState == EMovementState::Crouching)
	{
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = GetMovementSpeedForState(MovementState);

	UE_LOG(LogTemplateCharacter, Verbose,
		TEXT("[%s] ApplyMovementSpeed: MaxWalkSpeed=%.1f (State=%d)"),
		*GetNameSafe(this), GetCharacterMovement()->MaxWalkSpeed, (int32)MovementState);
}

void ASpartaSurvivalCharacter::AdjustCapsuleSize()
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!Capsule) return;

	// ── 크라우치 캡슐은 OnStartCrouch / OnEndCrouch 콜백에서 처리합니다 ──
	// 여기서 건드리면 언리얼 내장 크라우치 시스템과 충돌하므로 건너뜁니다.
	if (bIsCrouching) return;

	if (MovementState == EMovementState::Sprinting)
	{
		Capsule->SetCapsuleSize(SprintCapsuleRadius, SprintCapsuleHalfHeight, true);

		UE_LOG(LogTemplateCharacter, Verbose,
			TEXT("[%s] AdjustCapsuleSize: Sprint(R=%.1f, HH=%.1f)"),
			*GetNameSafe(this), SprintCapsuleRadius, SprintCapsuleHalfHeight);
	}
	else
	{
		Capsule->SetCapsuleSize(DefaultCapsuleRadius, DefaultCapsuleHalfHeight, true);

		UE_LOG(LogTemplateCharacter, Verbose,
			TEXT("[%s] AdjustCapsuleSize: Default(R=%.1f, HH=%.1f)"),
			*GetNameSafe(this), DefaultCapsuleRadius, DefaultCapsuleHalfHeight);
	}
}

void ASpartaSurvivalCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// 착지 직후 즉시 상태를 갱신합니다 (다음 Tick 전에 올바른 상태를 보장).
	bIsInAir = false;
	UpdateMovementState();
}
