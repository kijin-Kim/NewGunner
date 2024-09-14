// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define LOG_LOCALROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetLocalRole()))
#define LOG_LOCALROLEINFO_SUB *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), Cast<AActor>(GetOuter())->GetLocalRole()))

#define LOG_REMOTEROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetRemoteRole()))
#define LOG_REMOTEROLEINFO_SUB *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), Cast<AActor>(GetOuter())->GetRemoteRole()))

#define LOG_NETMODEINFO ((GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GPlayInEditorID) : ((GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")))
#define LOG_NETMODEINFO_SUB ((Cast<AActor>(GetOuter())->GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GPlayInEditorID) : ((Cast<AActor>(GetOuter())->GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")))

#define LOG_CALL_INFO ANSI_TO_TCHAR(__FUNCTION__)
#define GR_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__))
#define GR_LOG_SUB(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_NETMODEINFO_SUB, LOG_LOCALROLEINFO_SUB, LOG_REMOTEROLEINFO_SUB, LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__))

DECLARE_LOG_CATEGORY_EXTERN(LogGunner, Log, All);


