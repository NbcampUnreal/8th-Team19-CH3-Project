// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpartaSurvivalGameMode.h"
#include "SpartaSurvivalCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASpartaSurvivalGameMode::ASpartaSurvivalGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
