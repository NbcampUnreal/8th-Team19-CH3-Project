
#include "GunController.h"
#include "EnhancedInputSubsystems.h"

AGunController::AGunController()
	:WeaponMappingContext(nullptr), FireAction(nullptr), ReloadAction(nullptr), ZoomAction(nullptr)
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGunController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(WeaponMappingContext, 1);
		}
	}
}

