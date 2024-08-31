// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerStateMachineTransitionData.h"
#include "Components/ActorComponent.h"
#include "GunnerStateMachineComponent.generated.h"

class UGunnerStateComponent;

USTRUCT()
struct FGunnerStateMachineTransitionRule
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UGunnerStateComponent> ToStateComponent;
	UPROPERTY()
	TObjectPtr<UGunnerStateMachineTransitionCondition> TransitionCondition;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerStateMachineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunnerStateMachineComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:
	void RegisterStateComponentFromOwner();
	void RegisterTransitionData();
	void CheckTransitions();
	
	UGunnerStateComponent* GetStateComponentByClass(TSubclassOf<UGunnerStateComponent> StateComponentClass) const;

	
private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGunnerStateComponent> StartStateComponentClass;
	UPROPERTY()
	TObjectPtr<UGunnerStateComponent> CurrentStateComponent;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UGunnerStateMachineTransitionData> TransitionData;

	
	TMap<TObjectPtr<UGunnerStateComponent>, TArray<FGunnerStateMachineTransitionRule>> StateTransitionMap;
	

	UPROPERTY()
	TArray<TObjectPtr<UGunnerStateComponent>> OwnerStateComponents;
	
};
