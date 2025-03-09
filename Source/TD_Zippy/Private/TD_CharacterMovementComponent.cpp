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

UTD_CharacterMovementComponent::FTD_NetworkPredictionData_Client_Character::FTD_NetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UTD_CharacterMovementComponent::FTD_NetworkPredictionData_Client_Character::AllocateNewMove()
{
	return MakeShared<FTD_SavedMove_Character>();
	// return FSavedMovePtr(new FTD_SavedMove_Character());
}

UTD_CharacterMovementComponent::FTD_NetworkPredictionData_Server_Character::FTD_NetworkPredictionData_Server_Character(const UCharacterMovementComponent& ServerMovement)
	: Super(ServerMovement)
{
}

UTD_CharacterMovementComponent::UTD_CharacterMovementComponent()
{
}

void UTD_CharacterMovementComponent::SprintPressed()
{
	Safe_bWantsToSprint = true;
}

void UTD_CharacterMovementComponent::SprintReleased()
{
	Safe_bWantsToSprint = false;
}

FNetworkPredictionData_Client* UTD_CharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)

	if (ClientPredictionData == nullptr)
	{
		UTD_CharacterMovementComponent* MutableThis = const_cast<UTD_CharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FTD_NetworkPredictionData_Client_Character(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f; 
	}
	return ClientPredictionData;
	
	// return Super::GetPredictionData_Client();
}

FNetworkPredictionData_Server* UTD_CharacterMovementComponent::GetPredictionData_Server() const
{
	check(PawnOwner != nullptr)
	
	if (ServerPredictionData == nullptr)
	{
		UTD_CharacterMovementComponent* MutableThis = const_cast<UTD_CharacterMovementComponent*>(this);
		MutableThis->ServerPredictionData = new FTD_NetworkPredictionData_Server_Character(*this);
	}
	return ServerPredictionData;
	// return Super::GetPredictionData_Server();
}

void UTD_CharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void UTD_CharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	
	if (MovementMode == MOVE_Walking)
	{
		if (Safe_bWantsToSprint)
		{
			MaxWalkSpeed = Sprint_MaxWalkSpeed;
		}
		else
		{
			MaxWalkSpeed = Walk_MaxWalkSpeed;
		}
	}
}
