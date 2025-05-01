// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


#define LOG_LOCALROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetLocalRole()))
#define LOG_LOCALROLEINFO_SUB(LogOwner) *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), LogOwner->GetLocalRole()))

#define LOG_REMOTEROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetRemoteRole()))
#define LOG_REMOTEROLEINFO_SUB(LogOwner) *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), LogOwner->GetRemoteRole()))

#define LOG_NETMODEINFO ((GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"),  static_cast<int32>(GPlayInEditorID)) : ((GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")))
#define LOG_NETMODEINFO_SUB(LogOwner) ((LogOwner->GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), static_cast<int32>(GPlayInEditorID)) : (LogOwner->GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER"))

#define LOG_CALL_INFO ANSI_TO_TCHAR(__FUNCTION__)

#define NX_LOG(CategoryName, Verbosity, Format, ...) UE_LOG(CategoryName, Verbosity, TEXT("[%s][%s/%s](%d) %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, GFrameNumber, *FString::Printf(Format, ##__VA_ARGS__))
#define NX_LOG_FN(CategoryName, Verbosity, Format, ...) UE_LOG(CategoryName, Verbosity, TEXT("[%s][%s/%s](%d) %s %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, GFrameNumber, LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__))
#define NX_CLOG(Condition, CategoryName, Verbosity, Format, ...) UE_CLOG(Condition, CategoryName, Verbosity, TEXT("[%s][%s/%s](%d) %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, GFrameNumber, *FString::Printf(Format, ##__VA_ARGS__))
#define NX_CLOG_FN(Condition, CategoryName, Verbosity, Format, ...) UE_CLOG(Condition, CategoryName, Verbosity, TEXT("[%s][%s/%s](%d) %s %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, GFrameNumber, LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__))

#define NX_LOG_SUB(LogOwner, CategoryName, Verbosity, Format, ...) UE_LOG(CategoryName, Verbosity, TEXT("[%s][%s/%s](%d) %s"), LOG_NETMODEINFO_SUB(Cast<AActor>(LogOwner)), LOG_LOCALROLEINFO_SUB(Cast<AActor>(LogOwner)), LOG_REMOTEROLEINFO_SUB(Cast<AActor>(LogOwner)), GFrameNumber, *FString::Printf(Format, ##__VA_ARGS__))
#define NX_LOG_SUB_FN(LogOwner, CategoryName, Verbosity, Format, ...) UE_LOG(CategoryName, Verbosity, TEXT("[%s][%s/%s](%d) %s %s"), LOG_NETMODEINFO_SUB(Cast<AActor>(LogOwner)), LOG_LOCALROLEINFO_SUB(Cast<AActor>(LogOwner)), LOG_REMOTEROLEINFO_SUB(Cast<AActor>(LogOwner)), GFrameNumber, LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__))
#define NX_CLOG_SUB(LogOwner, Condition, CategoryName, Verbosity, Format, ...) UE_CLOG(Condition, CategoryName, Verbosity, TEXT("[%s][%s/%s](%d) %s"), LOG_NETMODEINFO_SUB(Cast<AActor>(LogOwner)), LOG_LOCALROLEINFO_SUB(Cast<AActor>(LogOwner)), LOG_REMOTEROLEINFO_SUB(Cast<AActor>(LogOwner)), GFrameNumber, *FString::Printf(Format, ##__VA_ARGS__))
#define NX_CLOG_SUB_FN(LogOwner, Condition, CategoryName, Verbosity, Format, ...) UE_CLOG(Condition, CategoryName, Verbosity, TEXT("[%s][%s/%s](%d) %s %s"), LOG_NETMODEINFO_SUB(Cast<AActor>(LogOwner)), LOG_LOCALROLEINFO_SUB(Cast<AActor>(LogOwner)), LOG_REMOTEROLEINFO_SUB(Cast<AActor>(LogOwner)), GFrameNumber, LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__))



#define NX_VLOG_FN(LogOwner, CategoryName, Verbosity, Format, ...) \
{\
	UE_VLOG(LogOwner, CategoryName, Verbosity, TEXT("%s %s"), LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__));\
	NX_LOG_FN(CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}
#define NX_VLOG_SUB_FN(LogOwner, CategoryName, Verbosity, Format, ...) \
{\
	UE_VLOG(LogOwner, CategoryName, Verbosity, TEXT("%s %s"), LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__));\
	NX_LOG_SUB_FN(LogOwner, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}
#define NX_VLOG(LogOwner, CategoryName, Verbosity, Format, ...) \
{\
	UE_VLOG(LogOwner, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
	NX_LOG(CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}
#define NX_VLOG_SUB(LogOwner, CategoryName, Verbosity, Format, ...) \
{\
	UE_VLOG(LogOwner, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
	NX_LOG_SUB(LogOwner, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}


#define NX_CVLOG_FN( Condition, LogOwner, CategoryName, Verbosity, Format, ...) \
{\
	UE_CVLOG(Condition, LogOwner, CategoryName, Verbosity, TEXT("%s %s"), LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__));\
	NX_CLOG_FN(Condition, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}

#define NX_CVLOG_SUB_FN(Condition, LogOwner, CategoryName, Verbosity, Format, ...) \
{\
	UE_CVLOG(Condition, LogOwner, CategoryName, Verbosity, TEXT("%s %s"), LOG_CALL_INFO, *FString::Printf(Format, ##__VA_ARGS__));\
	NX_CLOG_SUB_FN(LogOwner, Condition, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}

#define NX_CVLOG(Condition, LogOwner, CategoryName, Verbosity, Format, ...) \
{\
	UE_CVLOG(Condition, LogOwner, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
	NX_CLOG(Condition, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}

#define NX_CVLOG_SUB(Condition, LogOwner, CategoryName, Verbosity, Format, ...) \
{\
	UE_CVLOG(Condition, LogOwner, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
	NX_CLOG_SUB(LogOwner, Condition, CategoryName, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__));\
}

DECLARE_LOG_CATEGORY_EXTERN(LogNexus, Display, All);

DECLARE_LOG_CATEGORY_EXTERN(LogNexusAction, Display, All);

DECLARE_LOG_CATEGORY_EXTERN(LogNexusProperty, Display, All)

DECLARE_LOG_CATEGORY_EXTERN(LogNexusSideEffect, Display, All);

DECLARE_LOG_CATEGORY_EXTERN(LogNexusCue, Display, All);

DECLARE_LOG_CATEGORY_EXTERN(LogNexusPrediction, Display, All);

DECLARE_LOG_CATEGORY_EXTERN(LogNexusGameplayTag, Display, All);


// 로그 메시지 형식: 컨텍스트: 컨텍스트에 관한 데이터; 추가 데이터
// 컨텍스트는 간결하고 명확하게 문장형이 아닌 절 또는 단어로 작성한다.
// 컨텍스트와 직접적인 연관이 없는 데이터는 추가 데이터에 작성한다.

// check/checkf: 개발 빌드에서 치명적인 오류가 발생했고 진행할 수 없는 경우. 예를 들어 null 포인터 역참조, 잘못된 인덱스 접근 등. 그 외의 빌드에서 표현이 평가되지 않아도 되는 경우.
// Fatal: 빌드 상관없이 실행 환경에 의하여 치명적인 오류가 발생했고 진행할 수 없는 경우. 예를 들어 꼭 필요한 파일이 누락된 경우.
// Error: 치명적인 오류가 발생하였지만 진행할 수 있는 경우. 예를 들어 중요한 변수가 null인 경우 함수 반환 또는 복구를 해도 진행이 가능한 경우.
// Warning: 예상 밖의 오류가 일어났지만 복구를 하지 않아도 진행이 가능한 경우. 예를 들어 특정 함수 중복 호출, 컨테이너에 중복 추가 시도, 예상되는 값과의 불일치 하는 경우.

// 일반적인 게임 이벤트 로그
// Display: 화면에 나타나야 하는 이벤트. 예를 들어 액션의 실행과 종료, 사이드이펙트 시작과 종료 이벤트가 발생한 경우.
// Log: Display의 노이즈를 제거하기 위해 상대적으로 중요하지 않은 이벤트에 사용 한다.

// 개발용 로그
// Verbose: 디버깅용 로그. 주로 개발자에게만 필요한 정보.
// VeryVerbose: Verbose의 노이즈를 제거하기 위해 상대적으로 중요하지 않은 이벤트에 사용 한다

// VisLog: 위치, 시간, 순서와 같은 정보가 필요한 경우에 사용 한다.
