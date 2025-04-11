// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NexusAnimMontagePlayerComponent.generated.h"

// TODO: NetSerialize구현을 통한 Atomic Replication
USTRUCT()
struct NEXUSACTION_API FNexusAnimMontageRepData
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
struct NEXUSACTION_API FNexusLocalAnimMontageData
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> AnimMontage;
	UPROPERTY(Transient)
	uint8 AnimMontageInstanceID = 0;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEXUSACTION_API UNexusAnimMontagePlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNexusAnimMontagePlayerComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION(BlueprintCallable)
	float PlayMontage(UAnimMontage* AnimMontage, bool bIsThirdPerson, float InPlayRate = 1.f, FName StartSectionName = NAME_None);
	UFUNCTION(BlueprintCallable)
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
	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedAnimMontage)
	FNexusAnimMontageRepData ReplicatedAnimMontageData;
	FNexusLocalAnimMontageData LocalAnimMontageData;
};
