// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionComponent.h"

#include "GunnerTeamAgentInterface.h"
#include "Action/NexusAction.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/Character.h"
#include "GameFramework/HUD.h"
#include "Gunner/Action/GunnerActionSet.h"
#include "Gunner/Action/GunnerAction_Fire.h"
#include "Gunner/Item/GunnerInventoryManagerComponent.h"

UGunnerActionComponent::UGunnerActionComponent()
{
	SetIsReplicatedByDefault(true);
}

void UGunnerActionComponent::InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& YL, float& YPos)
{
	Super::InternalOnShowDebugInfo(DebugTarget, HUD, Canvas, DebugDisplayInfo, YL, YPos);
	if (UGunnerInventoryManagerComponent* InventoryManagerComponent = UGunnerInventoryManagerComponent::GetInventoryManagerComponentFromActor(DebugTarget))
	{
		InventoryManagerComponent->OnShowDebugInfo(Canvas, DebugDisplayInfo, YL, YPos);
	}
}

void UGunnerActionComponent::OnSetupActionComponent()
{
	Super::OnSetupActionComponent();
	// for (UGunnerActionSet* ActionSet : ActionSets)
	// {
	// 	if (!IsOwnerActorAuthoritative())
	// 	{
	// 		continue;
	// 	}
	//
	// 	for (const auto& [Tag, Value] : ActionSet->Properties)
	// 	{
	// 		AuthAddProperty(Tag, Value);
	// 	}
	//
	// 	for (TSubclassOf<UNexusAction> ActionClass : ActionSet->ActionClasses)
	// 	{
	// 		if (ActionClass)
	// 		{
	// 			AuthAddAction(ActionClass, GetAgentActor());
	// 		}
	// 	}
	// }
}


void UGunnerActionComponent::ClientSendDebugHitConfirmedData_Implementation(const TArray<FGunnerDebugHitConfirmInfo>& DebugHitConfirmInfos)
{
	for (const FGunnerDebugHitConfirmInfo& HitConfirmInfo : DebugHitConfirmInfos)
	{
		if (HitConfirmInfo.TargetCharacter)
		{
			const FColor DebugDrawColor = HitConfirmInfo.bServerConfirmedHit ? FColor::Green : FColor::Red;
			UGunnerAction_Fire::DrawDebugHitBoxData(GetWorld(), HitConfirmInfo.DebugHitBoxInfos, DebugDrawColor, true, 0.0f);


			FString DebugString = FString::Printf(TEXT("TimeStamp: %.2f\nSnapshotFound: %s"), HitConfirmInfo.ServerRewindTimeStamp, HitConfirmInfo.bRewindSnapshotFound ? TEXT("true") : TEXT("false"));
			FVector StringLocation = HitConfirmInfo.Location;
			StringLocation.Z += HitConfirmInfo.TargetCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			DrawDebugString(GetWorld(), StringLocation, DebugString, nullptr, DebugDrawColor, -1.0f, true);
		}
	}
}
