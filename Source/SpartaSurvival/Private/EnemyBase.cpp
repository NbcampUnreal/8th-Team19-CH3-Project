#include "EnemyBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h" 

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
        PlayAnimMontage(AttackMontage);

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
    FHitResult HitResult;
    FVector Start = GetActorLocation() + GetActorForwardVector() * 50.f;
    FVector End = Start + GetActorForwardVector() * 100.f;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    
    bool bHit = GetWorld()->SweepSingleByChannel(
        HitResult, Start, End, FQuat::Identity,
        ECC_Pawn, FCollisionShape::MakeSphere(50.f), Params
    );

    if (bHit && HitResult.GetActor())
    {
        
        UGameplayStatics::ApplyDamage(
            HitResult.GetActor(),
            AttackDamage,         
            GetController(),      
            this,                 
            nullptr               
        );
    }
}

void AEnemyBase::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    bIsAttacking = false;
}