// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerActionFire.h"

#include "Camera/CameraComponent.h"
#include "Event/NexusEventManagerComponent.h"
#include "GameFramework/Character.h"
#include "Gunner/Equipment/GunnerEquipment.h"
#include "Gunner/Equipment/GunnerEquipmentDef.h"
#include "Gunner/_Core/Damage/GunnerDamageContext.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"
#include "Gunner/_Core/GunnerLagCompensationComponent.h"
#include "Gunner/_Core/Damage/GunnerDamageType.h"
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
		UGunnerLagCompensationComponent* LagCompensationComponent = TargetCharacter->GetComponentByClass<UGunnerLagCompensationComponent>();
		check(LagCompensationComponent);
		LagCompensationComponent->AuthBeginRewind(TimeStamp);
	}
}

void UGunnerActionFire::AuthEndRewind(TArray<ACharacter*> LagCompensationTargetCharacters)
{
	for (ACharacter* TargetCharacter : LagCompensationTargetCharacters)
	{
		UGunnerLagCompensationComponent* LagCompensationComponent = TargetCharacter->GetComponentByClass<UGunnerLagCompensationComponent>();
		check(LagCompensationComponent);
		LagCompensationComponent->AuthEndRewind();
	}
}

TArray<FHitResult> UGunnerActionFire::HitScanTrace()
{
	AGunnerEquipment* Equipment = GetEquipment();
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

void UGunnerActionFire::AuthHitScanTraceConfirm(FNexusTargetDataHandle HitTargetDataHandle)
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
		if (Character && Character->GetComponentByClass<UGunnerLagCompensationComponent>())
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

	AGunnerEquipment* Equipment = GetEquipment();
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
		AuthApplyDamage(HitResult.GetActor(), HitResult.BoneName, HitResult.ImpactNormal);
	}
}

void UGunnerActionFire::AuthApplyDamage(AActor* HitActor, FName HitBoneName, FVector HitNormal)
{
	if (UNexusEventManagerComponent* EventManagerComponent = UNexusEventManagerComponent::GetEventManagerComponentFromActor(HitActor))
	{
		FNexusEventMessage DamageEventMessage;
		DamageEventMessage.EventTag = TAG_GameEvent_Damaged;
		AGunnerEquipment* EquipmentOwner = GetEquipment();
		APawn* EquipmentPawnOwner = Cast<APawn>(EquipmentOwner->GetOwner());
		DamageEventMessage.Instigator = EquipmentPawnOwner->GetController();


		UGunnerDamageContext* DamageContext = NewObject<UGunnerDamageContext>();

		DamageContext->Instigator = EquipmentPawnOwner->GetController();
		DamageContext->Causer = EquipmentOwner;
		DamageContext->Target = HitActor;
		DamageContext->HitNormal = HitNormal;
		DamageContext->HitBoneName = HitBoneName;

		DamageContext->DamageAmount = EquipmentOwner->GetEquipmentDef()->CalculateDamageByContext(DamageContext);
		DamageEventMessage.EventDataObject = DamageContext;

		EventManagerComponent->SendEventToActor(TAG_GameEvent_Damaged, DamageEventMessage, HitActor);
	}
}
