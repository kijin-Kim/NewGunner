// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NexusAgentInfo.h"
#include "NexusActionDef.h"
#include "NexusPrediction.h"
#include "Event/NexusEventMessage.h"
#include "UObject/Object.h"
#include "NexusAction.generated.h"


class UNexusAction;
DECLARE_MULTICAST_DELEGATE_TwoParams(OnNexusActionEndedSignature, FNexusActionDefHandle, UNexusAction*);

UENUM()
enum class ENexusActionNetMethod
{
	LocalOnly UMETA(DisplayName = "Local Only"),
	LocalPredicted UMETA(DisplayName = "Local Predicted"),
	ServerOnly UMETA(DisplayName = "Server Only"),
	ServerAuthoritative UMETA(DisplayName = "Server Authoritative"),
};

/**
 * 
 */
UCLASS(Blueprintable)
class NEXUSACTION_API UNexusAction : public UObject
{
	GENERATED_BODY()

public:
	//~ Begin UObject Interface.
	virtual UWorld* GetWorld() const override;
	//~ End UObject Interface.

	void InitializeAction(FNexusActionDefHandle InActionDefinitionHandle, TWeakPtr<FNexusAgentInfo> InAgentInfo);
	void SetActionCurrentEventMessage(const FNexusEventMessage& InEventMessage);
	UFUNCTION(BlueprintNativeEvent)
	void OnActionAdded();
	UFUNCTION(BlueprintNativeEvent)
	bool OnCanTriggerAction() const;
	UFUNCTION(BlueprintNativeEvent)
	void OnTriggerAction();
	UFUNCTION(BlueprintNativeEvent)
	void OnEndAction();

	UFUNCTION(BlueprintCallable)
	void EndAction();

	ENexusActionNetMethod GetActionNetMethod() const { return ActionNetMethod; }
	const FGameplayTagContainer& GetActionTriggerEventTags() const { return ActionTriggerEventTags; }
	const FGameplayTagContainer& GetActionOwnedTags() const { return ActionOwnedTags; }
	const FGameplayTagContainer& GetShouldHaveTags() const { return ShouldHaveTags; }
	const FGameplayTagContainer& GetShouldNotHaveTags() const { return ShouldNotHaveTags; }
	bool ShouldTriggerOnAdded() const { return bShouldTriggerOnAdded; }
	bool IsTriggering() const { return bIsTriggering; }
	bool IsRetriggerable() const { return bIsRetriggerable; }

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsLocallyPlayerControlled() const { return AgentInfo.IsValid() && AgentInfo.Pin()->IsLocallyPlayerControlled(); }

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsLocallyControlled() const { return AgentInfo.IsValid() && AgentInfo.Pin()->IsLocallyControlled(); }

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsOwnerActorAuthoritative() const { return AgentInfo.IsValid() && AgentInfo.Pin()->IsOwnerActorAuthoritative(); }

	UFUNCTION(BlueprintCallable)
	AActor* GetOwnerActor() const { return AgentInfo.IsValid() ? AgentInfo.Pin()->OwnerActor.Get() : nullptr; }

	UFUNCTION(BlueprintCallable)
	AActor* GetAgentActor() const { return AgentInfo.IsValid() ? AgentInfo.Pin()->AgentActor.Get() : nullptr; }

	UFUNCTION(BlueprintCallable)
	AController* GetController() const { return AgentInfo.IsValid() ? AgentInfo.Pin()->Controller.Get() : nullptr; }

	FNexusActionDefHandle GetActionDefinitionHandle() const { return ActionDefinitionHandle; }
	bool IsRemoteTriggerable() const { return bAllowRemoteTrigger; }

public:
	FNexusPredictionTag InitPredictionHandle;
	OnNexusActionEndedSignature OnActionEndedDelegate;

protected:
	FNexusActionDefHandle ActionDefinitionHandle;


	UPROPERTY(EditDefaultsOnly, Category = "Action Trigger Config")
	ENexusActionNetMethod ActionNetMethod = ENexusActionNetMethod::LocalOnly;
	UPROPERTY(EditDefaultsOnly, Category = "Action Trigger Config")
	bool bAllowRemoteTrigger = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action Trigger Config")
	FGameplayTagContainer ActionTriggerEventTags;
	UPROPERTY(EditDefaultsOnly, Category = "Action Trigger Config")
	bool bIsRetriggerable = false;
	UPROPERTY(EditDefaultsOnly, Category = "Action Trigger Config")
	bool bShouldTriggerOnAdded = false;


	UPROPERTY(EditDefaultsOnly, Category = "ActionTag")
	FGameplayTagContainer ActionOwnedTags;

	UPROPERTY(EditDefaultsOnly, Category = "ActionTag|Requirement")
	FGameplayTagContainer ShouldHaveTags;
	UPROPERTY(EditDefaultsOnly, Category = "ActionTag|Requirement")
	FGameplayTagContainer ShouldNotHaveTags;


	TWeakPtr<FNexusAgentInfo> AgentInfo;

	UPROPERTY(BlueprintReadOnly)
	FNexusEventMessage EventMessage;

private:
	bool bIsTriggering = false;
};
