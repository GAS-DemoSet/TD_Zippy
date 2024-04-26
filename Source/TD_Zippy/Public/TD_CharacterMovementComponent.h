// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TD_CharacterMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TD_ZIPPY_API UTD_CharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FTD_SavedMove_Character : public FSavedMove_Character
	{
		typedef FSavedMove_Character Super;
		
	public:
		uint8 Saved_bWantsToSprint:1;

		FTD_SavedMove_Character();

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FTD_NetworkPredictionData_Client_Character : public FNetworkPredictionData_Client_Character
	{
		typedef FNetworkPredictionData_Client_Character Super;
		
	public:
		FTD_NetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement);

		virtual FSavedMovePtr AllocateNewMove() override;
	};

public:
	// Sets default values for this component's properties
	UTD_CharacterMovementComponent();

	// ~Begin UCharacterMovementComponent Interface
	/** 重写预测 */
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	// ~End UCharacterMovementComponent Interface


protected:
	// ~Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// ~End UActorComponent Interface
	
	// ~Begin UCharacterMovementComponent Interface
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	// ~End UCharacterMovementComponent Interface

public:
	bool Safe_bWantsToSprint;
};
