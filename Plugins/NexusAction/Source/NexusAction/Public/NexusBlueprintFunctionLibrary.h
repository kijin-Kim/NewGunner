// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SideEffect/NexusSideEffect.h"
#include "SideEffect/NexusSideEffectInstance.h"
#include "NexusBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class NEXUSACTION_API UNexusBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Nexus|SideEffect")
	static void SetInjectedValue(const FNexusSideEffectInstanceDefHandle& Handle, FGameplayTag Tag, float Value)
	{
		check(Handle.IsValid());
		Handle.GetData()->InjectedValues.Add(FNexusInjectedValuePair{Tag, Value});
	}

	UFUNCTION(BlueprintCallable, Category = "Nexus|SideEffect")
	static void AddDynamicTagModifier(const FNexusSideEffectInstanceDefHandle& Handle, const FNexusGameplayTagMod& TagMod)
	{
		check(Handle.IsValid());
		Handle.GetData()->DynamicTagModifiers.Add(TagMod);
	}
};
