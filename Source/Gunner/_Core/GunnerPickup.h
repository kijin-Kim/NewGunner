// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GunnerPickup.generated.h"

class UBoxComponent;


UCLASS()
class GUNNER_API AGunnerPickup : public AActor
{
	GENERATED_BODY()

public:
	AGunnerPickup();
	UFUNCTION(BlueprintNativeEvent)
	bool CanPickup(AActor* OtherActor, UActorComponent* OtherComponent) const;
	UFUNCTION(BlueprintNativeEvent)
	void OnPickup(AActor* OtherActor, UActorComponent* OtherComponent);
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* BoxComponent;
};
