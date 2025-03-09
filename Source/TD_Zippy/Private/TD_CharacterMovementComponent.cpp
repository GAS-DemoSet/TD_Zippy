// Fill out your copyright notice in the Description page of Project Settings.


#include "TD_CharacterMovementComponent.h"

#include "TD_LogDefine.h"
#include "GameFramework/Character.h"

UTD_CharacterMovementComponent::FTD_SavedMove_Character::FTD_SavedMove_Character()
	: Super()
	, Saved_bWantsToSprint(0)
{
}

bool UTD_CharacterMovementComponent::FTD_SavedMove_Character::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const
{
	const FTD_SavedMove_Character* NewSaveMove = static_cast<FTD_SavedMove_Character*>(NewMovePtr.Get());

	if (Saved_bWantsToSprint != NewSaveMove->Saved_bWantsToSprint)
	{
		return false;
	}
	
	return Super::CanCombineWith(NewMovePtr, InCharacter, MaxDelta);
}

void UTD_CharacterMovementComponent::FTD_SavedMove_Character::Clear()
{
	Super::Clear();

	Saved_bWantsToSprint = 0;
}

uint8 UTD_CharacterMovementComponent::FTD_SavedMove_Character::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	
	if (Saved_bWantsToSprint)
	{
		Result |= FLAG_Custom_0;
	}
	
	return Result;
}

void UTD_CharacterMovementComponent::FTD_SavedMove_Character::SetMoveFor(ACharacter* C, float InDeltaTime,
	FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	if (const UTD_CharacterMovementComponent* TempCMC = Cast<UTD_CharacterMovementComponent>(C->GetCharacterMovement()))
	{
		UE_LOG(TD_Log_CMC, Warning, TEXT("SetMoveFor:: %i"), (int32)TempCMC->GetOwner()->GetLocalRole());
		Saved_bWantsToSprint = TempCMC->Safe_bWantsToSprint;
	}
}

void UTD_CharacterMovementComponent::FTD_SavedMove_Character::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	if (UTD_CharacterMovementComponent* TempCMC = Cast<UTD_CharacterMovementComponent>(C->GetCharacterMovement()))
	{
		UE_LOG(TD_Log_CMC, Warning, TEXT("PrepMoveFor:: %i"), (int32)TempCMC->GetOwner()->GetLocalRole());
		TempCMC->Safe_bWantsToSprint = Saved_bWantsToSprint;
	}
}
