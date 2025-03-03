// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NexusActionDefHandle.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "NexusActionDef.generated.h"

/**
 * 
 */

struct FNexusActionDef;
class UNexusAction;

DECLARE_DELEGATE_OneParam(FOnActionDefAddedSignature, FNexusActionDef& /*ActionDef*/);
DECLARE_DELEGATE_OneParam(FOnActionDefRemovedSignature, FNexusActionDef& /*ActionDef*/);


struct FNexusActionDefContainer;

USTRUCT(BlueprintType)
struct NEXUSACTION_API FNexusActionDef : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FNexusActionDef();
	FNexusActionDef(UObject* InSourceObject, TSubclassOf<UNexusAction> InActionClass);
	bool operator==(const FNexusActionDef& Other) const;
	bool operator!=(const FNexusActionDef& Other) const;

	FString ToString() const;

	void PostReplicatedAdd(const FNexusActionDefContainer& InArraySerializer);
	void PreReplicatedRemove(const FNexusActionDefContainer& InArraySerializer);

	// 액션의 고유 핸들. 서버와 클라이언트 사이에서도 유일한 값으로 사용됨.
	UPROPERTY()
	FNexusActionDefHandle Handle;

	// 액션의 바리에이션을 위한 데이터 오브젝트. 예를 들어 각 무기별 사격 행동에 대한 애니메이션
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> SourceObject;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UNexusAction> ActionClass;
	
	// 액션이 추가될 시 로컬에서 각 에이전트마다 생성되는 액션 인스턴스
	UPROPERTY(NotReplicated)
	TObjectPtr<UNexusAction> ActionInstance;

	// 액션 실행시 소유하는 태그들
	FGameplayTagContainer OwnedTags;
	
};

USTRUCT()
struct NEXUSACTION_API FNexusActionDefContainer : public FFastArraySerializer
{
	GENERATED_BODY()


	void AuthAdd(const FNexusActionDef& ActionDef);
	void AuthRemove(const FNexusActionDefHandle& Handle);
	void AuthRemoveAll();
	FNexusActionDef* FindActionDefByHandle(FNexusActionDefHandle Handle);
	bool HasSameActionClassAndSourceObject(const FNexusActionDef& ActionDef) const;
	FNexusActionDefHandle FindActionDefHandle(TSubclassOf<UNexusAction> ActionClass, UObject* SourceObject) const;
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);
	void OnAdded(FNexusActionDef& ActionDef) const;
	void OnRemoved(FNexusActionDef& ActionDef) const;


	UPROPERTY()
	TArray<FNexusActionDef> Items;
	FOnActionDefAddedSignature OnActionDefAddedDelegate;
	FOnActionDefRemovedSignature OnActionDefRemovedDelegate;
};


template <>
struct TStructOpsTypeTraits<FNexusActionDefContainer> : public TStructOpsTypeTraitsBase2<FNexusActionDefContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
