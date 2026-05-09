#include "EnemyBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h" 
#include "EnemyAIController.h"
#include "DrawDebugHelpers.h"
#include "SpartaSurvival/SpartaSurvivalCharacter.h"
#include "Engine/DamageEvents.h"

AEnemyBase::AEnemyBase()
{
    PrimaryActorTick.bCanEverTick = false;

    CurrentHP = MaxHP;

    GetCharacterMovement()->bOrientRotationToMovement = true;
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

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    FPointDamageEvent* PointDamageEvent = (FPointDamageEvent*)&DamageEvent;

    if (PointDamageEvent)
    {
        if (PointDamageEvent->HitInfo.BoneName == FName("Head"))
        {
            ActualDamage *= 3.0f;
            UE_LOG(LogTemp, Warning, TEXT("헤드샷!!!"));
        }
    }
    CurrentHP = FMath::Clamp(CurrentHP - ActualDamage, 0.f, MaxHP);

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
}

void AEnemyBase::OnDeathAnimationFinished()
{
    if (!IsPendingKillPending())
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

    float SphereRadius = 60.f;

   
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

    DrawDebugSphere(GetWorld(), End, SphereRadius, 16, DebugColor, false, 0.5f);
}

void AEnemyBase::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    bIsAttacking = false;

    AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController());
    if (AIC)
    {
        AIC->ChasePlayer();
    }
}