// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


#define LOG_LOCALROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetLocalRole()))
#define LOG_LOCALROLEINFO_SUB *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), Cast<AActor>(GetOuter())->GetLocalRole()))

#define LOG_REMOTEROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetRemoteRole()))
#define LOG_REMOTEROLEINFO_SUB *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), Cast<AActor>(GetOuter())->GetRemoteRole()))

#define LOG_NETMODEINFO ((GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"),  static_cast<int32>(GPlayInEditorID)) : ((GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")))
#define LOG_NETMODEINFO_SUB ((Cast<AActor>(GetOuter())->GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), static_cast<int32>(GPlayInEditorID)) : ((Cast<AActor>(GetOuter())->GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")))

#define LOG_CALL_INFO ANSI_TO_TCHAR(__FUNCTION__)
#define NX_LOG_FN(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__))
#define NX_LOG_SUB_FN(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_NETMODEINFO_SUB, LOG_LOCALROLEINFO_SUB, LOG_REMOTEROLEINFO_SUB, LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__))

#define NX_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define NX_LOG_SUB(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s"), LOG_NETMODEINFO_SUB, LOG_LOCALROLEINFO_SUB, LOG_REMOTEROLEINFO_SUB, *FString::Printf(Format, ##__VA_ARGS__))


DECLARE_LOG_CATEGORY_EXTERN(LogNexus, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogNexusAction, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogNexusProperty, Log, All)
DECLARE_LOG_CATEGORY_EXTERN(LogNexusSideEffect, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogNexusCue, Log, All);