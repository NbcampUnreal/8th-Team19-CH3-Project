// Fill out your copyright notice in the Description page of Project Settings.


#include "SpartaSurvivalGameState.h"
#include "SpartaSurvival/SpartaSurvivalGameMode.h"





ASpartaSurvivalGameState::ASpartaSurvivalGameState()
{
    PlayerLevel = 1;
    CurrentHP = MaxHP;
    CurrentEXP = 0.f;
    PlayerAttackPoint = 100.0f;
}

void ASpartaSurvivalGameState::AddPlayerEXP(float Amount)
{
    CurrentEXP += Amount;

    // 경험치가 목표치(MaxEXP)를 넘으면 레벨업 처리
    while (CurrentEXP >= MaxEXP)
    {
        CurrentEXP -= MaxEXP; // 남은 경험치 이월

        PlayerLevel++;        // 1. 레벨 증가
       // MaxHP += 20.f;        // 2. 최대 체력 20 증가 (원하는 수치로 조절 가능)
        CurrentHP = MaxHP;    // 3. 레벨업 보상으로 체력 풀피 회복
        PlayerAttackPoint *= 1.2f;
       // MaxEXP *= 1.3f;       // 4. 다음 레벨업에 필요한 경험치 통 늘리기 (30% 증가)

        // 로그창에 레벨업 확인용 출력
        UE_LOG(LogTemp, Warning, TEXT("Level Up! Current Level: %d, MaxHP: %f"), PlayerLevel, MaxHP);

    }
}

void ASpartaSurvivalGameState::AddScore(int32 Amount)
{
    CurrentScore += Amount;

    // 로그 출력 (클래스 이름만 ASpartaSurvivalGameState로 변경됨)
    UE_LOG(LogTemp, Warning, TEXT("점수 획득: +%d | 현재 총점: %d"), Amount, CurrentScore);
}


   /*  // 3. 레벨별 체력 증가 공식 적용 (레벨이 오를수록 증가폭 상승)
        float HPBonus = PlayerLevel * 10.f; 
        MaxHP += HPBonus;     
        CurrentHP = MaxHP; */

void ASpartaSurvivalGameState::AddPlayerHP(float Amount)
{
    CurrentHP = FMath::Clamp(CurrentHP + Amount, 0.f, MaxHP);
    if (CurrentHP <= 0.0f)
    {
        if (UWorld* World = GetWorld())
        {
            // 내 진짜 게임모드 클래스로 캐스팅(Cast)합니다.
            if (ASpartaSurvivalGameMode* MyGameMode = Cast<ASpartaSurvivalGameMode>(World->GetAuthGameMode()))
            {
              
                MyGameMode->GameOver();
            }
        }
    }
}
void ASpartaSurvivalGameState::AddPlayerAttackPoint(float Amount)
{
    PlayerAttackPoint += (int32)Amount;

    UE_LOG(LogTemp, Warning, TEXT("공격력 변동: +%f | 현재 총 공격력: %d"), Amount, PlayerAttackPoint);
}

