// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SideEffect/NexusSideEffectDef.h"
#include "NexusSideEffectComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEXUSACTION_API UNexusSideEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNexusSideEffectComponent();
	void Init(AActor* Actor);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void TriggerSideEffectByDef(const FNexusSideEffectDef& NewSideEffectDef, FNexusPredictionTag PredictionTag, FNexusPredictionEventSignature::FDelegate&& OnPredictionEnded = {}, FNexusPredictionEventSignature::FDelegate&& OnPredictionFailed = {});
	void RemoveSideEffect(FNexusSideEffectDefHandle SideEffectDefHandle);

	const FNexusSideEffectDefContainer& GetSideEffectDefs() const;

private:
	UPROPERTY(Replicated)
	FNexusSideEffectDefContainer SideEffectDefs;
};
