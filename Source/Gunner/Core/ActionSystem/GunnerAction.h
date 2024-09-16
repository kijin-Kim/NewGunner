// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GunnerActionDefinition.h"
#include "UObject/Object.h"
#include "GunnerAction.generated.h"


struct FGunnerActionAgentInfo;

UENUM()
enum class EGunnerActionNetMethod
{
	ClientOnly UMETA(DisplayName = "Client Only"),
	ClientPredicted UMETA(DisplayName = "Client Predicted"),
	ServerOnly UMETA(DisplayName = "Server Only"),
	ServerAuthoritative UMETA(DisplayName = "Server Authoritative"),
};


class UGunnerAction;
DECLARE_DELEGATE_TwoParams(FOnGunnerActionEndedSignature, FGunnerActionDefinitionHandle, UGunnerAction*);
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerAction : public UObject
{
	GENERATED_BODY()

public:
	virtual bool CanTriggerAction();
	virtual void TriggerAction(FGunnerActionDefinitionHandle InActionDefinitionHandle, TWeakPtr<FGunnerActionAgentInfo> InAgentInfo);
	virtual void EndAction();

	EGunnerActionNetMethod GetActionNetMethod() const { return ActionNetMethod; }
	const FGameplayTagContainer& GetTriggerEvents() const { return TriggerEvents; }

public:
	inline static FOnGunnerActionEndedSignature OnGunnerActionEndedDelegate;

protected:
	FGunnerActionDefinitionHandle ActionDefinitionHandle;
	UPROPERTY(EditDefaultsOnly)
	EGunnerActionNetMethod ActionNetMethod = EGunnerActionNetMethod::ClientOnly;
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer TriggerEvents;
	TWeakPtr<FGunnerActionAgentInfo> AgentInfo;
private:
	bool bIsRunning = false;
	
};
