// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "TD_PlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class TD_ZIPPY_API ATD_PlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float CrouchBlendDuration=.2f;
	float CrouchBlendTime;

public:
	ATD_PlayerCameraManager();

	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
};
