// Fill out your copyright notice in the Description page of Project Settings.


#include "SpartaSurvivalPlayerController.h"
#include "Blueprint/UserWidget.h"





ASpartaSurvivalPlayerController::ASpartaSurvivalPlayerController()
	: MainMenuWidgetClass(nullptr)
	, MainMenuWidgetInstance(nullptr)
	, HUDWidgetClass(nullptr)
	, HUDWidgetInstance(nullptr)
	, DamageWidgetClass(nullptr)
	, DamageWidgetInstance(nullptr)
	, PauseMenuWidgetClass(nullptr)
	, PauseMenuWidgetInstance(nullptr)
	, LevelUpWidgetClass(nullptr)
	, LevelUpWidgetInstance(nullptr)
{
	// 생성자 초기화 로직 (없다면 비워두어도 됩니다)
}

void ASpartaSurvivalPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FString CurrentMapName = GetWorld()->GetMapName();
	if (CurrentMapName.Contains("MainMenuLevel"))
	{
		ShowMainMenu(false);
	}
	else
	{
		ShowGameHUD();
	}
}

UUserWidget* ASpartaSurvivalPlayerController::GetHUDWidget() const
{
	return HUDWidgetInstance;
}
void ASpartaSurvivalPlayerController::ShowMainMenu(bool bIsRestart)
{
	// HUD가 켜져 있다면 닫기
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (MainMenuWidgetClass)
	{
		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport();

			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());
		}

		
	}

}
void ASpartaSurvivalPlayerController::ShowGameHUD()
{
	// 1. 기존에 떠 있던 메뉴 위젯 찌꺼기 완벽하게 청소
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	// 2. 원래 되던 조작 상태(게임 입력 모드)로 완벽하게 리셋
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	SetShowMouseCursor(false);

	// 3. UI에 빼앗겼던 윈도우 마우스 포커스를 강제로 게임 화면으로 회수
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetAllUserFocusToGameViewport();
	}

	// 4. 인게임 플레이용 HUD 생성 및 화면 출력
	if (HUDWidgetClass)
	{
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->RemoveFromParent();
			HUDWidgetInstance = nullptr;
		}

		HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
		}
	}
}
void ASpartaSurvivalPlayerController::ShowPauseMenu()
{

}
void ASpartaSurvivalPlayerController::ShowLevelUp()
{

}
void ASpartaSurvivalPlayerController::StartGame()
{

}
