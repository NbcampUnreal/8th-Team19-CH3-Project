#include "EnemyBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h" 
#include "EnemyAIController.h"
#include "DrawDebugHelpers.h"
#include "SpartaSurvival/SpartaSurvivalCharacter.h"
#include "Engine/DamageEvents.h"
#include "SpartaSurvivalGameState.h"

AEnemyBase::AEnemyBase()
{
    PrimaryActorTick.bCanEverTick = false;

    CurrentHP = MaxHP;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
    bUseControllerRotationYaw = false;
}

void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();
    CurrentHP = MaxHP;
}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (EnemyState == EEnemyState::Death) return 0.f;

   
    float FinalDamage = DamageAmount;

   
    if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
       
        const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);

        if (PointDamageEvent)
        {
            
            if (PointDamageEvent->HitInfo.BoneName.ToString().ToLower() == TEXT("Head"))
            {
                FinalDamage *= 3.0f;
                UE_LOG(LogTemp, Warning, TEXT("💥 헤드샷!!! 3배 데미지 적용: %f"), FinalDamage);
            }
        }
    }

   
    float ActualDamage = Super::TakeDamage(FinalDamage, DamageEvent, EventInstigator, DamageCauser);

    
    CurrentHP = FMath::Clamp(CurrentHP - ActualDamage, 0.f, MaxHP);
    UE_LOG(LogTemp, Log, TEXT("[%s] 남은 체력: %f / %f"), *GetName(), CurrentHP, MaxHP);

    if (CurrentHP <= 0.f)
    {
        Die();
    }

    return ActualDamage;
}

void AEnemyBase::Attack()
{
    if (EnemyState == EEnemyState::Death) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    
    if (AttackMontage && AnimInstance && !AnimInstance->Montage_IsPlaying(AttackMontage))
    {
        bIsAttacking = true;

        AnimInstance->Montage_Play(AttackMontage);

        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &AEnemyBase::OnAttackMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
    }
}

void AEnemyBase::Die()
{
    EnemyState = EEnemyState::Death;

    GetCharacterMovement()->DisableMovement();

    if (DeathMontage)
    {
        PlayAnimMontage(DeathMontage);
    }

    if (GetWorld())
    {
        ASpartaSurvivalGameState* MyGameState = Cast<ASpartaSurvivalGameState>(GetWorld()->GetGameState());

        if (MyGameState)
        {
            MyGameState->AddPlayerEXP(10.0f);
        }
    }
}

void AEnemyBase::OnDeathAnimationFinished()
{
    if (IsValid(this))
    {
        Destroy();
    }
}

void AEnemyBase::AttackCheck()
{
    
    TArray<FHitResult> HitResults;

    FVector Start = GetActorLocation() + GetActorForwardVector() * 50.f;
    FVector End = Start + GetActorForwardVector() * 50.f;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

   
    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        Start,
        End,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(SphereRadius),
        Params
    );

    FColor DebugColor = FColor::Green;

    if (bHit)
    {
        for (const FHitResult& It : HitResults)
        {
            AActor* HitActor = It.GetActor();
            if (HitActor)
            {

                ASpartaSurvivalCharacter* Player = Cast<ASpartaSurvivalCharacter>(HitActor);
                if (Player)
                {
                    DebugColor = FColor::Red;

                    UGameplayStatics::ApplyDamage(
                        Player,
                        AttackDamage,
                        GetController(),
                        this,
                        nullptr
                    );

                    break;
                }
            }
        }
    }
}

void AEnemyBase::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    bIsAttacking = false;

    AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController());
    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    if (AIC && PlayerChar)
    {
        FVector Dir = PlayerChar->GetActorLocation() - GetActorLocation();
        Dir.Z = 0.f;
        FRotator TargetRot = Dir.Rotation();

        struct FRotationHelper
        {
            int32 Steps = 0;
            FTimerHandle TimerHandle;
        };

        TSharedPtr<FRotationHelper> Helper = MakeShareable(new FRotationHelper());

        TWeakObjectPtr<AEnemyBase> WeakThis(this);

        GetWorldTimerManager().SetTimer(Helper->TimerHandle, [WeakThis, TargetRot, Helper]() {

         
            if (!WeakThis.IsValid())
            {
                return;
            }

            if (Helper->Steps >= 100)
            {
               
                if (WeakThis->GetWorld())
                {
                    WeakThis->GetWorld()->GetTimerManager().ClearTimer(Helper->TimerHandle);
                }
                return;
            }

           
            FRotator NewRot = FMath::RInterpTo(WeakThis->GetActorRotation(), TargetRot, 0.01f, 7.f);
            WeakThis->SetActorRotation(NewRot);

            Helper->Steps++;
            }, 0.01f, true);

        AIC->ChasePlayer();
    }
}
