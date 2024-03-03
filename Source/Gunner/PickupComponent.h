// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GunnerCharacterBase.h"
#include "Components/SphereComponent.h"
#include "PickupComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPickUp, AGunnerCharacterBase*, PickUpCharacter);

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GUNNER_API UPickupComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UPickupComponent();
	virtual void BeginPlay() override;
	
private:
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	UPROPERTY(BlueprintAssignable)
	FOnPickUp OnPickUp;
};