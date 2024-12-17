// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerEquipmentManagerComponent.h"

#include "DisplayDebugHelpers.h"
#include "GunnerEquipment.h"
#include "Camera/CameraComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/HUD.h"
#include "Gunner/Gunner.h"
#include "Gunner/LagCompensationHitBoxCapsuleComponent.h"
#include "Gunner/PoseSnapshotCharacter.h"
#include "Gunner/_Core/HitBox.h"
#include "Gunner/_Core/HitBoxActorInterface.h"
#include "Gunner/_Core/LagCompComponent.h"
#include "Gunner/_Core/LagCompensationComponent.h"
#include "Gunner/_Core/Event/GunnerEventManagerComponent.h"
#include "Gunner/_Core/Input/GunnerEventMessage.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SphylElem.h"


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

void UGunnerEquipmentManagerComponent::LocalHitScan2(TArray<FHitResult>& OutHitResults, const FCollisionQueryParams& CollisionQueryParams)
{
	AActor* ActorOwner = GetOwner();
	UWorld* World = ActorOwner->GetWorld();
	UCameraComponent* CameraComponet = ActorOwner->GetComponentByClass<UCameraComponent>();
	FVector CameraLocation = CameraComponet->GetComponentLocation();
	FVector CameraForward = CameraComponet->GetForwardVector();

	World->LineTraceMultiByChannel(OutHitResults,
	                               CameraLocation,
	                               CameraLocation + CameraForward * 10000.0f,
	                               ECollisionChannel::ECC_Visibility, CollisionQueryParams, FCollisionResponseParams(ECR_Overlap));

	FlushPersistentDebugLines(World);
	DrawDebugHitBoxByHitResult(OutHitResults);
}

void UGunnerEquipmentManagerComponent::AuthApplyDamage(AActor* HitActor, FName BoneName, FVector HitNormal)
{
	if (UGunnerEventManagerComponent* EventManagerComponent = UGunnerEventManagerComponent::GetEventManagerComponentFromActor(HitActor))
	{
		FGunnerEventMessage HitScanMessage;
		HitScanMessage.Instigator = GetOwner();
		UGunnerHitMessageData* HitMessageData = NewObject<UGunnerHitMessageData>();
		HitMessageData->HitBoneName = BoneName;
		HitMessageData->HitNormal = HitNormal;
		HitMessageData->HitEquipment = CurrentEquippedEquipment;
		HitScanMessage.EventDataObject = HitMessageData;

		EventManagerComponent->SendEventToActor(FGameplayTag::RequestGameplayTag(FName("GameEvent.Damaged")), HitScanMessage, HitActor);
	}
}

void UGunnerEquipmentManagerComponent::ServerRequestHitScanConfirm_Implementation(const TArray<FClientHitScanData>& ClientHitScanData, float TimeStamp)
{
	
	TArray<ACharacter*> LagCompensationTargetCharacters;
	for (const FClientHitScanData& HitScanData : ClientHitScanData)
	{
		ACharacter* Character = Cast<ACharacter>(HitScanData.HitActor);
		if (Character && Character->GetComponentByClass<ULagCompComponent>())
		{
			LagCompensationTargetCharacters.AddUnique(Character);
		}
	}


	for (ACharacter* TargetCharacter : LagCompensationTargetCharacters)
	{
		ULagCompComponent* LagCompensationComponent = TargetCharacter->GetComponentByClass<ULagCompComponent>();
		check(LagCompensationComponent);
		LagCompensationComponent->AuthBeginRewind(TimeStamp);
	}


	FCollisionQueryParams CollisionQueryParams;
	TArray<AActor*> LagCompensatableActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), LagCompensatableActors);
	for (AActor* CompensatableActor : LagCompensatableActors)
	{
		ACharacter* Character = Cast<ACharacter>(CompensatableActor);
		check(Character);
		CollisionQueryParams.AddIgnoredComponent(Character->GetMesh());
	}

	TArray<FHitResult> HitResults;
	TArray<AActor*> IgnoredActors = {GetOwner(), GetCurrentEquippedEquipment()};
	CollisionQueryParams.AddIgnoredActors(IgnoredActors);
	LocalHitScan2(HitResults, CollisionQueryParams);


	for (ACharacter* TargetCharacter : LagCompensationTargetCharacters)
	{
		ULagCompComponent* LagCompensationComponent = TargetCharacter->GetComponentByClass<ULagCompComponent>();
		check(LagCompensationComponent);
		LagCompensationComponent->AuthEndRewind();
	}

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

void UGunnerEquipmentManagerComponent::Draw(ACharacter* DummyCharacter, FColor Color)
{
	TArray<FHitBox> HitBoxes;
	UPhysicsAsset* PhysAsset = DummyCharacter->GetMesh()->GetPhysicsAsset();
	check(PhysAsset);
	for (USkeletalBodySetup* BodySetup : PhysAsset->SkeletalBodySetups)
	{
		FTransform BodyTransform = DummyCharacter->GetMesh()->GetSocketTransform(BodySetup->BoneName);
		for (FKSphylElem& SphylElem : BodySetup->AggGeom.SphylElems)
		{
			FTransform HitBoxTransform = SphylElem.GetTransform() * BodyTransform;
			HitBoxes.Add({
				.Transform = HitBoxTransform,
				.HalfHeight = SphylElem.GetScaledHalfLength(FVector(1.0f, 1.0f, 1.0f)),
				.Radius = SphylElem.GetScaledRadius(FVector(1.0f, 1.0f, 1.0f)),
				.BoneName = BodySetup->BoneName,
			});
		}
	}
	for (const FHitBox& HitBox : HitBoxes)
	{
		HitBox.DrawDebug(GetWorld(), Color, true);
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
