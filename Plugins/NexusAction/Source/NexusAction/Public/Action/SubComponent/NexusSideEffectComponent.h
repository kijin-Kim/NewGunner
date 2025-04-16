// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NexusAgentBoundComponent.h"
#include "Components/ActorComponent.h"
#include "SideEffect/NexusSideEffectInstance.h"
#include "NexusSideEffectComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEXUSACTION_API UNexusSideEffectComponent : public UNexusAgentBoundComponent
{
	GENERATED_BODY()

public:
	UNexusSideEffectComponent();
	virtual void Setup(TSharedPtr<FNexusAgentInfo> InAgentInfo) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	FNexusSideEffectInstanceHandle ApplySideEffectByDef(const FNexusSideEffectInstanceDef& NewSideEffectDef, FNexusPredictionTag PredictionTag, FNexusPredictionEventSignature::FDelegate&& OnPredictionEnded = {}, FNexusPredictionEventSignature::FDelegate&& OnPredictionFailed = {});
	const FNexusSideEffectInstanceContainer& GetSideEffectInstances() const;

	void UnregisterAndRemoveSideEffect(const FNexusSideEffectInstanceHandle& SideEffectDefHandle);
private:
	FNexusSideEffectInstanceHandle RegisterAndApplySideEffect(const FNexusSideEffectInstanceDef& SideEffectInstanceDef);



private:
	UPROPERTY(Replicated)
	FNexusSideEffectInstanceContainer SideEffectInstances;
};
