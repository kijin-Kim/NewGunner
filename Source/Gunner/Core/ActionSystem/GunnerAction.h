// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunnerActionDefinition.h"
#include "UObject/Object.h"
#include "GunnerAction.generated.h"


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
	virtual void TriggerAction(FGunnerActionDefinitionHandle InActionDefinitionHandle);
	virtual void EndAction();

	EGunnerActionNetMethod GetActionNetMethod() const { return ActionNetMethod; }

public:
	inline static FOnGunnerActionEndedSignature OnGunnerActionEndedDelegate;

protected:
	FGunnerActionDefinitionHandle ActionDefinitionHandle;
	UPROPERTY(EditDefaultsOnly)
	EGunnerActionNetMethod ActionNetMethod = EGunnerActionNetMethod::ClientOnly;

private:
	bool bIsRunning = false;
};
