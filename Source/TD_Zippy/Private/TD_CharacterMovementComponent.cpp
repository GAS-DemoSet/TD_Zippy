// Fill out your copyright notice in the Description page of Project Settings.


#include "TD_CharacterMovementComponent.h"

#include "TD_LogDefine.h"
#include "GameFramework/Character.h"


UTD_CharacterMovementComponent::FTD_SavedMove_Character::FTD_SavedMove_Character()
	: Super()
	, Saved_bWantsToSprint(0)
{
}

bool UTD_CharacterMovementComponent::FTD_SavedMove_Character::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FTD_SavedMove_Character* Temp_SaveMove = static_cast<FTD_SavedMove_Character*>(NewMove.Get());

	if (Saved_bWantsToSprint != Temp_SaveMove->Saved_bWantsToSprint)
	{
		return false;
	}
	
	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
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

	if (UTD_CharacterMovementComponent* TempCMC = Cast<UTD_CharacterMovementComponent>(C->GetCharacterMovement()))
	{
		//UE_LOG(TD_Log, Warning, TEXT("SetMoveFor:: %i"), (int32)TempCMC->GetOwner()->GetLocalRole());
		Saved_bWantsToSprint = TempCMC->Safe_bWantsToSprint;
	}
}

void UTD_CharacterMovementComponent::FTD_SavedMove_Character::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	if (UTD_CharacterMovementComponent* TempCMC = Cast<UTD_CharacterMovementComponent>(C->GetCharacterMovement()))
	{
		//UE_LOG(TD_Log, Warning, TEXT("PrepMoveFor:: %i"), (int32)TempCMC->GetOwner()->GetLocalRole());
		TempCMC->Safe_bWantsToSprint = Saved_bWantsToSprint;
	}
}

UTD_CharacterMovementComponent::FTD_NetworkPredictionData_Client_Character::FTD_NetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UTD_CharacterMovementComponent::FTD_NetworkPredictionData_Client_Character::AllocateNewMove()
{
	return FSavedMovePtr(new UTD_CharacterMovementComponent::FTD_SavedMove_Character());
}

UTD_CharacterMovementComponent::FTD_NetworkPredictionData_Server_Character::FTD_NetworkPredictionData_Server_Character(const UCharacterMovementComponent& ServerMovement)
	: Super(ServerMovement)
{
}

// Sets default values for this component's properties
UTD_CharacterMovementComponent::UTD_CharacterMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
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
}

void UTD_CharacterMovementComponent::SprintPressed()
{
	Safe_bWantsToSprint = true;
}

void UTD_CharacterMovementComponent::SprintReleased()
{
	Safe_bWantsToSprint = false;
}

void UTD_CharacterMovementComponent::CrouchPressed()
{
	bWantsToCrouch = !bWantsToCrouch;
}

void UTD_CharacterMovementComponent::CrouchReleased()
{
}

// Called when the game starts
void UTD_CharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTD_CharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                   FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
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
			UE_LOG(TD_Log, Warning, TEXT("11::%s : %f--%i"), *GetOwner()->GetName(), MaxWalkSpeed, (int32)GetOwner()->GetLocalRole());
		}
		else
		{
			MaxWalkSpeed = Walk_MaxWalkSpeed;
			UE_LOG(TD_Log, Warning, TEXT("22::%s : %f--%i"), *GetOwner()->GetName(), MaxWalkSpeed, (int32)GetOwner()->GetLocalRole());
		}
	}
	UE_LOG(TD_Log, Warning, TEXT("--------------------------------------------"))
}

