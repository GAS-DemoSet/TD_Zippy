// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TD_CharacterMovementComponent.generated.h"

UENUM(BlueprintType)
enum ETD_CustomMovementMode : int
{
	CMOVE_None			UMETA(Hidden),
	CMOVE_Slide			UMETA(DisplayName = "Slide"),
	CMOVE_MAX			UMETA(Hidden),
};

class ATD_ZippyCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TD_ZIPPY_API UTD_CharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FTD_SavedMove_Character : public FSavedMove_Character
	{
		typedef FSavedMove_Character Super;
		
	public:
		uint8 Saved_bWantsToSprint:1;

		uint8 Saved_bPrevWantsToCrouch:1;

	public:

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

	// ~Begin UCharacterMovementComponent Interface
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;
	// ~End UCharacterMovementComponent Interface


	/** 触发冲刺 */
	UFUNCTION(BlueprintCallable)
	void SprintPressed();
	/** 结束冲刺 */
	UFUNCTION(BlueprintCallable)
	void SprintReleased();

	/** 触发蹲伏 */
	UFUNCTION(BlueprintCallable)
	void CrouchPressed();
	/** 结束蹲伏 */
	UFUNCTION(BlueprintCallable)
	void CrouchReleased();

	/** 是否为自定义模式移动 */
	UFUNCTION(BlueprintCallable)
	bool IsCustomMovementMode(ETD_CustomMovementMode InMovementMode) const;

protected:
	// ~Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// ~End UActorComponent Interface
	
	// ~Begin UCharacterMovementComponent Interface
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	// ~End UCharacterMovementComponent Interface

	// ~Begin UActorComponent Interface
	virtual void InitializeComponent() override;
	// ~End UActorComponent Interface

private:
	/** 开始滑行 */
	void EnterSlide();
	/** 结束滑行 */
	void ExitSlide();
	/** 滑行物理计算 */
	void PhysSlide(float DeltaTime, int32 Iterations);
	/** 获取滑行面碰撞信息 */
	bool GetSlideSurface(FHitResult& OutHitResult) const;

public:
	/** 冲刺状态 */
	bool Safe_bWantsToSprint;

	/**  */
	bool Safe_bPrevWantsToCrouch;

protected:
	UPROPERTY(Transient, DuplicateTransient)
	TWeakObjectPtr<ATD_ZippyCharacter> ZippyCharacterOwner;

private:
	/** 冲刺时最大速度 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Sprint", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float Sprint_MaxWalkSpeed;

	/** 行走最大速度 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Sprint", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float Walk_MaxWalkSpeed;


	
	/** 滑行最小速度 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Slide", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float Slide_MinSpeed = 350;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Slide")
	float Slide_EnterImpulse = 550;

	/** 滑行重力 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Slide")
	float Slide_GravityForce = 5000;

	/** 滑行摩檫力 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Slide")
	float Slide_Friction = 1.3;
};
