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

	/** 滑行功能 */
	CMOVE_Slide			UMETA(DisplayName = "Slide"),

	/** 爬行 */
	CMOVE_Prone			UMETA(DisplayName = "Prone"),

	/** 墙跑 */
	CMOVE_WallRun		UMETA(DisplayName = "Wall Run"),

	/** 挂墙 */
	CMOVE_Hang			UMETA(DisplayName = "Hang"),

	/** 爬墙 */
	CMOVE_Climb			UMETA(DisplayName = "Climb"),

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

		// flags
		uint8 Saved_bWantsToSprint : 1;
		uint8 Saved_bWantsToDash : 1;
		uint8 Saved_bPressedZippyJump:1;

		// Other Variables
		uint8 Saved_bPrevWantsToCrouch : 1;
		uint8 Saved_bWantsToProne : 1;
		uint8 Saved_bHadAnimRootMotion : 1;
		uint8 Saved_bTransitionFinished : 1;
		uint8 Saved_bWallRunIsRight : 1;

	public:

		FTD_SavedMove_Character();

		/** 如果此移动可以与 NewMove 结合使用以进行复制而不更改任何行为，则返回 true */
		virtual bool CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const override;

		/** 清除已保存的移动属性，以便可以重新使用。 */
		virtual void Clear() override;
		
		/**
		 * 返回一个包含编码的特殊移动信息（跳跃、蹲伏等）的字节。
		 * 压缩需要发送服务器的状态值。
		 */
		virtual uint8 GetCompressedFlags() const override;

		/**
		 * 调用以设置此保存的移动（最初创建时）以进行预测性更正。
		 * 在最开始本地预测时保存状态，一帧只调用一次。
		 */
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

		/**
		 * 仅在客户端运行，影响客户端的 CMC，并且只有客户端需要回滚重演之前保存下来的 SaveMode 时才会调用。
		 * 只有在收到服务器校正、需要回滚预测时，才会用它来逐帧还原状态并重演。
		 * 当客户端“预测回滚”时，使用之前保存的 SavedMove，把角色状态还原成当时的状态，以重新模拟那一帧。
		 * 在 ClientUpdatePosition 使用此 SavedMove 进行预测性更正之前调用。
		 */
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
	// Flags
	/** 是否想要冲刺 */
	bool Safe_bWantsToSprint = false;
	/** 是否想要爬行 */
	bool Safe_bWantsToProne;
	/** 是否想要冲刺 */
	bool Safe_bWantsToDash;

	/** 角色上一帧的状态（上一帧是否为蹲伏状态） */
	bool Safe_bPrevWantsToCrouch = false;

	bool Safe_bHadAnimRootMotion;
	bool Safe_bTransitionFinished;

	/** 当前时左墙跑还是右墙跑 */
	bool Safe_bWallRunIsRight;
	/////////////////////////////// End ///////////////////////////////

	TSharedPtr<FRootMotionSource_MoveToForce> TransitionRMS;
	FString TransitionName;
	UPROPERTY(Transient)
	UAnimMontage* TransitionQueuedMontage;
	float TransitionQueuedMontageSpeed;
	uint16 TransitionRMS_ID;

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

	UFUNCTION(BlueprintCallable)
	void ClimbPressed();
	UFUNCTION(BlueprintCallable)
	void ClimbReleased();
	/////////////////////////////// End ///////////////////////////////

	
	/** 是否为自定义模式移动 */
	UFUNCTION(BlueprintCallable)
	bool IsCustomMovementMode(ETD_CustomMovementMode InMovementMode) const;

	UFUNCTION(BlueprintPure)
	bool IsMovementMode(EMovementMode InMovementMode) const { return InMovementMode == MovementMode; }

	UFUNCTION(BlueprintPure)
	bool IsWallRunning() const { return IsCustomMovementMode(CMOVE_WallRun); }
	UFUNCTION(BlueprintPure)
	bool WallRunningIsRight() const { return Safe_bWallRunIsRight; }
	UFUNCTION(BlueprintPure)
	bool IsHanging() const { return IsCustomMovementMode(CMOVE_Hang); }
	UFUNCTION(BlueprintPure)
	bool IsClimbing() const { return IsCustomMovementMode(CMOVE_Climb); }
	
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

	/**
	 * 收到客户端移动 RPC 数据（SaveMove）后调用
	 * 从已保存的 move 中解压缩 flag 并相应地设置 state。请参阅 FSavedMove_Character。
	 * 影响服务器的 CMC
	 */
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	/**
	 * 在移动更新结束时触发的事件。如果启用了有范围的移动更新（bEnableScopedMovementUpdates），则这是在这样的范围内。
	 * 如果不需要，请改为绑定到 CharacterOwner 的 OnMovementUpdated 事件，因为该事件是在限定范围的移动更新后触发的。
	 * 
	 * 这个函数在每一帧角色移动完成后被调用，用于对移动之后的角色状态做进一步的处理。比如你可以在这里添加特效、动画更新、同步网络状态等。
	 */
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	/**
	 * 在执行实际位置更改之前更新 PerformMovement 中的角色状态。
	 *
	 * 这个函数在每一帧角色移动之前被调用，用于更新角色当前的状态，比如是否应该开始下落、游泳，或者切换移动模式等。
	 */
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	/**
	 * 位置更改后，在 PerformMovement 中更新角色状态。在此之后会进行一些轮换更新。
	 * 这个函数在每一帧角色移动后被调用
	 */
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;
	/**
	 * 移动更新函数只能通过 StartNewPhysics（） 调用。
	 */
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	/**
	 * 在 MovementMode 更改后调用。Base 实现对启动某些模式进行特殊处理，然后通知 CharacterOwner。
	 *
	 * 在移动模式发生变化时调用，比如从行走变成游泳、飞行、下落等。可以在这里执行状态切换时的逻辑，比如播放转换动画、初始化某些参数。
	* 当 SetMovementMode 被调用并且移动模式发生了真正的改变时，比如：从 MOVE_Walking → MOVE_Falling。
	 */
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	/** 如果当前移动状态允许尝试跳跃，则返回 true。由 Character：：CanJump（） 使用。 */
	virtual bool CanAttemptJump() const override;

	/** 执行 jump 。当检测到跳跃时由角色调用，因为 Character->bPressedJump 为 true。检查 Character->CanJump（）。请注意，您通常应该通过 Character：：Jump（） 触发跳转。 */
	virtual bool DoJump(bool bReplayingMoves, float DeltaTime) override;
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

	///////////////////// ~Begin Mantle /////////////////////

	/**  */
	bool TryMantle();
	
	/**  */
	FVector GetMantleStartLocation(FHitResult FrontHit, FHitResult SurfaceHit, bool bTallMantle) const;
	
	UFUNCTION()
	void OnRep_ShortMantle();
	UFUNCTION()
	void OnRep_TallMantle();
	///////////////////// ~End Mantle /////////////////////

	///////////////////// ~Begin Wall Run /////////////////////
	bool TryWallRun();
	void PhysWallRun(float deltaTime, int32 Iterations);
	///////////////////// ~End Wall Run /////////////////////

	///////////////////// ~Begin Climb /////////////////////
	bool TryHang();

	bool TryClimb();
	void PhysClimb(float deltaTime, int32 Iterations);
	///////////////////// ~End Climb /////////////////////
	
private:
	bool IsServer() const;
	/** 返回按组件缩放缩放的胶囊体半径。 */
	float CapR() const;
	/** 返回按组件缩放缩放的胶囊体半高。这包括圆柱体和半球帽。 */
	float CapHH() const;
	
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

	/** 冲刺冷却时间，并非可以连续触发冲刺。 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Dash")
	float AuthDashCooldownDuration = .9f;

	/** 开始冲刺（同步多端知晓） */
	UPROPERTY(ReplicatedUsing = OnRep_DashStart)
	bool Proxy_bDashStart;
	
	float DashStartTime = 0.f;
	FTimerHandle TimerHandle_DashCooldown;
	///////////////////// ~End Dash /////////////////////

		
	///////////////////// ~Begin Mantle /////////////////////
	/** 翻墙时允许距离墙的最大距离，超过这个距离不会进行翻墙 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Mantle")
	float MantleMaxDistance = 200;

	/** 翻墙时最高可以攀上的高度，超高这个高度禁止翻墙 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Mantle")
	float MantleReachHeight = 50;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Mantle")
	float MinMantleDepth = 30;

	/** 斜坡与地面所形成的夹角（允许角色翻越墙体的最小夹角度） */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Mantle")
	float MantleMinWallSteepnessAngle = 75;

	/** 即将爬上的墙面与地面的角度 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Mantle")
	float MantleMaxSurfaceAngle = 40;

	/** 角色正前方与斜坡所形成的夹角 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Mantle")
	float MantleMaxAlignmentAngle = 45;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Mantle")
	UAnimMontage* TallMantleMontage;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Mantle")
	UAnimMontage* TransitionTallMantleMontage;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Mantle")
	UAnimMontage* ProxyTallMantleMontage;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Mantle")
	UAnimMontage* ShortMantleMontage;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Mantle")
	UAnimMontage* TransitionShortMantleMontage;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Mantle")
	UAnimMontage* ProxyShortMantleMontage;

	UPROPERTY(ReplicatedUsing=OnRep_ShortMantle)
	bool Proxy_bShortMantle;
	UPROPERTY(ReplicatedUsing=OnRep_TallMantle)
	bool Proxy_bTallMantle;
	///////////////////// ~End Mantle /////////////////////

	
	///////////////////// ~Begin Wall Run /////////////////////
	/** 最小墙跑速度 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Wall Run")
	float MinWallRunSpeed = 200.f;

	/** 最大墙跑速度 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Wall Run")
	float MaxWallRunSpeed = 800.f;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Wall Run")
	float MaxVerticalWallRunSpeed = 200.f;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Wall Run")
	float WallRunPullAwayAngle = 75.f;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Wall Run")
	float WallAttractionForce = 200.f;

	/** 最小的墙跑高度，低于这个高度时，将结束墙跑 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Wall Run")
	float MinWallRunHeight = 50.f;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Wall Run")
	TObjectPtr<UCurveFloat> WallRunGravityScaleCurve;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Wall Run")
	float WallJumpOffForce = 300.f;
	///////////////////// ~End Wall Run /////////////////////

	
	///////////////////// ~Begin Hang /////////////////////
	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Hang")
	UAnimMontage* TransitionHangMontage;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Hang")
	UAnimMontage* WallJumpMontage;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Hang")
	float WallJumpForce = 400.f;
	///////////////////// ~End Hang /////////////////////

	
	///////////////////// ~Begin Climb /////////////////////
	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Climb")
	float MaxClimbSpeed = 300.f;
	
	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Climb")
	float BrakingDecelerationClimbing = 1000.f;

	/**  */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Climb")
	float ClimbReachDistance = 200.f;
	///////////////////// ~End Climb /////////////////////
	
	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<ATD_ZippyCharacter> ZippyCharacterOwner;
};