// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SpartaSurvivalPlayerController.generated.h"


/**ASpartaSurvivalPlayerController
 * 
 */


 /**
  *
  */
UCLASS()
class SPARTASURVIVAL_API ASpartaSurvivalPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ASpartaSurvivalPlayerController();


    // // 메인메뉴 UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MainMenu")
    TSubclassOf<UUserWidget> MainMenuWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MainMenu")
    UUserWidget* MainMenuWidgetInstance;

    // // HUD UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
    TSubclassOf<UUserWidget> HUDWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD")
    UUserWidget* HUDWidgetInstance;

    // // 데미지 UI
   /** UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    TSubclassOf<UUserWidget> DamageWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    UUserWidget* DamageWidgetInstance;*/

    // // 일시정지 메뉴 UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pause")
    TSubclassOf<UUserWidget> PauseMenuWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pause")
    UUserWidget* PauseMenuWidgetInstance;

    // // 레벨업 선택 UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LevelUp")
    TSubclassOf<UUserWidget> LevelUpWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LevelUp")
    UUserWidget* LevelUpWidgetInstance;

    //게임 오버
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameOver")
    TSubclassOf<UUserWidget> GameOverWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameOver")
    UUserWidget* GameOverWidgetInstance;

    //게임 오버
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameClear")
    TSubclassOf<UUserWidget> GameClearWidgetClass;

    //게임클리어
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameClear")
    UUserWidget* GameClearWidgetInstance;
    
    //적 피격 확인
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    TSubclassOf<class UUserWidget> DamageWidgetClass;
    void ShowDamageText(FVector WorldLocation);

    UFUNCTION(BlueprintPure, Category = "HUD")
    UUserWidget* GetHUDWidget() const;


    // 메인 메뉴 표시
    UFUNCTION(BlueprintCallable, Category = "MainMenu")
    void ShowMainMenu(bool bIsRestart);

    // HUD 표시
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowGameHUD();

    //  일시정지 메뉴 UI
    UFUNCTION(BlueprintCallable, Category = "Pause")
    void ShowPauseMenu();

    //  레벨업 선택 UI
    UFUNCTION(BlueprintCallable, Category = "LevelUp")
    void ShowLevelUp();

    // 게임 시작
   // UFUNCTION(BlueprintCallable, Category = "MainMenu")
   // void StartGame();

     //게임오버
    UFUNCTION(BlueprintCallable, Category = "GameOver")
    void ShowGameOver();
    
    //게임 클리어
    UFUNCTION(BlueprintCallable, Category = "GameClear")
    void ShowGameClear();

protected:

    virtual void BeginPlay() override;


};
