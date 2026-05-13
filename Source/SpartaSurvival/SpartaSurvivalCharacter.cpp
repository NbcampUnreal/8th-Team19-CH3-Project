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

//총기 사용 해더
#include "Components/SceneComponent.h"
#include "GunController.h"
#include "DefaultGun.h"
#include "Shotgun.h"
#include "UObject/ConstructorHelpers.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ASpartaSurvivalCharacter

ASpartaSurvivalCharacter::ASpartaSurvivalCharacter()
{
	// Tick 활성화 (매 프레임 상태 갱신에 필요)
	PrimaryActorTick.bCanEverTick = true;

	//총기 잡는 그립 소캣생성
	WeaponSocket = CreateDefaultSubobject<USceneComponent>(TEXT("GripPoint"));
	WeaponSocket->SetupAttachment(GetMesh(), TEXT("hand_r"));
	WeaponSocket->SetRelativeLocation(FVector::ZeroVector);
	WeaponSocket->SetRelativeRotation(WeaponBaseRot);

	SupportSocket = CreateDefaultSubobject<USceneComponent>(TEXT("SupportPoint"));
	SupportSocket->SetupAttachment(GetMesh(), TEXT("hand_l"));
	SupportSocket->SetRelativeLocation(FVector::ZeroVector);
	SupportSocket->SetRelativeRotation(FRotator::ZeroRotator);

	// ─── 콜리전 캡슐 ───────────────────────────────────────────────
	GetCapsuleComponent()->InitCapsuleSize(DefaultCapsuleRadius, DefaultCapsuleHalfHeight);

	// ─── 컨트롤러 회전 설정 ────────────────────────────────────────
	// 컨트롤러 회전을 캐릭터 회전에 직접 반영하지 않습니다.
	// 카메라 붐만 컨트롤러를 따라 회전하고, 캐릭터는 이동 방향을 향합니다.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// ─── 이동 컴포넌트 설정 ────────────────────────────────────────
	UCharacterMovementComponent* MovComp = GetCharacterMovement();
	MovComp->bOrientRotationToMovement = false;           // 이동 방향으로 캐릭터 회전
	MovComp->RotationRate = FRotator(0.f, 500.f, 0.f);
	MovComp->JumpZVelocity = 700.f;
	MovComp->AirControl = 0.35f;
	MovComp->MaxWalkSpeed = WalkSpeed;     // 걷기 속도로 초기화
	MovComp->MinAnalogWalkSpeed = 20.f;
	MovComp->BrakingDecelerationWalking = 2000.f;
	MovComp->BrakingDecelerationFalling = 1500.f;
	MovComp->NavAgentProps.bCanCrouch = true;  // 언리얼 내장 크라우치 활성화

	// ─── 카메라 붐 ─────────────────────────────────────────────────
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true; // 컨트롤러(마우스)에 따라 붐 회전

	// ─── 팔로우 카메라 ─────────────────────────────────────────────
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // 붐 기준으로 고정, 별도 회전 없음

	//총기 관련 // shotgunbp 알려주기 
	static ConstructorHelpers::FClassFinder<AShotgun> ShotgunClass(
		TEXT("/Game/Blueprints/BP_Shotgun")
	);

	if (ShotgunClass.Succeeded())
	{
		ShotgunBP = ShotgunClass.Class;
	}
}

//////////////////////////////////////////////////////////////////////////
// 라이프사이클

void ASpartaSurvivalCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 초기 캡슐 크기 및 이동 속도 적용
	AdjustCapsuleSize();
	ApplyMovementSpeed();

	if (!ShotgunBP)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShotgunBP is null."));
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;

	AShotgun* SpawnedShotgun = GetWorld()->SpawnActor<AShotgun>(
		ShotgunBP,
		GetActorLocation(),
		GetActorRotation(),
		Params
	);

	SpawnedShotgun->EquipToCharacter(this);


}

void ASpartaSurvivalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 매 프레임 이동 상태를 갱신합니다.
	UpdateMovementState();

	// ── 앉기 시 카메라 높이 부드럽게 보간 ────────────────────────
	// 목표 Z 오프셋: 앉으면 CrouchCameraZOffset, 일어서면 0
	float TargetCameraZ = bIsCrouched ? CrouchCameraZOffset : 0.f;
	FVector CurrentOffset = CameraBoom->GetRelativeLocation();
	float SmoothedZ = FMath::FInterpTo(CurrentOffset.Z, TargetCameraZ, DeltaTime, CrouchCameraInterpSpeed);
	CameraBoom->SetRelativeLocation(FVector(CurrentOffset.X, CurrentOffset.Y, SmoothedZ));

	if (WeaponSocket)
	{
		FRotator TargetRot = WeaponBaseRot; 
		if (bIsInAir) { TargetRot = WeaponJumpOffsetRot + WeaponBaseRot; }
		else if (bWeaponMovePose) { TargetRot = WeaponMoveOffsetRot + WeaponBaseRot; }
		else TargetRot = WeaponBaseRot;

		WeaponSocket->SetRelativeRotation(
			FMath::RInterpTo(
				WeaponSocket->GetRelativeRotation(),
				TargetRot,
				DeltaTime,
				WeaponRotInterpSpeed
			)
		);
	}
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
			if (WeaponMappingContext)
			{
				Subsystem->AddMappingContext(WeaponMappingContext, 1);
			}
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
		// SprintAction은 언리얼 에디터에서 Input Action 에셋을 새로 만들고
		// DefaultMappingContext에 Left Shift 키를 매핑해야 합니다.
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

		//총기 액션
		if (FireAction)
		{
			EIC->BindAction(FireAction, ETriggerEvent::Started, this, &ASpartaSurvivalCharacter::Fire);
		}

		if (ReloadAction)
		{
			EIC->BindAction(ReloadAction, ETriggerEvent::Started, this, &ASpartaSurvivalCharacter::Reload);
		}

		if (ZoomAction)
		{
			EIC->BindAction(ZoomAction, ETriggerEvent::Started, this, &ASpartaSurvivalCharacter::StartZoom);
			EIC->BindAction(ZoomAction, ETriggerEvent::Completed, this, &ASpartaSurvivalCharacter::EndZoom);
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
	bWeaponMovePose = true;

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
	bWeaponMovePose = false;
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
	Crouch();               // 언리얼 내장 크라우치 (Is Crouching = true)
}

void ASpartaSurvivalCharacter::StopCrouch()
{
	bCrouchKeyHeld = false;
	UnCrouch();             // 언리얼 내장 크라우치 해제
}

//////////////////////////////////////////////////////////////////////////
// 이동 상태 관리

void ASpartaSurvivalCharacter::UpdateMovementState()
{
	// ── 공중 여부 확인 ────────────────────────────────────────────
	bIsInAir = GetCharacterMovement()->IsFalling();

	// ── 수평 이동 속력 계산 ───────────────────────────────────────
	FVector Velocity   = GetVelocity();
	Velocity.Z         = 0.f;
	CurrentSpeed       = Velocity.Size();
	bIsMoving          = CurrentSpeed > 10.f;

	// ── 언리얼 내장 크라우치 상태를 직접 읽습니다 ─────────────────
	// bCrouchKeyHeld 대신 엔진이 관리하는 bIsCrouched 사용
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

void ASpartaSurvivalCharacter::ApplyMovementSpeed()
{
	float TargetSpeed;
	if      (MovementState == EMovementState::Sprinting)  TargetSpeed = SprintSpeed;
	else if (MovementState == EMovementState::Crouching)  TargetSpeed = CrouchSpeed;
	else                                                   TargetSpeed = WalkSpeed;

	GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
}

void ASpartaSurvivalCharacter::AdjustCapsuleSize()
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!Capsule) return;

	// 크라우치 상태일 때는 언리얼 내장 Crouch()가 캡슐을 자동 관리하므로
	// 여기서 건드리지 않습니다.
	if (bIsCrouching) return;

	if (MovementState == EMovementState::Sprinting)
	{
		Capsule->SetCapsuleSize(SprintCapsuleRadius, SprintCapsuleHalfHeight, true);
	}
	else
	{
		Capsule->SetCapsuleSize(DefaultCapsuleRadius, DefaultCapsuleHalfHeight, true);
	}
}

void ASpartaSurvivalCharacter::Landed(const FHitResult& Hit)
{
	// 착지 직후 즉시 상태를 갱신합니다 (다음 Tick 전에 올바른 상태를 보장).
	bIsInAir = false;
	UpdateMovementState();
}

//총기 액션 함수들 ───────────────────────────────────────

void ASpartaSurvivalCharacter::SetEquippedGun(ADefaultGun* ToBeEquippedGun)
{
	EquippedGun = ToBeEquippedGun;
}
void ASpartaSurvivalCharacter::Fire()
{
	if (EquippedGun)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fire called. EquippedGun: %s / Class: %s"),
			*EquippedGun->GetName(),
			*EquippedGun->GetClass()->GetName()
		);

		EquippedGun->Fire();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Fire called, but EquippedGun is NULL."));
	}
}
void ASpartaSurvivalCharacter::Reload()
{
	UE_LOG(LogTemp, Warning, TEXT("Character Reload called"));

	if (EquippedGun)
	{
		EquippedGun->Reload();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Reload failed: EquippedGun NULL"));
	}
}
void ASpartaSurvivalCharacter::StartZoom()
{
	if (EquippedGun)
	{
		EquippedGun->Zoom(true);
	}
}

void ASpartaSurvivalCharacter::EndZoom()
{
	if (EquippedGun)
	{
		EquippedGun->Zoom(false);
	}
}

