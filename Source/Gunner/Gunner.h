// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NexusLog.h"

#define GR_LOG(CategoryName, Verbosity, Format, ...) UE_LOG(CategoryName, Verbosity, TEXT("[%s][%s/%s] %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define GR_LOG_FN(CategoryName, Verbosity, Format, ...) UE_LOG(CategoryName, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__))
#define GR_CLOG(Condition, CategoryName, Verbosity, Format, ...) UE_CLOG(Condition, CategoryName, Verbosity, TEXT("[%s][%s/%s] %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define GR_CLOG_FN(Condition, CategoryName, Verbosity, Format, ...) UE_CLOG(Condition, CategoryName, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__))

#define GR_LOG_SUB(LogOwner, CategoryName, Verbosity, Format, ...) UE_LOG(CategoryName, Verbosity, TEXT("[%s][%s/%s] %s"), LOG_NETMODEINFO_SUB(Cast<AActor>(LogOwner)), LOG_LOCALROLEINFO_SUB(Cast<AActor>(LogOwner)), LOG_REMOTEROLEINFO_SUB(Cast<AActor>(LogOwner)), *FString::Printf(Format, ##__VA_ARGS__))
#define GR_LOG_SUB_FN(LogOwner, CategoryName, Verbosity, Format, ...) UE_LOG(CategoryName, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_NETMODEINFO_SUB(Cast<AActor>(LogOwner)), LOG_LOCALROLEINFO_SUB(Cast<AActor>(LogOwner)), LOG_REMOTEROLEINFO_SUB(Cast<AActor>(LogOwner)), LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__))
#define GR_CLOG_SUB(LogOwner, Condition, CategoryName, Verbosity, Format, ...) UE_CLOG(Condition, CategoryName, Verbosity, TEXT("[%s][%s/%s] %s"), LOG_NETMODEINFO_SUB(Cast<AActor>(LogOwner)), LOG_LOCALROLEINFO_SUB(Cast<AActor>(LogOwner)), LOG_REMOTEROLEINFO_SUB(Cast<AActor>(LogOwner)), *FString::Printf(Format, ##__VA_ARGS__))
#define GR_CLOG_SUB_FN(LogOwner, Condition, CategoryName, Verbosity, Format, ...) UE_CLOG(Condition, CategoryName, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_NETMODEINFO_SUB(Cast<AActor>(LogOwner)), LOG_LOCALROLEINFO_SUB(Cast<AActor>(LogOwner)), LOG_REMOTEROLEINFO_SUB(Cast<AActor>(LogOwner)), LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__))



#define GR_VLOG_FN(LogOwner, CategoryName, Verbosity, Format, ...) \
{\
	UE_VLOG(LogOwner, CategoryName, Verbosity, TEXT("%s %s"), LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__));\
	GR_LOG_FN(CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}
#define GR_VLOG_SUB_FN(LogOwner, CategoryName, Verbosity, Format, ...) \
{\
	UE_VLOG(LogOwner, CategoryName, Verbosity, TEXT("%s %s"), LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__));\
	GR_LOG_SUB_FN(LogOwner, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}
#define GR_VLOG(LogOwner, CategoryName, Verbosity, Format, ...) \
{\
	UE_VLOG(LogOwner, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
	GR_LOG(CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}
#define GR_VLOG_SUB(LogOwner, CategoryName, Verbosity, Format, ...) \
{\
	UE_VLOG(LogOwner, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
	GR_LOG_SUB(LogOwner, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}


#define GR_CVLOG_FN( Condition, LogOwner, CategoryName, Verbosity, Format, ...) \
{\
	UE_CVLOG(Condition, LogOwner, CategoryName, Verbosity, TEXT("%s %s"), LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__));\
	GR_CLOG_FN(Condition, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}

#define GR_CVLOG_SUB_FN(Condition, LogOwner, CategoryName, Verbosity, Format, ...) \
{\
	UE_CVLOG(Condition, LogOwner, CategoryName, Verbosity, TEXT("%s %s"), LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__));\
	GR_CLOG_SUB_FN(LogOwner, Condition, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}

#define GR_CVLOG(Condition, LogOwner, CategoryName, Verbosity, Format, ...) \
{\
	UE_CVLOG(Condition, LogOwner, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
	GR_CLOG(Condition, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}

#define GR_CVLOG_SUB(Condition, LogOwner, CategoryName, Verbosity, Format, ...) \
{\
	UE_CVLOG(Condition, LogOwner, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
	GR_CLOG_SUB(LogOwner, Condition, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}


DECLARE_LOG_CATEGORY_EXTERN(LogGunner, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGunnerInventory, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(LogGunnerSession, Display, All);
