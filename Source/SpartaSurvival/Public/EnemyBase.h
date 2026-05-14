#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"


UENUM(BlueprintType)
enum class EEnemyState : uint8
{
    Idle,
    Chase,
    Attack,
    Death
};

UCLASS()
class SPARTASURVIVAL_API AEnemyBase : public ACharacter
{

    GENERATED_BODY()

public:
    AEnemyBase();

protected:
    virtual void BeginPlay() override;

    
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Die();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnDeathAnimationFinished();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackCheck();

public:
   
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHP = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentHP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 10.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EEnemyState EnemyState = EEnemyState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsAttacking = false;

    // --- 애니메이션 관련 ---
    UPROPERTY(EditAnywhere, Category = "Effects")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, Category = "Effects")
    UAnimMontage* DeathMontage;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual void Attack();

    UFUNCTION()
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};