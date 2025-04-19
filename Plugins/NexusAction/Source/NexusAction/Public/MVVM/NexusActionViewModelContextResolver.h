// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "View/MVVMViewModelContextResolver.h"

#include "NexusActionViewModelContextResolver.generated.h"

class UNexusActionComponent;
/**
 * 
 */
UCLASS()
class NEXUSACTION_API UNexusActionViewModelContextResolver : public UMVVMViewModelContextResolver
{
	GENERATED_BODY()

public:
	UNexusActionComponent* GetActionComponent(const UUserWidget* UserWidget) const;
};
