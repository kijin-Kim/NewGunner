// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquipmentManagerComponent.h"

#include "DisplayDebugHelpers.h"
#include "GunnerEquipment.h"
#include "Camera/CameraComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/HUD.h"
#include "Gunner/Gunner.h"
#include "Gunner/LagCompensationHitBoxCapsuleComponent.h"
#include "Gunner/_Core/HitBox.h"
#include "Gunner/_Core/HitBoxActorInterface.h"
#include "Gunner/_Core/LagCompensationComponent.h"
#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"
#include "Gunner/_Core/Input/GunnerEventMessage.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UGunnerEquipmentManagerComponent::UGunnerEquipmentManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	EquipmentSlots.SetNum(MaxSlots);
	SetIsReplicatedByDefault(true);


	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		AHUD::OnShowDebugInfo.AddStatic(&ThisClass::OnShowDebugInfo);
	}
}

void UGunnerEquipmentManagerComponent::InitEquipmentManagerComponent()
{
	check(InitialEquipmentClasses.Num() <= 3);
	for (int i = 0; i < InitialEquipmentClasses.Num(); ++i)
	{
		if (InitialEquipmentClasses[i])
		{
			AuthAddEquipmentToSlot(i, InitialEquipmentClasses[i]);
		}
	}
}

void UGunnerEquipmentManagerComponent::RelaseEquipmentManagerComponent()
{
	if (GetOwner()->HasAuthority())
	{
		AuthRemoveAllEquipments();
	}
}

void UGunnerEquipmentManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGunnerEquipmentManagerComponent, EquipmentSlots);
	DOREPLIFETIME(UGunnerEquipmentManagerComponent, CurrentEquippedEquipment);
}

void UGunnerEquipmentManagerComponent::AuthAddEquipmentToSlot(int32 SlotIndex, TSubclassOf<AGunnerEquipment> EquipmentClass)
{
	if (!EquipmentSlots.IsValidIndex(SlotIndex))
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	AGunnerEquipment* NewEquipment = GetWorld()->SpawnActor<AGunnerEquipment>(EquipmentClass, SpawnParams);
	NewEquipment->OnAcquired();

	if (EquipmentSlots[SlotIndex])
	{
		EquipmentSlots[SlotIndex]->OnLost();
	}

	EquipmentSlots[SlotIndex] = NewEquipment;
}

void UGunnerEquipmentManagerComponent::AuthRemoveAllEquipments()
{
	for (AGunnerEquipment* Equipment : EquipmentSlots)
	{
		if (Equipment)
		{
			Equipment->Destroy();
		}
	}
	EquipmentSlots.Empty();
}

void UGunnerEquipmentManagerComponent::SetCurrentEquipmentByIndex(int32 SlotIndex)
{
	if (EquipmentSlots.IsValidIndex(SlotIndex) && EquipmentSlots[SlotIndex] != CurrentEquippedEquipment)
	{
		AGunnerEquipment* LastEquipment = CurrentEquippedEquipment;
		CurrentEquippedEquipment = EquipmentSlots[SlotIndex];
		OnRep_CurrentEquippedEquipment(LastEquipment);
	}
}

AGunnerEquipment* UGunnerEquipmentManagerComponent::GetEquipmentByIndex(int32 SlotIndex) const
{
	if (EquipmentSlots.IsValidIndex(SlotIndex))
	{
		return EquipmentSlots[SlotIndex];
	}
	return nullptr;
}

AGunnerEquipment* UGunnerEquipmentManagerComponent::GetCurrentEquippedEquipment() const
{
	return CurrentEquippedEquipment;
}

void UGunnerEquipmentManagerComponent::LocalHitScan(TArray<FHitResult>& OutHitResults, const TArray<AActor*>& ActorsToIgnore)
{
	AActor* ActorOwner = GetOwner();
	UWorld* World = ActorOwner->GetWorld();
	UCameraComponent* CameraComponet = ActorOwner->GetComponentByClass<UCameraComponent>();
	FVector CameraLocation = CameraComponet->GetComponentLocation();
	FVector CameraForward = CameraComponet->GetForwardVector();

	FCollisionQueryParams CollisionQueryParams;
	TArray<AActor*> IgnoredActors = {ActorOwner, GetCurrentEquippedEquipment()};
	IgnoredActors.Append(ActorsToIgnore);
	CollisionQueryParams.AddIgnoredActors(IgnoredActors);
	World->LineTraceMultiByChannel(OutHitResults,
	                               CameraLocation,
	                               CameraLocation + CameraForward * 10000.0f,
	                               ECollisionChannel::ECC_Visibility, CollisionQueryParams, FCollisionResponseParams(ECR_Overlap));

	FlushPersistentDebugLines(World);
	DrawDebugHitBoxByHitResult(OutHitResults);
}

void UGunnerEquipmentManagerComponent::AuthApplyDamage(const TArray<FHitResult>& HitResults)
{
	TArray<AActor*> AlreadyHitActors;
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && AlreadyHitActors.Find(HitActor) == INDEX_NONE)
		{
			AlreadyHitActors.Add(HitActor);
			if (UGunnerEventManagerComponent* EventManagerComponent = UGunnerEventManagerComponent::GetEventManagerComponentFromActor(HitActor))
			{
				FGunnerEventMessage HitScanMessage;
				HitScanMessage.Instigator = GetOwner();
				UGunnerHitMessageData* HitMessageData = NewObject<UGunnerHitMessageData>();
				HitMessageData->HitBoneName = HitResult.BoneName;
				HitMessageData->HitNormal = HitResult.Normal;
				HitMessageData->HitEquipment = CurrentEquippedEquipment;
				HitScanMessage.EventDataObject = HitMessageData;

				EventManagerComponent->SendEventToActor(FGameplayTag::RequestGameplayTag(FName("GameEvent.Damaged")), HitScanMessage, HitActor);
			}
		}
	}
}

void UGunnerEquipmentManagerComponent::ServerRequestHitScanConfirm_Implementation(const TArray<FClientHitScanData>& ClientHitScanData, float TimeStamp)
{
	GR_LOG_SUB(LogGunner, Display, TEXT("Time Discrpency: %f"), GetWorld()->GetTimeSeconds() - TimeStamp);

	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	check(GameMode);
	ULagCompensationComponent* LagCompensationComponent = GameMode->GetComponentByClass<ULagCompensationComponent>();
	check(LagCompensationComponent);

	TArray<AActor*> RewindTargets;
	for (const FClientHitScanData& HitScanData : ClientHitScanData)
	{
		if (HitScanData.HitActor && HitScanData.HitActor->Implements<UHitBoxActorInterface>())
		{
			RewindTargets.AddUnique(HitScanData.HitActor);
		}
	}


	for (AActor* RewindTarget : RewindTargets)
	{
		if (IHitBoxActorInterface* HitBoxActor = Cast<IHitBoxActorInterface>(RewindTarget))
		{
			TArray<FHitBox> HitBoxes = HitBoxActor->CollectAndGetHitBoxes();
			NetMulticastSendCurrentHitBoxes(HitBoxes, FName(), FVector(), FColor::Black, FColor::Cyan);
		}
	}


	TArray<AActor*> ActorsToIgnoreWhenRewounded;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UHitBoxActorInterface::StaticClass(), ActorsToIgnoreWhenRewounded);


	
	LagCompensationComponent->BeginRewind(TimeStamp, RewindTargets);
	TArray<FHitResult> HitResults;
	LocalHitScan(HitResults, ActorsToIgnoreWhenRewounded);
	TArray<AActor*> AlreadyHitActors;
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		IHitBoxActorInterface* HitBoxActor = Cast<IHitBoxActorInterface>(HitActor);
		if (HitBoxActor && AlreadyHitActors.Find(HitActor) == INDEX_NONE)
		{
			AlreadyHitActors.Add(HitActor);
			TArray<FHitBox> HitBoxes = HitBoxActor->CollectAndGetHitBoxes();
			if (ULagCompensationHitBoxCapsuleComponent* HitBoxComponent = Cast<ULagCompensationHitBoxCapsuleComponent>(HitResult.GetComponent()))
			{
				NetMulticastSendCurrentHitBoxes(HitBoxes, HitBoxComponent->GetBoneName(), HitResult.Location, FColor::Magenta, FColor::Yellow);
			}
		}
	}
	LagCompensationComponent->EndRewind();


	// TSet<IHitBoxActorInterface*> ClientHitHitBoxActors;
	// for (const FClientHitScanData& HitScanData : ClientHitScanData)
	// {
	// 	if (HitScanData.HitActor && HitScanData.HitActor->Implements<UHitBoxActorInterface>())
	// 	{
	// 		ClientHitHitBoxActors.Add(Cast<IHitBoxActorInterface>(HitScanData.HitActor));
	// 	}
	// }
	//
	// TSet<IHitBoxActorInterface*> ServerHitHitBoxActors;
	// for (const FHitResult& HitResult : HitResults)
	// {
	// 	if (HitResult.GetActor() && HitResult.GetActor()->Implements<UHitBoxActorInterface>())
	// 	{
	// 		ServerHitHitBoxActors.Add(Cast<IHitBoxActorInterface>(HitResult.GetActor()));
	// 	}
	// }
	//
	// for (IHitBoxActorInterface* HitBoxActor : ClientHitHitBoxActors)
	// {
	// 	if (!ServerHitHitBoxActors.Contains(HitBoxActor))
	// 	{
	// 		TArray<FHitBox> HitBoxes = HitBoxActor->CollectAndGetHitBoxes();
	// 		NetMulticastSendCurrentHitBoxes(HitBoxes, FName(), FVector());
	// 		return;
	// 	}
	// }
	//
	// for (const FHitResult& HitResult : HitResults)
	// {
	// 	AActor* ServerHitActor = HitResult.GetActor();
	// 	if (!ServerHitActor || !ServerHitActor->Implements<UHitBoxActorInterface>())
	// 	{
	// 		continue;
	// 	}
	// 	const FClientHitScanData* ClientHitScanDataPtr = ClientHitScanData.FindByPredicate([ServerHitActor](const FClientHitScanData& HitScanData)
	// 	{
	// 		return HitScanData.HitActor == ServerHitActor;
	// 	});
	//
	// 	if (!ClientHitScanDataPtr)
	// 	{
	// 		UE_DEBUG_BREAK();
	// 		continue;
	// 	}
	//
	// 	if (ClientHitScanDataPtr->HitBoneName != HitResult.BoneName)
	// 	{
	// 		IHitBoxActorInterface* ServerHitBoxActor = Cast<IHitBoxActorInterface>(ServerHitActor);
	// 		NetMulticastSendCurrentHitBoxes(ServerHitBoxActor->CollectAndGetHitBoxes(), HitResult.BoneName, HitResult.Location);
	// 		return;
	// 	}
	// }


	AuthApplyDamage(HitResults);
}

void UGunnerEquipmentManagerComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	AActor* DebugTarget = HUD->GetCurrentDebugTargetActor();
	if (!DebugTarget)
	{
		return;
	}

	if (UGunnerEquipmentManagerComponent* EquipmentManager = DebugTarget->GetComponentByClass<UGunnerEquipmentManagerComponent>())
	{
		EquipmentManager->InternalOnShowDebugInfo(DebugTarget, HUD, Canvas, DebugDisplayInfo, X, Y);
	}
}

void UGunnerEquipmentManagerComponent::InternalOnShowDebugInfo(AActor* Actor, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	if (HUD->ShouldDisplayDebug(TEXT("ActionSystem")))
	{
		DisplayDebugManager.SetFont(GEngine->GetTinyFont());
		DisplayDebugManager.SetDrawColor(FColor::Orange);
		DisplayDebugManager.DrawString(FString::Printf(TEXT("Current Equipped Equipment: %s"), CurrentEquippedEquipment ? *CurrentEquippedEquipment->GetName() : TEXT("None")));

		DisplayDebugManager.SetDrawColor(FColor::White);

		if (CurrentEquippedEquipment)
		{
			CurrentEquippedEquipment->OnShowDebugInfo(HUD, Canvas, DebugDisplayInfo, X, Y);
		}
	}
}

void UGunnerEquipmentManagerComponent::OnRep_CurrentEquippedEquipment(AGunnerEquipment* OldEquippedEquipment)
{
	if (OldEquippedEquipment)
	{
		OldEquippedEquipment->OnUnequipped();
	}

	if (CurrentEquippedEquipment)
	{
		CurrentEquippedEquipment->OnEquipped();
	}

	OnEquippedEquipmentChangedDelegate.Broadcast(CurrentEquippedEquipment, OldEquippedEquipment);
}

void UGunnerEquipmentManagerComponent::OnRep_EquipmentSlots(const TArray<AGunnerEquipment*>& OldEquipmentSlots)
{
	for (AGunnerEquipment* Equipment : EquipmentSlots)
	{
		if (Equipment && !OldEquipmentSlots.Contains(Equipment))
		{
			Equipment->OnAcquired();
		}
	}

	for (AGunnerEquipment* Equipment : OldEquipmentSlots)
	{
		if (Equipment && !EquipmentSlots.Contains(Equipment))
		{
			Equipment->OnLost();
		}
	}
}

void UGunnerEquipmentManagerComponent::DrawDebugHitBoxByHitResult(const TArray<FHitResult>& HitResults)
{
	TArray<AActor*> AlreadyHitActors;
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && HitActor->Implements<UHitBoxActorInterface>() && AlreadyHitActors.Find(HitActor) == INDEX_NONE)
		{
			AlreadyHitActors.Add(HitActor);
			IHitBoxActorInterface* HitBoxActor = Cast<IHitBoxActorInterface>(HitActor);
			TArray<FHitBox> HitBoxes = HitBoxActor->CollectAndGetHitBoxes();
			for (const FHitBox& HitBox : HitBoxes)
			{
				HitBox.DrawDebug(GetWorld(), HitResult.BoneName == HitBox.BoneName ? FColor::Green : FColor::Black, true);
			}

			DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 15.0f, FColor::Red, true);
		}
	}
}

void UGunnerEquipmentManagerComponent::NetMulticastSendCurrentHitBoxes_Implementation(const TArray<FHitBox>& HitBoxes, FName HitBoneName, FVector HitLocation, FColor HitColor, FColor NonHitColor)
{
	for (const FHitBox& HitBox : HitBoxes)
	{
		HitBox.DrawDebug(GetWorld(), (!HitBoneName.IsNone() && HitBoneName == HitBox.BoneName) ? HitColor : NonHitColor, true);
	}

	if (HitLocation != FVector::ZeroVector)
	{
		DrawDebugPoint(GetWorld(), HitLocation, 15.0f, FColor::Orange, true);
	}
}
