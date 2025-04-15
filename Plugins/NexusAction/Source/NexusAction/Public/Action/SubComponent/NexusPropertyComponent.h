// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NexusAgentBoundComponent.h"
#include "Components/ActorComponent.h"
#include "SideEffect/NexusProperty.h"
#include "NexusPropertyComponent.generated.h"


class UNexusProperty;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEXUSACTION_API UNexusPropertyComponent : public UNexusAgentBoundComponent
{
	GENERATED_BODY()
	
public:
	UNexusPropertyComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	 
	void AddProperty(FGameplayTag Tag, float Value);
	void AuthRemoveAllProperties();


	const TArray<TObjectPtr<UNexusProperty>>& GetProperties() const { return Properties; }
	UNexusProperty* GetProperty(FGameplayTag Tag);
	void AddStaticOperation(FGameplayTag Tag, FNexusPropertyOperation Operation);
	void AddDynamicOperation(FGameplayTag Tag, FNexusPropertyOperation Operation);
	void RemoveOperationByHandle(FGameplayTag Tag, FNexusPropertyOperationHandle OperationHandle);

private:
	// 서버와 클라이언트 양쪽에서 프로퍼티를 추가합니다. 서버와 클라이언트에서의 프로퍼티 등장순서가 같아야 합니다.	
	UPROPERTY(Replicated)
	TArray<TObjectPtr<UNexusProperty>> Properties;
	
};
