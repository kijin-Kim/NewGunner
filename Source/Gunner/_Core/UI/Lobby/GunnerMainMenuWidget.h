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

	UFUNCTION()
	void OnJoinButtonClicked();
	UFUNCTION()
	void OnHostButtonClicked();
	UFUNCTION()
	void OnDeveloperToolButtonClicked();
	UFUNCTION()
	void OnShutdownButtonClicked();

	UFUNCTION()
	void OnLoopbackJoinButtonClicked();
	UFUNCTION()
	void OnLoopbackHostButtonClicked();
	UFUNCTION()
	void OnBackwardButtonClicked();
	
	UFUNCTION()
	void OnQuitMenuButtonClicked();

private:
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* WidgetSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	UGunnerButtonWidget* JoinButton;
	UPROPERTY(meta = (BindWidget))
	UGunnerButtonWidget* HostButton;
	UPROPERTY(meta = (BindWidget))
	UGunnerButtonWidget* DeveloperToolButton;
	UPROPERTY(meta = (BindWidget))
	UGunnerButtonWidget* ShutdownButton;
	
	UPROPERTY(meta = (BindWidget))
	UGunnerButtonWidget* LoopbackJoinButton;
	UPROPERTY(meta = (BindWidget))
	UGunnerButtonWidget* LoopbackHostButton;
	UPROPERTY(meta = (BindWidget))
	UGunnerButtonWidget* BackwardButton;
	
	UPROPERTY(meta = (BindWidget))
	UGunnerButtonWidget* QuitMenuButton;
	uint32 LastWidgetIndex = 0;

	
	IOnlineSessionPtr SessionInterfacePtr;

	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
};
