// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "TD_PlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class TD_ZIPPY_API ATD_PlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ATD_PlayerCameraManager(const FObjectInitializer& ObjectInitializer);
	/** 计算给定视图目标的更新 POV */
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

private:
	/** 蹲伏过程中相机混合总持续时间 */
	UPROPERTY(EditDefaultsOnly)
	float CrouchBlendDuration = .2f;

	/** 当前蹲伏相机已持续时间 */
	float CrouchBlendTime = 0.f;
};
