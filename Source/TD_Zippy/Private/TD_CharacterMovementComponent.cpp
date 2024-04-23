// Fill out your copyright notice in the Description page of Project Settings.


#include "TD_CharacterMovementComponent.h"

#include "GameFramework/Character.h"


bool UTD_CharacterMovementComponent::FSavedMove_TD_Character::CanCombineWith(const FSavedMovePtr& NewMove,
                                                                             ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_TD_Character* Temp_SaveMove = static_cast<FSavedMove_TD_Character*>(NewMove.Get());

	if (Saved_bWantsToSprint != Temp_SaveMove->Saved_bWantsToSprint)
	{
		return false;
	}
	
	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UTD_CharacterMovementComponent::FSavedMove_TD_Character::Clear()
{
	Super::Clear();

	Saved_bWantsToSprint = 0;
}

uint8 UTD_CharacterMovementComponent::FSavedMove_TD_Character::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (Saved_bWantsToSprint)
	{
		Result |= FLAG_Custom_0;
	}
	
	return Result;
}

void UTD_CharacterMovementComponent::FSavedMove_TD_Character::SetMoveFor(ACharacter* C, float InDeltaTime,
	FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	if (UTD_CharacterMovementComponent* TempCMC = Cast<UTD_CharacterMovementComponent>(C->GetCharacterMovement()))
	{
		Saved_bWantsToSprint = TempCMC->Safe_bWantsToSprint;
	}
}

void UTD_CharacterMovementComponent::FSavedMove_TD_Character::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	if (UTD_CharacterMovementComponent* TempCMC = Cast<UTD_CharacterMovementComponent>(C->GetCharacterMovement()))
	{
		TempCMC->Safe_bWantsToSprint = Saved_bWantsToSprint;
	}
}

// Sets default values for this component's properties
UTD_CharacterMovementComponent::UTD_CharacterMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
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

