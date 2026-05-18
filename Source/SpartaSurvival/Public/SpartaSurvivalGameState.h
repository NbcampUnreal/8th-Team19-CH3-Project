// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SpartaSurvivalGameState.generated.h"

/**
 * 
 */
UCLASS()
class SPARTASURVIVAL_API ASpartaSurvivalGameState : public AGameStateBase
{	
    GENERATED_BODY()
	
public:
    ASpartaSurvivalGameState();

    // ----- UI(위젯) 비율 계산용 함수 -----
    UFUNCTION(BlueprintCallable, Category = "Stats|UI")
    float GetPlayerHPPercent() const { return (MaxHP > 0.f) ? (CurrentHP / MaxHP) : 0.f; }

    UFUNCTION(BlueprintCallable, Category = "Stats|UI")
    float GetPlayerEXPPercent() const { return (MaxEXP > 0.f) ? (CurrentEXP / MaxEXP) : 0.f; }

    UFUNCTION(BlueprintCallable, Category = "Stats|UI")
    int32 GetPlayerLevel() const { return PlayerLevel; }

    // ----- 캐릭터나 외부에서 데이터를 더해줄 함수들 -----
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void AddPlayerHP(float Amount) { CurrentHP = FMath::Clamp(CurrentHP + Amount, 0.f, MaxHP); }

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void AddPlayerEXP(float Amount); 

public:
    // 플레이어 관련
    //레벨
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 PlayerLevel = 1;

    //체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHP = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentHP;

    //경험치
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxEXP = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentEXP = 0.f;

    //점수
    UFUNCTION(BlueprintCallable, Category = "GameSystem")
    void AddScore(int32 Amount);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
    int32 CurrentScore = 0;

};
