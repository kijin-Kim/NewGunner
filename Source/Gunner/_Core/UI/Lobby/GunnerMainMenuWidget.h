// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "Gunner/_Core/UI/GunnerUserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GunnerMainMenuWidget.generated.h"

class UWidgetSwitcher;
class UGunnerButtonWidget;
class FOnlineSessionSearch;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerMainMenuWidget : public UGunnerUserWidget
{
	GENERATED_BODY()

public:
	
	virtual void NativeConstruct() override;

private:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName Name, EOnJoinSessionCompleteResult::Type Arg);

	UFUNCTION(BlueprintCallable)
	void OnJoinButtonClicked();
	UFUNCTION(BlueprintCallable)
	void OnHostButtonClicked();
	UFUNCTION(BlueprintCallable)
	void OnShutdownButtonClicked();

private:
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	
	IOnlineSessionPtr SessionInterfacePtr;

	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
};
