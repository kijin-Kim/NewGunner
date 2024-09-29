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
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGunnerActionEndedSignature, FGunnerActionDefinitionHandle, UGunnerAction*);
/**
 * 
 */
UCLASS(Blueprintable)
class GUNNER_API UGunnerAction : public UObject
{
	GENERATED_BODY()

public:
	void InitializeGunnerAction(FGunnerActionDefinitionHandle InActionDefinitionHandle, TWeakPtr<FGunnerActionAgentInfo> InAgentInfo);
	void SetActionCurrentEventMessage(const FGunnerEventMessage& InEventMessage);
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


	EGunnerActionNetMethod GetActionNetMethod() const { return ActionNetMethod; }
	const FGameplayTagContainer& GetActionTriggerEventTags() const { return ActionTriggerEventTags; }
	const FGameplayTagContainer& GetActionOwnedTags() const { return ActionOwnedTags; }
	const FGameplayTagContainer& GetShouldHaveTags() const { return ShouldHaveTags; }
	const FGameplayTagContainer& GetShouldNotHaveTags() const { return ShouldNotHaveTags; }
	bool ShouldTriggerOnAdded() const { return bShouldTriggerOnAdded; }


	UFUNCTION(BlueprintCallable)
	FGunnerActionAgentInfo GetActionAgentInfo() const { return *AgentInfo.Pin(); }

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

public:
	FOnGunnerActionEndedSignature OnGunnerActionEndedDelegate;

protected:
	FGunnerActionDefinitionHandle ActionDefinitionHandle;


	UPROPERTY(EditDefaultsOnly, Category = "Action Trigger Config")
	EGunnerActionNetMethod ActionNetMethod = EGunnerActionNetMethod::LocalOnly;
	UPROPERTY(EditDefaultsOnly, Category = "Action Trigger Config")
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


	TWeakPtr<FGunnerActionAgentInfo> AgentInfo;

	UPROPERTY(BlueprintReadOnly)
	FGunnerEventMessage EventMessage;

private:
	bool bIsTriggering = false;
};
