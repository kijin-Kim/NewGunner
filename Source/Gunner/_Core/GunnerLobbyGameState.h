// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GunnerLobbyGameState.generated.h"


class UGunnerSessionHelperSubsystem;

USTRUCT(BlueprintType)
struct FTeamBoxSlot
{
	GENERATED_BODY()

	FTeamBoxSlot()
	{
		Reset();
	}

	FTeamBoxSlot(const FString& InNickName, int32 InPingInMs)
		: NickName(InNickName)
		  , PingInMs(InPingInMs)
		  , UniqueNetId(FUniqueNetIdRepl())
	{
	}

	bool operator==(const FTeamBoxSlot& Other) const
	{
		return UniqueNetId == Other.UniqueNetId;
	}

	bool IsValid() const
	{
		return NickName != TEXT("None") && PingInMs != -1;
	}

	void Reset()
	{
		NickName = TEXT("None");
		PingInMs = -1;
		UniqueNetId = FUniqueNetIdRepl();
	}

	UPROPERTY(BlueprintReadOnly)
	FString NickName = TEXT("None");

	UPROPERTY(BlueprintReadOnly)
	int32 PingInMs = -1;

	UPROPERTY(BlueprintReadOnly)
	FUniqueNetIdRepl UniqueNetId;
};


USTRUCT(BlueprintType)
struct FNormalizedTeamBoxSlots
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TArray<FTeamBoxSlot> LeftTeamSlots;
	UPROPERTY(BlueprintReadWrite)
	TArray<FTeamBoxSlot> RightTeamSlots;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTeamBoxSlotsDirtySignautre);

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
	AGunnerLobbyGameState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	UFUNCTION(BlueprintCallable)
	void AuthSetMapName(FString NewMapName);
	UFUNCTION(BlueprintCallable)
	void AuthSetLobbyName(FString NewLobbyName);

	void AuthOnPlayerJoinedLobby(APlayerState* PlayerState);
	void AuthOnPlayerLeftLobby(APlayerState* PlayerState);
	void AuthChangeTeamBoxSlot(APlayerState* PlayerState);

	UFUNCTION(BlueprintCallable)
	FNormalizedTeamBoxSlots GetNormalizedTeamBoxSlots() const;

private:
	UFUNCTION()
	void OnRep_MapName();
	UFUNCTION()
	void OnRep_LobbyName();
	UFUNCTION()
	void OnRep_TeamBoxSlots();
	


	UGunnerSessionHelperSubsystem* GetSessionHelperSubsystem() const;
	int32 GetBalancedTeamBoxSlotIndex() const;
	void RearrangeTeamBoxSlots();

public:
	UPROPERTY(BlueprintAssignable)
	FOnMapNameChangedSignature OnMapNameChangedReplicated;
	UPROPERTY(BlueprintAssignable)
	FOnLobbyNameChangedSignature OnLobbyNameChangedReplicated;
	UPROPERTY(BlueprintAssignable)
	FOnTeamBoxSlotsDirtySignautre OnTeamBoxSlotsDirty;

private:
	UPROPERTY(ReplicatedUsing=OnRep_TeamBoxSlots, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TArray<FTeamBoxSlot> TeamBoxSlots;


	UPROPERTY(ReplicatedUsing=OnRep_MapName, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FString MapName;
	UPROPERTY(ReplicatedUsing=OnRep_LobbyName, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FString LobbyName;
};
