// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimMontagePlayerComponent.h"

#include "AnimMontagePlayerInterface.h"
#include "Net/UnrealNetwork.h"


UAnimMontagePlayerComponent::UAnimMontagePlayerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UAnimMontagePlayerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UAnimMontagePlayerComponent, ReplicatedAnimMontageData, COND_SimulatedOnly);
}

void UAnimMontagePlayerComponent::BeginPlay()
{
	Super::BeginPlay();
	check(GetOwner()->Implements<UAnimMontagePlayerInterface>() && "Owner Must Implements AnimMontagePlayerInterface");
}

void UAnimMontagePlayerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (GetOwner()->HasAuthority())
	{
		AuthUpdateReplicatedAnimMontage();
	}
}

float UAnimMontagePlayerComponent::PlayMontage(UAnimMontage* AnimMontage, bool bIsThirdPerson, float InPlayRate, FName StartSectionName)
{
	const float Duration = LocalPlayMontage(AnimMontage, bIsThirdPerson, 0.0f, InPlayRate, StartSectionName);

	if (bIsThirdPerson)
	{
		if (GetOwner()->HasAuthority())
		{
			const uint8 CurrentID = ReplicatedAnimMontageData.AnimMontageInstanceID;
			ReplicatedAnimMontageData.AnimMontageInstanceID = CurrentID < UINT8_MAX ? CurrentID + 1 : 0;
		}
		LocalAnimMontageData.AnimMontage = Duration > 0.0f ? AnimMontage : nullptr;
	}
	return Duration;
}

void UAnimMontagePlayerComponent::SetMontageEndDelegate(UAnimMontage* AnimMontage, bool bIsThirdPerson, FOnMontageEnded& OnMontageEnded)
{
	UAnimInstance* AnimInstance = GetDesiredAnimInstance(bIsThirdPerson);
	if (AnimMontage && AnimInstance)
	{
		AnimInstance->Montage_SetEndDelegate(OnMontageEnded, AnimMontage);
	}
}

void UAnimMontagePlayerComponent::SetMontageBlendingOutStartedDelegate(UAnimMontage* AnimMontage, bool bIsThirdPerson, FOnMontageBlendingOutStarted& OnMontageBlendingOutStarted)
{
	UAnimInstance* AnimInstance = GetDesiredAnimInstance(bIsThirdPerson);
	if (AnimMontage && AnimInstance)
	{
		AnimInstance->Montage_SetBlendingOutDelegate(OnMontageBlendingOutStarted, AnimMontage);
	}
}

UAnimInstance* UAnimMontagePlayerComponent::GetDesiredAnimInstance(bool bIsThirdPerson) const
{
	USkeletalMeshComponent* TargetMesh = bIsThirdPerson
		                                     ? IAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(GetOwner())
		                                     : IAnimMontagePlayerInterface::Execute_GetFirstPersonMeshComponent(GetOwner());
	return (TargetMesh) ? TargetMesh->GetAnimInstance() : nullptr;
}

void UAnimMontagePlayerComponent::AuthUpdateReplicatedAnimMontage()
{
	USkeletalMeshComponent* ThirdPersonMeshComponent = IAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(GetOwner());
	if (UAnimInstance* AnimInstance = ThirdPersonMeshComponent->GetAnimInstance())
	{
		ReplicatedAnimMontageData.AnimMontage = AnimInstance->GetCurrentActiveMontage();
		ReplicatedAnimMontageData.PlayRate = AnimInstance->Montage_GetPlayRate(ReplicatedAnimMontageData.AnimMontage);
		ReplicatedAnimMontageData.StartSectionName = AnimInstance->Montage_GetCurrentSection(ReplicatedAnimMontageData.AnimMontage);
		ReplicatedAnimMontageData.Position = AnimInstance->Montage_GetPosition(ReplicatedAnimMontageData.AnimMontage);
		ReplicatedAnimMontageData.bIsStopped = AnimInstance->Montage_GetIsStopped(ReplicatedAnimMontageData.AnimMontage);
		ReplicatedAnimMontageData.bIsPaused = !AnimInstance->Montage_IsPlaying(ReplicatedAnimMontageData.AnimMontage);
	}
}

float UAnimMontagePlayerComponent::LocalPlayMontage(UAnimMontage* AnimMontage, bool bIsThirdPerson, float InStartTime, float InPlayRate, FName StartSectionName)
{
	UAnimInstance* AnimInstance = GetDesiredAnimInstance(bIsThirdPerson);
	if (AnimMontage && AnimInstance)
	{
		float const Duration = AnimInstance->Montage_Play(AnimMontage, InPlayRate, EMontagePlayReturnType::MontageLength, InStartTime);
		if (Duration > 0.f)
		{
			// Start at a given Section.
			if (StartSectionName != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(StartSectionName, AnimMontage);
			}

			return Duration;
		}
	}

	return 0.f;
}

void UAnimMontagePlayerComponent::OnRep_ReplicatedAnimMontage()
{
	// Server로부터 AnimMontage정보를 받아 Simulated Proxy에서 이 정보를 확인하고 갱신함.
	USkeletalMeshComponent* ThirdPersonMeshComponent = IAnimMontagePlayerInterface::Execute_GetThirdPersonMeshComponent(GetOwner());
	UAnimInstance* AnimInstance = ThirdPersonMeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	if (ReplicatedAnimMontageData.AnimMontage && (LocalAnimMontageData.AnimMontage != ReplicatedAnimMontageData.AnimMontage || LocalAnimMontageData.
		AnimMontageInstanceID != ReplicatedAnimMontageData.AnimMontageInstanceID))
	{
		LocalAnimMontageData.AnimMontageInstanceID = ReplicatedAnimMontageData.AnimMontageInstanceID;
		LocalAnimMontageData.AnimMontage = ReplicatedAnimMontageData.AnimMontage;
		LocalPlayMontage(ReplicatedAnimMontageData.AnimMontage, true);
		return;
	}

	if (LocalAnimMontageData.AnimMontage)
	{
		if (ReplicatedAnimMontageData.bIsStopped)
		{
			AnimInstance->Montage_Stop(LocalAnimMontageData.AnimMontage->BlendOut.GetBlendTime(), ReplicatedAnimMontageData.AnimMontage);
		}

		if (ReplicatedAnimMontageData.bIsPaused)
		{
			AnimInstance->Montage_Pause(LocalAnimMontageData.AnimMontage);
		}

		if (AnimInstance->Montage_GetPlayRate(LocalAnimMontageData.AnimMontage) != ReplicatedAnimMontageData.PlayRate)
		{
			AnimInstance->Montage_SetPlayRate(LocalAnimMontageData.AnimMontage, ReplicatedAnimMontageData.PlayRate);
		}
		if (AnimInstance->Montage_GetCurrentSection(LocalAnimMontageData.AnimMontage) != ReplicatedAnimMontageData.StartSectionName)
		{
			AnimInstance->Montage_JumpToSection(ReplicatedAnimMontageData.StartSectionName);
			return;
		}

		// AnimMontage Position의 최대 오류 허용치
		const float MONTAGE_POSITION_DELTA_TOLERANCE = 0.3f;
		const float LocalMontagePosition = AnimInstance->Montage_GetPosition(LocalAnimMontageData.AnimMontage);
		// Server와 Simulated Proxy사이의 Position의 차이가 허용치를 넘으면 Server의 값으로 갱신함.
		if (!FMath::IsNearlyEqual(LocalMontagePosition, ReplicatedAnimMontageData.Position, MONTAGE_POSITION_DELTA_TOLERANCE))
		{
			AnimInstance->Montage_SetPosition(LocalAnimMontageData.AnimMontage, ReplicatedAnimMontageData.Position);
		}
	}
}
