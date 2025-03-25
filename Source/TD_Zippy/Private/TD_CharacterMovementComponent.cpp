// Fill out your copyright notice in the Description page of Project Settings.


#include "TD_CharacterMovementComponent.h"

#include <complex.h>

#include "TD_LogDefine.h"
#include "TD_ZippyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

UTD_CharacterMovementComponent::FTD_SavedMove_Character::FTD_SavedMove_Character()
	: Super()
	, Saved_bWantsToSprint(0)
	, Saved_bPrevWantsToCrouch(0)
	, Saved_bWantsToProne(0)
	, Saved_bWantsToDash(0)
{
}

bool UTD_CharacterMovementComponent::FTD_SavedMove_Character::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const
{
	const FTD_SavedMove_Character* NewSaveMove = static_cast<FTD_SavedMove_Character*>(NewMovePtr.Get());

	if (Saved_bWantsToSprint != NewSaveMove->Saved_bWantsToSprint)
	{
		return false;
	}

	if (Saved_bWantsToDash != NewSaveMove->Saved_bWantsToDash)
	{
		return false;
	}
	
	return Super::CanCombineWith(NewMovePtr, InCharacter, MaxDelta);
}

void UTD_CharacterMovementComponent::FTD_SavedMove_Character::Clear()
{
	Super::Clear();

	Saved_bWantsToSprint = 0;
	Saved_bWantsToDash = 0;

	Saved_bWantsToProne = 0;
	Saved_bPrevWantsToCrouch = 0;
}

uint8 UTD_CharacterMovementComponent::FTD_SavedMove_Character::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	
	if (Saved_bWantsToSprint)
	{
		Result |= FLAG_Sprint;
	}

	if (Saved_bWantsToDash)
	{
		Result |= FLAG_Dash;
	}
	
	return Result;
}

void UTD_CharacterMovementComponent::FTD_SavedMove_Character::SetMoveFor(ACharacter* C, float InDeltaTime,
	FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	if (const UTD_CharacterMovementComponent* TempCMC = Cast<UTD_CharacterMovementComponent>(C->GetCharacterMovement()))
	{
		// 此处为本地权威代理操作，随后数据将发送服务器进行同步。
		// Print_Log_NetRole(TempCMC->GetOwner(), TD_Log_CMC_Debug, Warning, TEXT("SetMoveFor"));
		Saved_bWantsToSprint = TempCMC->Safe_bWantsToSprint;
		Saved_bPrevWantsToCrouch = TempCMC->Safe_bPrevWantsToCrouch;
		Saved_bWantsToProne = TempCMC->Safe_bWantsToProne;
		Saved_bWantsToDash = TempCMC->Safe_bWantsToDash;
	}
}

void UTD_CharacterMovementComponent::FTD_SavedMove_Character::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	if (UTD_CharacterMovementComponent* TempCMC = Cast<UTD_CharacterMovementComponent>(C->GetCharacterMovement()))
	{
		Print_Log_NetRole(TempCMC->GetOwner(), TD_Log_CMC_Debug, Warning, TEXT("PrepMoveFor"));
		TempCMC->Safe_bWantsToSprint = Saved_bWantsToSprint;
		TempCMC->Safe_bPrevWantsToCrouch = Saved_bPrevWantsToCrouch;
		TempCMC->Safe_bWantsToProne = Saved_bWantsToProne;
		TempCMC->Safe_bWantsToDash = Saved_bWantsToDash;
	}
}

UTD_CharacterMovementComponent::FTD_NetworkPredictionData_Client_Character::FTD_NetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UTD_CharacterMovementComponent::FTD_NetworkPredictionData_Client_Character::AllocateNewMove()
{
	return MakeShared<FTD_SavedMove_Character>();
	// return FSavedMovePtr(new FTD_SavedMove_Character());
}

UTD_CharacterMovementComponent::FTD_NetworkPredictionData_Server_Character::FTD_NetworkPredictionData_Server_Character(const UCharacterMovementComponent& ServerMovement)
	: Super(ServerMovement)
{
}

UTD_CharacterMovementComponent::UTD_CharacterMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 允许角色蹲伏状态
	NavAgentProps.bCanCrouch = true;
}

#pragma region CMC
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

	// 长按 ProneEnterHoldDuration 时间后进入爬行状态
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_EnterProne, this, &UTD_CharacterMovementComponent::TryEnterProne, ProneEnterHoldDuration);
}

void UTD_CharacterMovementComponent::CrouchReleased()
{
	// 取消进入爬行状态句柄
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_EnterProne);
}

void UTD_CharacterMovementComponent::DashPressed()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - DashStartTime >= DashCooldownDuration)
	{
		Safe_bWantsToDash = true;
	}
	else
	{
		// 每次按下都会执行冲刺，但是如果在冷却期内的话，执行将被延迟到下一个执行阶段
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_DashCooldown, this, &UTD_CharacterMovementComponent::OnDashCooldownFinished, DashCooldownDuration - (CurrentTime - DashStartTime));
	}
}

void UTD_CharacterMovementComponent::DashReleased()
{
	// 松开按键后将取消之前的延迟执行
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_DashCooldown);
	Safe_bWantsToDash = false;
}

bool UTD_CharacterMovementComponent::IsCustomMovementMode(ETD_CustomMovementMode InMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InMovementMode; 
}

bool UTD_CharacterMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide) || IsCustomMovementMode(CMOVE_Prone);
}

bool UTD_CharacterMovementComponent::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}

float UTD_CharacterMovementComponent::GetMaxSpeed() const
{
	if (IsMovementMode(MOVE_Walking) && Safe_bWantsToSprint && !IsCrouching()) return MaxSprintSpeed;
	
	if (MovementMode != MOVE_Custom) return Super::GetMaxSpeed();

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return MaxSlideSpeed;
	case CMOVE_Prone:
		return MaxProneSpeed;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
		return -1.f;
	}
}

float UTD_CharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if (MovementMode != MOVE_Custom) return Super::GetMaxBrakingDeceleration();

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return BrakingDecelerationSliding;
	case CMOVE_Prone:
		return BrakingDecelerationProning;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
		return -1.f;
	}
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
	
	// return Super::GetPredictionData_Client();
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
	// return Super::GetPredictionData_Server();
}

void UTD_CharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	// 此处仅同步到了服务端
	Safe_bWantsToSprint = (Flags & FTD_SavedMove_Character::FLAG_Sprint) != 0;
	// Print_Log_NetRole(this->GetOwner(), TD_Log_CMC_Debug, Warning, *FString::Printf(TEXT("UpdateFromCompressedFlags: Safe_bWantsToSprint=%s"), Safe_bWantsToSprint ? TEXT("true") : TEXT("false")));

	Safe_bWantsToDash = (Flags & FTD_SavedMove_Character::FLAG_Dash) != 0;
}

void UTD_CharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	// 如果在飞行状态并且没有根运动的状态下即切换为行走，最初是为了在根运动冲结束后将状态更正。
	if (IsMovementMode(MOVE_Flying) && !HasRootMotionSources())
	{
		SetMovementMode(MOVE_Walking);
	}
	
	Safe_bPrevWantsToCrouch = bWantsToCrouch;
}

void UTD_CharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	// Slide
	if (MovementMode == MOVE_Walking && !bWantsToCrouch && Safe_bPrevWantsToCrouch)
	{
		if (CanSlide())
		{
			SetMovementMode(MOVE_Custom, CMOVE_Slide);
		}
	}
	if (IsCustomMovementMode(CMOVE_Slide) && !bWantsToCrouch)
	{
		SetMovementMode(MOVE_Walking);
	}

	// Prone
	if (Safe_bWantsToProne) 
	{
		if (CanProne())
		{
			SetMovementMode(MOVE_Custom, CMOVE_Prone);
			if (!CharacterOwner->HasAuthority()) Server_EnterProne();
		}
		Safe_bWantsToProne = false;
	}
	if (IsCustomMovementMode(CMOVE_Prone) && !bWantsToCrouch)
	{
		SetMovementMode(MOVE_Walking);
	}

	// Dash
	bool bAuthProxy = CharacterOwner->HasAuthority() && !CharacterOwner->IsLocallyControlled(); // 如果是模拟端这个值将为 true
	if (Safe_bWantsToDash && CanDash())
	{
		if (!bAuthProxy || GetWorld()->GetTimeSeconds() - DashStartTime > AuthDashCooldownDuration)
		{
			// 执行冲刺行为（权威执行）
			PerformDash();
			Safe_bWantsToDash = false;
			Proxy_bDashStart = !Proxy_bDashStart;
		}
		else
		{
			// 模拟端行为（作弊）
			UE_LOG(LogTemp, Warning, TEXT("Client tried to cheat"))
		}
	}

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UTD_CharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	// UE_LOG(TD_Log_CMC_Debug, Warning, TEXT("PhysCustom"));

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		PhysSlide(deltaTime, Iterations);
		break;
	case CMOVE_Prone:
		PhysProne(deltaTime, Iterations);
		break;
	default:
		UE_LOG(TD_Log_CMC_Debug, Fatal, TEXT("Invalid Movement Mode"));
		break;
	}
}

void UTD_CharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Slide) ExitSlide();
	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Prone) ExitProne();
	
	if (IsCustomMovementMode(CMOVE_Slide)) EnterSlide(PreviousMovementMode, (ETD_CustomMovementMode)PreviousCustomMode);
	if (IsCustomMovementMode(CMOVE_Prone)) EnterProne(PreviousMovementMode, (ETD_CustomMovementMode)PreviousCustomMode);
}

void UTD_CharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	ZippyCharacterOwner = Cast<ATD_ZippyCharacter>(GetOuter());
}

void UTD_CharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
#pragma endregion

#pragma region Slide
void UTD_CharacterMovementComponent::EnterSlide(EMovementMode PrevMode, ETD_CustomMovementMode PrevCustomMode)
{
	bWantsToCrouch = true;
	bOrientRotationToMovement = false;
	Velocity += Velocity.GetSafeNormal2D() * SlideEnterImpulse;

	FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, true, nullptr);
}

void UTD_CharacterMovementComponent::ExitSlide()
{
	bWantsToCrouch = false;
	bOrientRotationToMovement = true;
}

void UTD_CharacterMovementComponent::PhysSlide(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}
	
	if (!CanSlide())
	{
		SetMovementMode(MOVE_Walking);
		StartNewPhysics(DeltaTime, Iterations);
		return;
	}

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = DeltaTime;

	// 执行移动
	while ( (remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)) )
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// 保存当前值
		UPrimitiveComponent * const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != nullptr) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		// 确保速度是水平的。
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;

		FVector SlopeForce = CurrentFloor.HitResult.Normal;
		SlopeForce.Z = 0.f;
		Velocity += SlopeForce * SlideGravityForce * DeltaTime;
		
		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector().GetSafeNormal2D());

		// 应用加速
		CalcVelocity(timeTick, GroundFriction * SlideFrictionFactor, false, GetMaxBrakingDeceleration());
		
		// 计算移动参数
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;
		bool bFloorWalkable = CurrentFloor.IsWalkableFloor();

		if ( bZeroDelta )
		{
			remainingTime = 0.f;
		}
		else
		{
			// 尝试向前迈进
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if ( IsFalling() )
			{
				// Pawn 决定跳起来
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}
				StartNewPhysics(remainingTime, Iterations);
				return;
			}
			else if ( IsSwimming() ) // 刚入水
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
		}

		// 更新地板。
		// StepUp 可能已经为我们完成了。
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, nullptr);
		}


		// 在此处检查壁架
		const bool bCheckLedges = !CanWalkOffLedges();
		if ( bCheckLedges && !CurrentFloor.IsWalkableFloor() )
		{
			// 计算可能的交替移动
			const FVector GravDir = FVector(0.f,0.f,-1.f);
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, OldFloor);
			if ( !NewDelta.IsZero() )
			{
				// 首先撤销此移动
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// 如果第一次失败，请避免重复移动壁架
				bTriedLedgeMove = true;

				// 尝试新的运动方向
				Velocity = NewDelta / timeTick;
				remainingTime += timeTick;
				continue;
			}
			else
			{
				// 看看跳不跳可以
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == nullptr || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ( (bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;

				// 撤销此移动
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// 验证楼层检查
			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{
						// 如果还在走，那就跌倒。如果没有，则假设用户设置了他们想要保留的不同模式。
						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// 地板检查失败，因为它开始渗透
				// 我们不想因为向下扫描失败而尝试向下移动，而是想尝试从地板中弹出。
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// 检查是否刚刚进入水中
			if ( IsSwimming() )
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// 看看我们是否需要开始下降。
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == nullptr || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;
			}
		}
		
		// 允许重叠事件等更改物理状态和速度
		if (IsMovingOnGround() && bFloorWalkable)
		{
			// 使速度反映实际移动
			if( !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		// 如果我们在这个迭代中根本没有移动，那么 abort （因为未来的迭代也会卡住）。
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}

	FHitResult Hit;
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(Velocity.GetSafeNormal2D(), FVector::UpVector).ToQuat();
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, false, Hit);
}

bool UTD_CharacterMovementComponent::CanSlide() const
{
	if (ensure(GetWorld()))
	{
		FVector Start = UpdatedComponent->GetComponentLocation();
		FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.5f * FVector::DownVector;
		FName ProfileName = TEXT("BlockAll");
		bool bValidSurface = GetWorld()->LineTraceTestByProfile(Start, End, ProfileName, ZippyCharacterOwner->GetIgnoreCharacterParams());
		bool bEnoughSpeed = Velocity.SizeSquared() > pow(MinSlideSpeed, 2);
	
		return bValidSurface && bEnoughSpeed;
	}
	return false;
}
#pragma endregion

#pragma region Prone
void UTD_CharacterMovementComponent::Server_EnterProne_Implementation()
{
	Safe_bWantsToProne = true;
}

void UTD_CharacterMovementComponent::EnterProne(EMovementMode PrevMode, ETD_CustomMovementMode PrevCustomMode)
{
	bWantsToCrouch = true;

	if (PrevMode == MOVE_Custom && PrevCustomMode == CMOVE_Slide)
	{
		Velocity += Velocity.GetSafeNormal2D() * ProneSlideEnterImpulse;
	}

	FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, true, nullptr);
}

void UTD_CharacterMovementComponent::ExitProne()
{
}

bool UTD_CharacterMovementComponent::CanProne() const
{
	return IsCustomMovementMode(CMOVE_Slide) || IsMovementMode(MOVE_Walking) && IsCrouching();
}

void UTD_CharacterMovementComponent::PhysProne(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CharacterOwner || (!CharacterOwner->Controller && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = deltaTime;

	// 执行移动
	while ( (remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)) )
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// 保存当前值
		UPrimitiveComponent * const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != nullptr) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		// 确保速度是水平的。
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;
		Acceleration.Z = 0.f;

		// 应用加速
		CalcVelocity(timeTick, GroundFriction, false, GetMaxBrakingDeceleration());
		
		// 计算移动参数
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity; // dx = v * dt
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;

		if ( bZeroDelta )
		{
			remainingTime = 0.f;
		}
		else
		{
			// 实际开始移动
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			// 下坠状态
			if ( IsFalling() )
			{
				// Pawn 决定跳起来
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}
				StartNewPhysics(remainingTime, Iterations);
				return;
			}
			// 入水状态
			else if ( IsSwimming() ) 
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
		}

		// 更新地板。
		// StepUp 可能已经为我们完成了。
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, nullptr);
		}


		// 在此处检查壁架
		const bool bCheckLedges = !CanWalkOffLedges();
		if (bCheckLedges && !CurrentFloor.IsWalkableFloor())
		{
			// 计算可能的交替移动
			const FVector GravDir = FVector(0.f,0.f,-1.f);
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, OldFloor);
			if ( !NewDelta.IsZero() )
			{
				// 首先撤销此移动
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// 如果第一次失败，请避免重复移动壁架
				bTriedLedgeMove = true;

				// 尝试新的运动方向
				Velocity = NewDelta / timeTick; // v = dx/dt
				remainingTime += timeTick;
				continue;
			}
			else
			{
				// 看看跳不跳可以
				// @todo collision : 唯一可能的问题是 OldBase 开启了世界碰撞
				bool bMustJump = bZeroDelta || (OldBase == nullptr || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ( (bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;

				// 撤销此移动
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// 验证楼层检查
			if (CurrentFloor.IsWalkableFloor())
			{
				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// 地板检查失败，因为它开始渗透
				// 我们不想因为向下扫描失败而尝试向下移动，而是想尝试从地板中弹出。
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// 检查是否刚刚进入水中
			if ( IsSwimming() )
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// 看看我们是否需要开始下降。
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;
			}
		}
		
		// 允许重叠事件等更改物理状态和速度
		if (IsMovingOnGround())
		{
			// 使速度反映实际移动
			if( !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick; // v = dx / dt
				MaintainHorizontalGroundVelocity();
			}
		}

		// 如果我们在这个迭代中根本没有移动，那么 abort （因为未来的迭代也会卡住）。
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}

	if (IsMovingOnGround())
	{
		MaintainHorizontalGroundVelocity();
	}
}
#pragma endregion


#pragma region Dash
void UTD_CharacterMovementComponent::OnDashCooldownFinished()
{
	Safe_bWantsToDash = true;
}

bool UTD_CharacterMovementComponent::CanDash() const
{
	return IsWalking() && !IsCrouching() || IsFalling();
}

void UTD_CharacterMovementComponent::PerformDash()
{
	// 记录本次冲刺执行时间
	DashStartTime = GetWorld()->GetTimeSeconds();

	// 修改为根运动方式
	// 获取冲刺方向
	// FVector DashDirection = (Acceleration.IsNearlyZero() ? UpdatedComponent->GetForwardVector() : Acceleration).GetSafeNormal2D();
	// Velocity = DashImpulse * (DashDirection + FVector::UpVector * .1f);
	//
	// FQuat NewRotation = FRotationMatrix::MakeFromXZ(DashDirection, FVector::UpVector).ToQuat();
	// FHitResult Hit;
	// SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, false, Hit);
	// SetMovementMode(MOVE_Falling);

	// SetMovementMode(MOVE_Flying);
	CharacterOwner->PlayAnimMontage(DashMontage);

	DashStartDelegate.Broadcast();
}
#pragma endregion


#pragma region Replication
void UTD_CharacterMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UTD_CharacterMovementComponent, Proxy_bDashStart, COND_SkipOwner)
}

void UTD_CharacterMovementComponent::OnRep_DashStart()
{
	CharacterOwner->PlayAnimMontage(DashMontage);
	DashStartDelegate.Broadcast();
}
#pragma endregion