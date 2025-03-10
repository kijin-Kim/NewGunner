// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TargetData/NexusTargetData.h"
#include "UObject/Object.h"
#include "NexusCue.generated.h"


UENUM()
enum class ENexusCueDurationType : uint8
{
	Instant,
	Duration
};

/**
 * 
 */
UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class NEXUSACTION_API UNexusCue : public UObject
{
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override;
	void CallOnTriggered(const FNexusTargetDataHandle& InTargetDataHandle);
	void CallOnBecomeRelevant();
	void CallOnCeaseRelevant();

protected:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnTriggered"))
	void BP_OnTriggered();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnBecomeRelevant"))
	void BP_OnBecomeRelevant();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnCeaseRelevant"))
	void BP_OnCeaseRelevant();

	virtual void OnTriggered(const FNexusTargetDataHandle& InTargetDataHandle);
	virtual void OnBecomeRelevant();
	virtual void OnCeaseRelevant();

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	ENexusCueDurationType DurationType = ENexusCueDurationType::Instant;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true, EditCondition = "DurationType == ENexusCueDurationType::Duration"))
	float Duration = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true, EditCondition = "DurationType == ENexusCueDurationType::Duration"))
	bool bIsInfiniteDuration = false;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FNexusTargetDataHandle TargetDataHandle;
};
