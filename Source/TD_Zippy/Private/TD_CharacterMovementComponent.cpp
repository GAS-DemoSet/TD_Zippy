// Fill out your copyright notice in the Description page of Project Settings.


#include "TD_CharacterMovementComponent.h"

#include "TD_LogDefine.h"
#include "TD_ZippyCharacter.h"
#include "Components/CapsuleComponent.h"
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
		Saved_bPrevWantsToCrouch = TempCMC->Safe_bPrevWantsToCrouch;
	}
}

void UTD_CharacterMovementComponent::FTD_SavedMove_Character::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	if (UTD_CharacterMovementComponent* TempCMC = Cast<UTD_CharacterMovementComponent>(C->GetCharacterMovement()))
	{
		//UE_LOG(TD_Log, Warning, TEXT("PrepMoveFor:: %i"), (int32)TempCMC->GetOwner()->GetLocalRole());
		TempCMC->Safe_bWantsToSprint = Saved_bWantsToSprint;
		TempCMC->Safe_bPrevWantsToCrouch = Saved_bPrevWantsToCrouch;
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
	NavAgentProps.bCanCrouch = true;
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

bool UTD_CharacterMovementComponent::IsCustomMovementMode(ETD_CustomMovementMode InMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InMovementMode; 
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
		}
		else
		{
			MaxWalkSpeed = Walk_MaxWalkSpeed;
		}
	}

	Safe_bPrevWantsToCrouch = bWantsToCrouch;
}

void UTD_CharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if (MovementMode == MOVE_Walking && !bWantsToCrouch && Safe_bPrevWantsToCrouch)
	{
		FHitResult PotentialSlideSurface;
		if (Velocity.SizeSquared() > pow(Slide_MinSpeed, 2) && GetSlideSurface(PotentialSlideSurface))
		{
			EnterSlide();
		}
	}

	if (IsCustomMovementMode(CMOVE_Slide) && !bWantsToCrouch)
	{
		SetMovementMode(MOVE_Walking);
	}

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UTD_CharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		PhysSlide(deltaTime, Iterations);
		break;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
		break;
	}
}

bool UTD_CharacterMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide);
}

bool UTD_CharacterMovementComponent::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}

void UTD_CharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	ZippyCharacterOwner = Cast<ATD_ZippyCharacter>(GetOwner());
}

void UTD_CharacterMovementComponent::EnterSlide()
{
	bWantsToCrouch = true;
	Velocity += Velocity.GetSafeNormal() *Slide_EnterImpulse;
	SetMovementMode(MOVE_Custom, CMOVE_Slide);
}

void UTD_CharacterMovementComponent::ExitSlide()
{
	bWantsToCrouch = false;
	FQuat NewRotator = FRotationMatrix::MakeFromZX(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), FVector::UpVector).ToQuat();
	FHitResult Hit;
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotator, true, Hit);
	SetMovementMode(MOVE_Walking);
}

void UTD_CharacterMovementComponent::PhysSlide(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
		return;

	RestorePreAdditiveRootMotionVelocity();

	FHitResult SurfaceHit;
	if (!GetSlideSurface(SurfaceHit) || Velocity.SizeSquared() < pow(Slide_MinSpeed, 2))
	{
		ExitSlide();
		StartNewPhysics(DeltaTime, Iterations);
		return;
	}

	Velocity += Slide_GravityForce * FVector::DownVector * DeltaTime; // v += a * dt

	if (FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector())) > .5f)
	{
		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
	}
	else
	{
		Acceleration = FVector::ZeroVector;
	}

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		CalcVelocity(DeltaTime, Slide_Friction, true, GetMaxBrakingDeceleration());
	}
	ApplyRootMotionToVelocity(DeltaTime);

	Iterations++;
	bJustTeleported = false;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FQuat OldRotator = UpdatedComponent->GetComponentRotation().Quaternion();
	FHitResult Hit(1.f);
	FVector Adjusted = Velocity * DeltaTime; // X = V * dt
	FVector VelPlaneDir = FVector::VectorPlaneProject(Velocity, SurfaceHit.Normal).GetSafeNormal();
	FQuat NewRotator = FRotationMatrix::MakeFromZX(VelPlaneDir, SurfaceHit.Normal).ToQuat();

	SafeMoveUpdatedComponent(Adjusted, NewRotator, true, Hit);

	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, DeltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1 - Hit.Time), Hit.Normal, Hit, true);
	}

	FHitResult NewSurfaceHit;
	if (!GetSlideSurface(NewSurfaceHit) || Velocity.SizeSquared() < pow(Slide_MinSpeed, 2))
	{
		ExitSlide();
	}

	// Update Outgoing Velocity & Acceleration
	if( !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / DeltaTime; // V = dx / dt
	}
}

bool UTD_CharacterMovementComponent::GetSlideSurface(FHitResult& OutHitResult) const
{
	const FVector TStart = UpdatedComponent->GetComponentLocation();
	const FVector TEnd = TStart + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2 * FVector::DownVector;
	FName ProfileName = TEXT("BlockAll");
	return this->GetWorld()->LineTraceSingleByProfile(OutHitResult, TStart, TEnd, ProfileName, ZippyCharacterOwner->GetIgnoreCharacterParams());
}

