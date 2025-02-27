// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionFire.h"

#include "Camera/CameraComponent.h"
#include "Event/NexusEventManagerComponent.h"
#include "GameFramework/Character.h"
#include "Gunner/Equipment/GunnerEquipment.h"
#include "Gunner/Equipment/TraceHitMessageData.h"
#include "Gunner/_Core/LagCompensationComponent.h"
#include "TargetData/GunnerTargetData_Hit.h"
#include "Kismet/GameplayStatics.h"


TArray<FHitResult> UGunnerActionFire::FilterDuplicateHitResultsByActor(const TArray<FHitResult>& HitResults)
{
	TArray<AActor*> HitActors;
	TArray<FHitResult> FilteredHitResults;
	for (const FHitResult& HitResult : HitResults)
	{
		if (!HitActors.Contains(HitResult.GetActor()))
		{
			HitActors.Add(HitResult.GetActor());
			FilteredHitResults.Add(HitResult);
		}
	}
	return FilteredHitResults;
}

TArray<AActor*> UGunnerActionFire::GetUniqueActorsFromHitResults(const TArray<FHitResult>& HitResults)
{
	FilterDuplicateHitResultsByActor(HitResults);
	TArray<AActor*> HitActors;
	for (const FHitResult& HitResult : HitResults)
	{
		HitActors.Add(HitResult.GetActor());
	}
	return HitActors;
}

TArray<AActor*> UGunnerActionFire::GetIgnoredActorsByTeam(AActor* EquipmentActorOwner)
{
	TArray<AActor*> IgnoredActors;
	IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(EquipmentActorOwner);
	if (!ensure(TeamAgentInterface))
	{
		return {};
	}

	TArray<AActor*> OtherAgents;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UGenericTeamAgentInterface::StaticClass(), OtherAgents);
	for (AActor* OtherAgent : OtherAgents)
	{
		if (TeamAgentInterface->GetTeamAttitudeTowards(*OtherAgent) != ETeamAttitude::Hostile)
		{
			IgnoredActors.Add(OtherAgent);
		}
	}

	return IgnoredActors;
}

void UGunnerActionFire::AuthBeginRewind(TArray<ACharacter*> LagCompensationTargetCharacters, float TimeStamp)
{
	for (ACharacter* TargetCharacter : LagCompensationTargetCharacters)
	{
		ULagCompensationComponent* LagCompensationComponent = TargetCharacter->GetComponentByClass<ULagCompensationComponent>();
		check(LagCompensationComponent);
		LagCompensationComponent->AuthBeginRewind(TimeStamp);
	}
}

void UGunnerActionFire::AuthEndRewind(TArray<ACharacter*> LagCompensationTargetCharacters)
{
	for (ACharacter* TargetCharacter : LagCompensationTargetCharacters)
	{
		ULagCompensationComponent* LagCompensationComponent = TargetCharacter->GetComponentByClass<ULagCompensationComponent>();
		check(LagCompensationComponent);
		LagCompensationComponent->AuthEndRewind();
	}
}

TArray<FHitResult> UGunnerActionFire::HitScanTrace()
{
	AActor* EquipmentActorOwner = Equipment->GetOwner();
	UWorld* World = EquipmentActorOwner->GetWorld();
	UCameraComponent* CameraComponet = EquipmentActorOwner->GetComponentByClass<UCameraComponent>();
	FVector CameraLocation = CameraComponet->GetComponentLocation();
	FVector CameraForward = CameraComponet->GetForwardVector();

	FCollisionQueryParams CollisionQueryParams;
	TArray<AActor*> IgnoredActors = {Equipment, EquipmentActorOwner};

	CollisionQueryParams.AddIgnoredActors(IgnoredActors);
	CollisionQueryParams.AddIgnoredActors(GetIgnoredActorsByTeam(EquipmentActorOwner));

	TArray<FHitResult> HitResults;
	World->LineTraceMultiByChannel(HitResults,
	                               CameraLocation,
	                               CameraLocation + CameraForward * 10000.0f, // TODO: 설정파일을 통해 설정할 수 있도록 변경
	                               ECollisionChannel::ECC_Visibility, CollisionQueryParams, FCollisionResponseParams(ECR_Overlap));

	return HitResults;
}

void UGunnerActionFire::HitScanTraceConfirm(FNexusTargetDataHandle HitTargetDataHandle)
{
	if (HitTargetDataHandle.IsValid() && HitTargetDataHandle.GetData()->GetStructType() != FGunnerTargetData_Hit::StaticStruct())
	{
		return;
	}

	TSharedPtr<FGunnerTargetData_Hit> HitTargetData = StaticCastSharedPtr<FGunnerTargetData_Hit>(HitTargetDataHandle.GetData());
	check(HitTargetData.IsValid());

	TArray<AActor*> HitActors = GetUniqueActorsFromHitResults(HitTargetData->HitResults);

	TArray<ACharacter*> LagCompensationTargetCharacters;
	for (AActor* HitActor : HitActors)
	{
		ACharacter* Character = Cast<ACharacter>(HitActor);
		if (Character && Character->GetComponentByClass<ULagCompensationComponent>())
		{
			LagCompensationTargetCharacters.AddUnique(Character);
		}
	}

	AuthBeginRewind(LagCompensationTargetCharacters, HitTargetData->TimeStamp);


	// line trace portion
	FCollisionQueryParams CollisionQueryParams;
	TArray<AActor*> LagCompensatableActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), LagCompensatableActors);
	for (AActor* CompensatableActor : LagCompensatableActors)
	{
		ACharacter* Character = Cast<ACharacter>(CompensatableActor);
		check(Character);
		CollisionQueryParams.AddIgnoredComponent(Character->GetMesh());
	}

	AActor* EquipmentActorOwner = Equipment->GetOwner();
	TArray<AActor*> IgnoredActors = {Equipment, EquipmentActorOwner};
	CollisionQueryParams.AddIgnoredActors(IgnoredActors);
	CollisionQueryParams.AddIgnoredActors(GetIgnoredActorsByTeam(EquipmentActorOwner));

	TArray<FHitResult> HitResults = HitScanTrace();

	AuthEndRewind(LagCompensationTargetCharacters);

	AuthApplyDamageByHitResults(HitResults);
}

void UGunnerActionFire::AuthApplyDamageByHitResults(const TArray<FHitResult>& HitResults)
{
	for (const FHitResult& HitResult : FilterDuplicateHitResultsByActor(HitResults))
	{
		AuthApplyDamage(HitResult.GetActor(), HitResult.BoneName, HitResult.ImpactNormal, 1.0f);
	}
}

void UGunnerActionFire::AuthApplyDamage(AActor* HitActor, FName BoneName, FVector HitNormal, float DamageAmount)
{
	if (UNexusEventManagerComponent* EventManagerComponent = UNexusEventManagerComponent::GetEventManagerComponentFromActor(HitActor))
	{
		FNexusEventMessage HitScanMessage;
		HitScanMessage.EventTag = FGameplayTag::RequestGameplayTag(FName("GameEvent.Damaged"));
		APawn* EquipmentPawnOwner = Cast<APawn>(Equipment->GetOwner());
		HitScanMessage.Instigator = EquipmentPawnOwner->GetController();
		UGunnerHitMessageData* HitMessageData = NewObject<UGunnerHitMessageData>();
		HitMessageData->HitBoneName = BoneName;
		HitMessageData->HitNormal = HitNormal;
		HitMessageData->HitEquipment = Equipment;
		HitMessageData->DamageAmount = DamageAmount;
		HitScanMessage.EventDataObject = HitMessageData;

		EventManagerComponent->SendEventToActor(FGameplayTag::RequestGameplayTag(FName("GameEvent.Damaged")), HitScanMessage, HitActor);
	}
}
