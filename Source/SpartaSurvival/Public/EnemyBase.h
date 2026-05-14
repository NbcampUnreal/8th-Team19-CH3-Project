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

    // 외부에서 사용하기 위해 public 변경
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
    virtual void BeginPlay() override;

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

    // --- ¾Ö´Ï¸ÞÀÌ¼Ç °ü·Ã ---
    UPROPERTY(EditAnywhere, Category = "Effects")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, Category = "Effects")
    UAnimMontage* DeathMontage;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual void Attack();

    UFUNCTION()
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};