// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GunnerActionAgentInfo.h"
#include "GunnerActionDefinition.h"
#include "Gunner/Core/Input/GunnerEventMessage.h"
#include "UObject/Object.h"
#include "GunnerAction.generated.h"


UENUM()
enum class EGunnerActionNetMethod
{
	LocalOnly UMETA(DisplayName = "Local Only"),
	LocalPredicted UMETA(DisplayName = "Local Predicted"),
	ServerOnly UMETA(DisplayName = "Server Only"),
	ServerAuthoritative UMETA(DisplayName = "Server Authoritative"),
};


class UGunnerAction;
DECLARE_DELEGATE_TwoParams(FOnGunnerActionEndedSignature, FGunnerActionDefinitionHandle, UGunnerAction*);
/**
 * 
 */
UCLASS(Blueprintable)
class GUNNER_API UGunnerAction : public UObject
{
	GENERATED_BODY()

public:
	bool CanTriggerAction() const;
	void TriggerAction(FGunnerActionDefinitionHandle InActionDefinitionHandle, TWeakPtr<FGunnerActionAgentInfo> InAgentInfo, const FGunnerEventMessage& InEventMessage);
	UFUNCTION(BlueprintCallable)
	void EndAction();


	UFUNCTION(BlueprintNativeEvent)
	bool CanTrigger() const;
	UFUNCTION(BlueprintNativeEvent)
	void Trigger(const FGunnerEventMessage& InEventMessage);
	UFUNCTION(BlueprintNativeEvent)
	void End();


	EGunnerActionNetMethod GetActionNetMethod() const { return ActionNetMethod; }
	const FGameplayTagContainer& GetActionTriggerEventTags() const { return ActionTriggerEventTags; }
	const FGameplayTagContainer& GetActionOwnedTags() const { return ActionOwnedTags; }
	const FGameplayTagContainer& GetShouldHaveTags() const { return ShouldHaveTags; }
	const FGameplayTagContainer& GetShouldNotHaveTags() const { return ShouldNotHaveTags; }
	

	UFUNCTION(BlueprintCallable)
	FGunnerActionAgentInfo GetActionAgentInfo() const { return *AgentInfo.Pin(); }

public:
	FOnGunnerActionEndedSignature OnGunnerActionEndedDelegate;

protected:
	FGunnerActionDefinitionHandle ActionDefinitionHandle;
	

	UPROPERTY(EditDefaultsOnly, Category = "ActionTrigger Config")
	EGunnerActionNetMethod ActionNetMethod = EGunnerActionNetMethod::LocalOnly;
	UPROPERTY(EditDefaultsOnly, Category = "ActionTrigger Config")
	FGameplayTagContainer ActionTriggerEventTags;
	UPROPERTY(EditDefaultsOnly, Category = "ActionTrigger Config")
	bool bIsRetriggerable = false;
	

	UPROPERTY(EditDefaultsOnly, Category = "ActionTag")
	FGameplayTagContainer ActionOwnedTags;

	UPROPERTY(EditDefaultsOnly, Category = "ActionTag|TriggerRequirement")
	FGameplayTagContainer ShouldHaveTags;
	UPROPERTY(EditDefaultsOnly, Category = "ActionTag|TriggerRequirement")
	FGameplayTagContainer ShouldNotHaveTags;

	
	TWeakPtr<FGunnerActionAgentInfo> AgentInfo;

	UPROPERTY(BlueprintReadOnly)
	FGunnerEventMessage EventMessage;

private:
	bool bIsTriggering = false;
};
