// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TD_CharacterMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDashStartDelegate);

UENUM(BlueprintType)
enum ETD_CustomMovementMode : int
{
	CMOVE_None			UMETA(Hidden),
	CMOVE_Slide			UMETA(DisplayName = "Slide"),
	CMOVE_Prone			UMETA(DisplayName = "Prone"),
	CMOVE_MAX			UMETA(Hidden),
};

class ATD_ZippyCharacter;
class UAnimMontage;

/**
 * 
 */
UCLASS()
class TD_ZIPPY_API UTD_CharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	/**
	 * 角色移动数据缓存
	 */
	class FTD_SavedMove_Character : public FSavedMove_Character
	{
		typedef FSavedMove_Character Super;
		
	public:
		enum CompressedFlags
		{
			FLAG_Sprint			= 0x10,
			FLAG_Dash			= 0x20,
			FLAG_Custom_2		= 0x40,
			FLAG_Custom_3		= 0x80,
		};

		
		uint8 Saved_bWantsToSprint : 1;
		
		uint8 Saved_bPrevWantsToCrouch : 1;

		uint8 Saved_bWantsToProne : 1;

		uint8 Saved_bWantsToDash : 1;

	public:

		FTD_SavedMove_Character();

		/** 如果此移动可以与 NewMove 结合使用以进行复制而不更改任何行为，则返回 true */
		virtual bool CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const override;

		/** 清除已保存的移动属性，以便可以重新使用。 */
		virtual void Clear() override;
		
		/** 返回一个包含编码的特殊移动信息（跳跃、蹲伏等）的字节 */
		virtual uint8 GetCompressedFlags() const override;

		/** 调用以设置此保存的移动（最初创建时）以进行预测性更正。 */
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

		/** 在 ClientUpdatePosition 使用此 SavedMove 进行预测性更正之前调用 */
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FTD_NetworkPredictionData_Client_Character : public FNetworkPredictionData_Client_Character
	{
		typedef FNetworkPredictionData_Client_Character Super;
		
	public:
		FTD_NetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement);

		/** 分配新的已保存移动。如果子类想要使用自定义移动类，则应覆盖此项。 */
		virtual FSavedMovePtr AllocateNewMove() override;
	};

	class FTD_NetworkPredictionData_Server_Character : public FNetworkPredictionData_Server_Character
	{
		typedef FNetworkPredictionData_Server_Character Super;

	public:
		FTD_NetworkPredictionData_Server_Character(const UCharacterMovementComponent& ServerMovement);
	};


	/**
	 * Begin
	 * 以下为安全得移动属性变量，可以放心在移动函数中使用
	 * 移动函数中不可使用非安全移动变量
	 */
	/** 是否想要冲刺 */
	bool Safe_bWantsToSprint = false;

	/**  */
	bool Safe_bPrevWantsToCrouch = false;

	/** 是否想要爬行 */
	bool Safe_bWantsToProne;

	/**  */
	bool Safe_bWantsToDash;
	/////////////////////////////// End ///////////////////////////////
	

public:
	UTD_CharacterMovementComponent();

	/**
	 * Begin
	 * 以下为非安全移动函数：
	 * 1、以下函数仅能在客户端调用
	 * 2、以下函数未经过 RPC 同步，所以仅能更改安全得移动属性（安全移动属性会主动同步）
	 */
	/** 触发冲刺 */
	UFUNCTION(BlueprintCallable)
	void SprintPressed();
	/** 结束冲刺 */
	UFUNCTION(BlueprintCallable)
	void SprintReleased();

	/** 触发蹲伏
	 * 1.双击进入滑行（速度需要达到一定的阈值）
	 * 2.长按进入爬行状态
	 */
	UFUNCTION(BlueprintCallable)
	void CrouchPressed();
	UFUNCTION(BlueprintCallable)
	void CrouchReleased();

	/**
	 * 
	 */
	UFUNCTION(BlueprintCallable)
	void DashPressed();
	UFUNCTION(BlueprintCallable)
	void DashReleased();
	/////////////////////////////// End ///////////////////////////////

	
	/** 是否为自定义模式移动 */
	UFUNCTION(BlueprintCallable)
	bool IsCustomMovementMode(ETD_CustomMovementMode InMovementMode) const;

	UFUNCTION(BlueprintPure)
	bool IsMovementMode(EMovementMode InMovementMode) const { return InMovementMode == MovementMode; }

	
	// ~Begin UCharacterMovementComponent Interface
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;
	// ~End UCharacterMovementComponent Interface

protected:
	// ~Begin UCharacterMovementComponent Interface
	/** 重写预测 */
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual FNetworkPredictionData_Server* GetPredictionData_Server() const override;

	/** 从已保存的 move 中解压缩 flag 并相应地设置 state。请参阅 FSavedMove_Character。 */
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	/**
	 * 在移动更新结束时触发的事件。如果启用了有范围的移动更新（bEnableScopedMovementUpdates），则这是在这样的范围内。
	 * 如果不需要，请改为绑定到 CharacterOwner 的 OnMovementUpdated 事件，因为该事件是在限定范围的移动更新后触发的。
	 */
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	/** 在执行实际位置更改之前更新 PerformMovement 中的角色状态 */
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	
	/** 移动更新函数只能通过 StartNewPhysics（） 调用 */
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	/** 在 MovementMode 更改后调用。Base 实现对启动某些模式进行特殊处理，然后通知 CharacterOwner。 */
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	// ~End UCharacterMovementComponent Interface

	
	// ~Begin UActorComponent Interface
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// ~End UActorComponent Interface

	// ~Begin UObject Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// ~End UObject Interface
	
private:
	///////////////////// ~Begin Slide /////////////////////
	/** 开始滑行 */
	void EnterSlide(EMovementMode PrevMode, ETD_CustomMovementMode PrevCustomMode);
	/** 结束滑行 */
	void ExitSlide();
	/** 滑行物理计算 */
	void PhysSlide(float DeltaTime, int32 Iterations);
	/** 是否可以进入滑行状态 */
	bool CanSlide() const;
	///////////////////// ~End Slide /////////////////////

	
	///////////////////// ~Begin Prone /////////////////////
	void TryEnterProne() { Safe_bWantsToProne = true; }
	UFUNCTION(Server, Reliable)
	void Server_EnterProne();
	
	void EnterProne(EMovementMode PrevMode, ETD_CustomMovementMode PrevCustomMode);
	void ExitProne();
	bool CanProne() const;
	void PhysProne(float deltaTime, int32 Iterations);
	///////////////////// ~End Prone /////////////////////


	///////////////////// ~Begin Dash /////////////////////
	/** 冲刺冷却结束（冲刺将延迟执行，等待冷却完毕/上一个冲刺状态结束后再次执行） */
	void OnDashCooldownFinished();

	/** 是否可以冲刺 */
	bool CanDash() const;

	/** 执行冲刺事件 */
	void PerformDash();

	/**  */
	UFUNCTION()
	void OnRep_DashStart();
	///////////////////// ~End Dash /////////////////////

public:
	/** 开始冲刺动作（通知客户端事件：例如播放蒙太奇） */
	UPROPERTY(BlueprintAssignable, Category="Character Movement: Dash")
	FDashStartDelegate DashStartDelegate;
	
protected:
	///////////////////// ~Begin Sprint /////////////////////
	/** 冲刺时最大速度 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Sprint", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxSprintSpeed = 1000.f;
	///////////////////// ~End Sprint /////////////////////

	
	///////////////////// ~Begin Slide /////////////////////
	/** 滑行最小速度 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Slide", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MinSlideSpeed = 400;

	/** 滑行最小速度 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Slide", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxSlideSpeed = 400;

	/** 首次进入滑行状态得一个冲力 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Slide")
	float SlideEnterImpulse = 550;

	/** 滑行重力应用值 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Slide")
	float SlideGravityForce = 5000;

	/** 滑行摩檫力 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Slide")
	float SlideFrictionFactor = 1.3;

	/** 滑行时的减速制动 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Slide")
	float BrakingDecelerationSliding = 1000.f;
	///////////////////// ~End Slide /////////////////////

	
	///////////////////// ~Begin Prone /////////////////////
	/** 进入爬行状态得持续时间 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Prone")
	float ProneEnterHoldDuration = .2f;

	/** 进入爬行得脉冲力 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Prone")
	float ProneSlideEnterImpulse = 300.f;

	/** 爬行最大速度 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Prone", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxProneSpeed = 300.f;

	/** 爬行时的减速制动 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Prone")
	float BrakingDecelerationProning = 2500.f;

	FTimerHandle TimerHandle_EnterProne;
	///////////////////// ~End Prone /////////////////////


	///////////////////// ~Begin Dash /////////////////////
	/** 向前冲刺的冲力（更改为根运动） */
	// UPROPERTY(EditDefaultsOnly, Category="Character Movement: Dash")
	// float DashImpulse = 1000.f;

	/** 冲刺蒙太奇动画 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Dash")
	TObjectPtr<UAnimMontage> DashMontage;

	/** 冲刺冷却时间 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Dash")
	float DashCooldownDuration = 1.f;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Dash")
	float AuthDashCooldownDuration = .9f;

	/** 开始冲刺（同步多端知晓） */
	UPROPERTY(ReplicatedUsing = OnRep_DashStart)
	bool Proxy_bDashStart;
	
	float DashStartTime = 0.f;
	FTimerHandle TimerHandle_DashCooldown;
	///////////////////// ~End Dash /////////////////////
	
	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<ATD_ZippyCharacter> ZippyCharacterOwner;
};