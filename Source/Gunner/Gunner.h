// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NexusLog.h"

#define GR_LOG_FN(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__))
#define GR_LOG_SUB_FN(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_NETMODEINFO_SUB, LOG_LOCALROLEINFO_SUB, LOG_REMOTEROLEINFO_SUB, LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__))

#define GR_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define GR_LOG_SUB(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s"), LOG_NETMODEINFO_SUB, LOG_LOCALROLEINFO_SUB, LOG_REMOTEROLEINFO_SUB, *FString::Printf(Format, ##__VA_ARGS__))

#define GR_VLOG_FN(LogOwner, LogCat, Verbosity, Format, ...) \
{\
	UE_VLOG(LogOwner, LogCat, Verbosity, TEXT("%s %s"), LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__));\
	GR_LOG_FN(LogCat, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}
#define GR_VLOG_SUB_FN(LogOwner, LogCat, Verbosity, Format, ...) \
{\
	UE_VLOG(LogOwner, LogCat, Verbosity, TEXT("%s %s"), LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__));\
	GR_LOG_SUB_FN(LogCat, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}
#define GR_VLOG(LogOwner, LogCat, Verbosity, Format, ...) \
{\
	UE_VLOG(LogOwner, LogCat, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
	GR_LOG(LogCat, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}
#define GR_VLOG_SUB(LogOwner, LogCat, Verbosity, Format, ...) \
{\
	UE_VLOG(LogOwner, LogCat, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
	GR_LOG_SUB(LogCat, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}


DECLARE_LOG_CATEGORY_EXTERN(LogGunner, Log, All);
