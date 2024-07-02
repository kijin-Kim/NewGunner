// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AnimMontagePlayerComponent.generated.h"

USTRUCT()
struct GUNNER_API FRepAnimMontageData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UAnimMontage> AnimMontage;
	UPROPERTY()
	uint8 AnimMontageInstanceID = 0;
	UPROPERTY()
	float PlayRate = 0.0f;
	UPROPERTY()
	float Position = 0.0f;
	UPROPERTY()
	bool bIsStopped = false;
	UPROPERTY()
	FName StartSectionName = NAME_None;
	UPROPERTY()
	bool bIsPaused = false;
};

USTRUCT()
struct GUNNER_API FLocalAnimMontageData
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> AnimMontage;
	UPROPERTY(Transient)
	uint8 AnimMontageInstanceID = 0;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UAnimMontagePlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAnimMontagePlayerComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	float PlayMontage(UAnimMontage* AnimMontage, bool bIsThirdPerson, float InPlayRate = 1.f, FName StartSectionName = NAME_None);
	void StopMontage(UAnimMontage* AnimMontage, bool bIsThirdPerson);
	FOnMontageEnded* GetMontageEndedDelegate(UAnimMontage* AnimMontage, bool bIsThirdPerson);
	FOnMontageBlendingOutStarted* GetMontageBlendingOutStartedDelegate(UAnimMontage* AnimMontage, bool bIsThirdPerson);


	UAnimInstance* GetDesiredAnimInstance(bool bIsThirdPerson) const;

private:
	void AuthUpdateReplicatedAnimMontage();
	float LocalPlayMontage(UAnimMontage* AnimMontage, bool bIsThirdPerson, float InStartTime = 0.0f, float InPlayRate = 1.f, FName StartSectionName = NAME_None);
	UFUNCTION()
	void OnRep_ReplicatedAnimMontage();

private:
	UPROPERTY(Transient, ReplicatedUsing=OnRep_ReplicatedAnimMontage)
	FRepAnimMontageData ReplicatedAnimMontageData;
	FLocalAnimMontageData LocalAnimMontageData;
};
