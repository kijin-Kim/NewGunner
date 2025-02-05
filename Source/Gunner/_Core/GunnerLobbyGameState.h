// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GunnerLobbyGameState.generated.h"


class UGunnerSessionHelperSubsystem;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMapNameChangedSignature, FString, NewMapName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyNameChangedSignature, FString, NewLobbyName);

/**
 * 
 */
UCLASS()
class GUNNER_API AGunnerLobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	UFUNCTION(BlueprintCallable)
	void AuthSetMapName(FString NewMapName);
	UFUNCTION(BlueprintCallable)
	void AuthSetLobbyName(FString NewLobbyName);

private:
	UFUNCTION()
	void OnRep_MapName();
	UFUNCTION()
	void OnRep_LobbyName();

	UGunnerSessionHelperSubsystem* GetSessionHelperSubsystem() const;

public:
	UPROPERTY(BlueprintAssignable)
	FOnMapNameChangedSignature OnMapNameChangedReplicated;
	UPROPERTY(BlueprintAssignable)
	FOnLobbyNameChangedSignature OnLobbyNameChangedReplicated;

private:
	UPROPERTY(ReplicatedUsing=OnRep_MapName, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FString MapName;
	UPROPERTY(ReplicatedUsing=OnRep_LobbyName, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FString LobbyName;
};
