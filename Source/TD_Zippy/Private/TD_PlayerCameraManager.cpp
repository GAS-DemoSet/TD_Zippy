﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "TD_PlayerCameraManager.h"

#include <Components/CapsuleComponent.h>

#include "TD_CharacterMovementComponent.h"
#include "TD_ZippyCharacter.h"

ATD_PlayerCameraManager::ATD_PlayerCameraManager()
{
}

void ATD_PlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);

	if (ATD_ZippyCharacter* ZippyCharacter = Cast<ATD_ZippyCharacter>(GetOwningPlayerController()->GetPawn()))
	{
		UTD_CharacterMovementComponent* ZMC = ZippyCharacter->GetTD_CharacterMovement();
		FVector TargetCrouchOffset = FVector(
			0,
			0,
			ZMC->GetCrouchedHalfHeight() - ZippyCharacter->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		);
		FVector Offset = FMath::Lerp(FVector::ZeroVector, TargetCrouchOffset, FMath::Clamp(CrouchBlendTime / CrouchBlendDuration, 0.f, 1.f));

		if (ZMC->IsCrouching())
		{
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime + DeltaTime, 0.f, CrouchBlendDuration);
			Offset -= TargetCrouchOffset;
		}
		else
		{
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime - DeltaTime, 0.f, CrouchBlendDuration);
		}

		OutVT.POV.Location += Offset;
	}
}