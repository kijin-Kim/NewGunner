// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NexusActionDef.h"
#include "NexusActionDefHandle.h"
#include "NexusAgentInfo.h"
#include "Prediction/NexusPrediction.h"
#include "Event/NexusEventMessage.h"
#include "UObject/Object.h"
#include "NexusAction.generated.h"


class INexusActionDefInterface;
class UNexusAction;
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnNexusActionEndedSignature, const FNexusActionDefHandle&, UNexusAction*);

UENUM()
enum class ENexusActionNetMethod : uint8
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
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	static UNexusAction* NewNexusActionObject(UClass* Class, const FNexusActionDefHandle& InActionDefHandle, TWeakPtr<FNexusAgentInfo> InAgentInfo, TWeakObjectPtr<UObject> InSourceObject);
	virtual UWorld* GetWorld() const override;
	void SetActionCurrentEventMessage(const FNexusEventMessage& InEventMessage);

	void CallOnAddAction();
	bool CallOnCanTriggerAction() const;
	void CallOnTriggerAction();
	void CallOnConfirmAction();
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "End Action"))
	void EndAction();
	void CallOnRemoveAction();


	ENexusActionNetMethod GetActionNetMethod() const { return ActionNetMethod; }
	const FGameplayTagContainer& GetActionTriggerEventTags() const { return ActionTriggerEventTags; }
	const FGameplayTagContainer& GetActionOwnedTags() const { return ActionOwnedTags; }
	const FGameplayTagContainer& GetActionCancelTags() const { return ActionCancelTags; }
	const FGameplayTagContainer& GetShouldHaveTags() const { return ShouldHaveTags; }
	const FGameplayTagContainer& GetShouldNotHaveTags() const { return ShouldNotHaveTags; }

	void SetPrimaryPredictionTag(FNexusPredictionTag InPredictionTag) { PrimaryPredictionTag = InPredictionTag; }
	FNexusPredictionTag GetPrimaryPredictionTag() const { return PrimaryPredictionTag; }
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

	template <typename T>
	T* GetSourceObject() const
	{
		return Cast<T>(SourceObject.Get());
	}

	UFUNCTION(BlueprintPure, meta = (DeterminesOutputType = "Class"))
	UObject* GetSourceObject(TSubclassOf<UObject> Class) const
	{
		return SourceObject.Get();
	}

	UObject* GetSourceObject() const { return SourceObject.Get(); }

	FNexusActionDefHandle GetActionDefHandle() const { return ActionDefHandle; }

	bool IsRemoteTriggerable() const { return bAllowRemoteTrigger; }

protected:

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Add Action"))
	void BP_OnAddAction();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Can Trigger Action"))
	bool BP_OnCanTriggerAction() const;
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Trigger Action"))
	void BP_OnTriggerAction();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Confirm Action"))
	void BP_OnConfirmAction();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On End Action"))
	void BP_OnEndAction();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Remove Action"))
	void BP_OnRemoveAction();


	virtual void OnAddAction();
	virtual bool OnCanTriggerAction() const;
	virtual void OnTriggerAction();
	virtual void OnConfirmAction();
	virtual void OnEndAction();
	virtual void OnRemoveAction();

protected:
	void InitializeAction(const FNexusActionDefHandle& InActionDefHandle, TWeakPtr<FNexusAgentInfo> InAgentInfo, TWeakObjectPtr<UObject> InSourceObject);

public:
	FOnNexusActionEndedSignature OnActionEndedDelegate;

protected:
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


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionTag")
	FGameplayTagContainer ActionOwnedTags;
	// 이 태그를 소유한 액션은 이 액션이 실행될 때 취소됩니다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionTag")
	FGameplayTagContainer ActionCancelTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionTag|Requirement")
	FGameplayTagContainer ShouldHaveTags;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionTag|Requirement")
	FGameplayTagContainer ShouldNotHaveTags;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UObject> SourceObject;

	UPROPERTY(BlueprintReadOnly)
	FNexusEventMessage EventMessage;

private:
	FNexusActionDefHandle ActionDefHandle;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsTriggering = false;
	FNexusPredictionTag PrimaryPredictionTag;
	TWeakPtr<FNexusAgentInfo> AgentInfo;
	
};
