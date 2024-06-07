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

	class FTD_NetworkPredictionData_Server_Character : public FNetworkPredictionData_Server_Character
	{
		typedef FNetworkPredictionData_Server_Character Super;

	public:
		FTD_NetworkPredictionData_Server_Character(const UCharacterMovementComponent& ServerMovement);
	};

public:
	// Sets default values for this component's properties
	UTD_CharacterMovementComponent();

	// ~Begin UCharacterMovementComponent Interface
	/** 重写预测 */
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual FNetworkPredictionData_Server* GetPredictionData_Server() const override;
	// ~End UCharacterMovementComponent Interface


	/** 触发冲刺 */
	UFUNCTION(BlueprintCallable)
	void SprintPressed();
	/** 结束冲刺 */
	UFUNCTION(BlueprintCallable)
	void SprintReleased();

	UFUNCTION(BlueprintCallable)
	void CrouchPressed();
	UFUNCTION(BlueprintCallable)
	void CrouchReleased();

protected:
	// ~Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// ~End UActorComponent Interface
	
	// ~Begin UCharacterMovementComponent Interface
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	// ~End UCharacterMovementComponent Interface

public:
	/** 冲刺状态 */
	bool Safe_bWantsToSprint;

private:
	/** 冲刺时最大速度 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Sprint", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float Sprint_MaxWalkSpeed;

	/** 行走最大速度 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Sprint", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float Walk_MaxWalkSpeed;
};
