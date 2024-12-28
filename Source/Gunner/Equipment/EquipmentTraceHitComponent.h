// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipmentTraceHitComponent.generated.h"


class AGunnerEquipment;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UEquipmentTraceHitComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEquipmentTraceHitComponent();
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "TraceHit"))
	void BP_TraceHit(TArray<FHitResult>& OutHitResults);
	virtual void BeginPlay() override;
	virtual void TraceHit(TArray<FHitResult>& OutHitResults, const FCollisionQueryParams& CollisionQueryParams);
	UFUNCTION(BlueprintNativeEvent)
	float CalculateDamage(const FHitResult& HitResult) const;

private:
	UFUNCTION(Server, Reliable)
	void ServerRequestHitScanConfirm(const TArray<AActor*>& HitActors, float TimeStamp);
	void AuthApplyDamageByHitResults(const TArray<FHitResult>& HitResults);
	void AuthApplyDamage(AActor* HitActor, FName BoneName, FVector HitNormal, float DamageAmount);
	

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AGunnerEquipment* EquipmentOwner;
};
