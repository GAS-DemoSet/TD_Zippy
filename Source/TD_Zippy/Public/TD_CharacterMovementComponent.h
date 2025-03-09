// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TD_CharacterMovementComponent.generated.h"

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
		uint8 Saved_bWantsToSprint : 1;

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
	bool Safe_bWantsToSprint;
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
	/////////////////////////////// End ///////////////////////////////

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
	// ~End UCharacterMovementComponent Interface

protected:
	/** 冲刺时最大速度 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Sprint", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float Sprint_MaxWalkSpeed;

	/** 行走最大速度 */
	UPROPERTY(EditDefaultsOnly, Category="Character Movement: Sprint", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float Walk_MaxWalkSpeed;
};