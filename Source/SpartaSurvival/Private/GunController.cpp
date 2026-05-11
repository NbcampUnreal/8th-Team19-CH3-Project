
#include "GunController.h"
#include "EnhancedInputSubsystems.h"

AGunController::AGunController()
	:InputMappingContext(nullptr), FireAction(nullptr), ReloadAction(nullptr), ZoomAction(nullptr)
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
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

