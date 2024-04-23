// Copyright Epic Games, Inc. All Rights Reserved.

#include "TD_ZippyGameMode.h"
#include "TD_ZippyCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATD_ZippyGameMode::ATD_ZippyGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
