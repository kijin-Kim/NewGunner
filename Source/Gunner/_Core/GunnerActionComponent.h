// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Action/NexusActionComponent.h"
#include "Debug/GunnerDebugHitData.h"
#include "GunnerActionComponent.generated.h"


class UGunnerActionSet;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerActionComponent : public UNexusActionComponent
{
	GENERATED_BODY()

public:
	UGunnerActionComponent();
	virtual void OnSetupActionComponent() override;

	UFUNCTION(Client, Reliable)
	void ClientSendDebugHitConfirmedData(const TArray<FGunnerDebugHitConfirmInfo>& DebugHitConfirmInfos);

private:
	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UGunnerActionSet>> ActionSets;
};
