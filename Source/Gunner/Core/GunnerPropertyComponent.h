// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GunnerActionProperty.h"
#include "ActionSystem/GunnerActionSideEffectDefinition.h"
#include "Components/ActorComponent.h"
#include "GunnerPropertyComponent.generated.h"



USTRUCT()
struct FGunnerActionPropertyMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTag Tag;
	UPROPERTY(EditAnywhere)
	float Value = 0.0f;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GUNNER_API UGunnerPropertyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGunnerPropertyComponent();
	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);
	virtual void InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void AddProperty(FGameplayTag Tag, float Value);
	UGunnerActionProperty* GetProperty(FGameplayTag Tag);
	void OnSideEffectFailed(FGunnerActionSideEffectDefinitionHandle SideEffectDefinitionHandle);
	void OnSideEffectEnded(FGunnerActionSideEffectDefinitionHandle SideEffectDefinitionHandle);

	

private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TArray<FGunnerActionPropertyMapping> StartProperties;
	
	UPROPERTY(Replicated)
	TArray<TObjectPtr<UGunnerActionProperty>> Properties;
};
