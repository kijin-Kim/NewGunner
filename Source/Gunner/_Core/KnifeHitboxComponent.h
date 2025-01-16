// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "KnifeHitboxComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UKnifeHitboxComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	UFUNCTION(BlueprintCallable)
	void StartHitDetection();
	UFUNCTION(BlueprintCallable)
	void StopHitDetection();

private:
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	void AuthApplyDamage(AActor* HitActor);

private:
	TArray<AActor*> AlreadyHitDetectedActors;
	
};
