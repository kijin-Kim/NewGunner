// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "SideEffect/NexusProperty.h"
#include "NexusPropertyComponent.generated.h"


class UNexusProperty;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEXUSACTION_API UNexusPropertyComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UNexusPropertyComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void AuthAddProperty(FGameplayTag Tag, float Value);
	void AuthRemoveAllProperties();


	const TArray<TObjectPtr<UNexusProperty>>& GetProperties() const { return Properties; }
	UNexusProperty* GetProperty(FGameplayTag Tag);
	void AddStaticOperation(FGameplayTag Tag, FNexusPropertyOperation Operation);
	void AddDynamicOperation(FGameplayTag Tag, FNexusPropertyOperation Operation);
	void RemoveOperationByHandle(FGameplayTag Tag, FNexusPropertyOperationHandle OperationHandle);

private:
	UPROPERTY(Replicated)
	TArray<TObjectPtr<UNexusProperty>> Properties;
	
};
