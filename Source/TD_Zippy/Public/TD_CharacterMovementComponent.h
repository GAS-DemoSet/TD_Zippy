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
};