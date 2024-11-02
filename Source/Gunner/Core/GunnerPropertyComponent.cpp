// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerPropertyComponent.h"

#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/Gunner.h"
#include "Net/UnrealNetwork.h"


UGunnerPropertyComponent::UGunnerPropertyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		AHUD::OnShowDebugInfo.AddStatic(&ThisClass::OnShowDebugInfo);
	}
	bReplicateUsingRegisteredSubObjectList = true;
}

void UGunnerPropertyComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	AActor* DebugTarget = HUD->GetCurrentDebugTargetActor();
	if (!DebugTarget)
	{
		return;
	}
	APlayerState* PS =  Cast<APawn>(DebugTarget)->GetPlayerState();
	if (UGunnerPropertyComponent* PropertyComponent = PS->GetComponentByClass<UGunnerPropertyComponent>())
	{
		PropertyComponent->InternalOnShowDebugInfo(DebugTarget, HUD, Canvas, DebugDisplayInfo, X, Y);
	}
}

void UGunnerPropertyComponent::InternalOnShowDebugInfo(AActor* DebugTarget, AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& X, float& Y)
{
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	DisplayDebugManager.SetDrawColor(FColor::White);
	static float LastValue = 0.0f;
	for (UGunnerActionProperty* Property : Properties)
	{
		if(LastValue < Property->Value)
		{
			UE_LOG(LogGunner, Warning, TEXT("LastValue: %f, CurrentValue: %f, Time: %f"), LastValue, Property->Value, GetWorld()->GetTimeSeconds());
			UE_DEBUG_BREAK();
		}
		DisplayDebugManager.DrawString(FString::Printf(TEXT("%s: %f"), *Property->Tag.ToString(), Property->Value));
		LastValue = Property->Value;
	}
}

void UGunnerPropertyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGunnerPropertyComponent, Properties);

}

void UGunnerPropertyComponent::OnRegister()
{
	Super::OnRegister();
	if (GetOwner()->HasAuthority())
	{
		for (const auto& [Tag, Value] : StartProperties)
		{
			AddProperty(Tag, Value);
		}
	}
}

void UGunnerPropertyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UGunnerPropertyComponent::AddProperty(FGameplayTag Tag, float Value)
{
	UGunnerActionProperty* NewProperty = Properties[Properties.Add(NewObject<UGunnerActionProperty>(GetOwner()))];
	NewProperty->Tag = Tag;
	AddReplicatedSubObject(NewProperty);
}

UGunnerActionProperty* UGunnerPropertyComponent::GetProperty(FGameplayTag Tag)
{
	for (UGunnerActionProperty* Property : Properties)
	{
		if (Property->Tag == Tag)
		{
			return Property;
		}
	}
	return nullptr;
}

void UGunnerPropertyComponent::OnSideEffectFailed(FGunnerActionSideEffectDefinitionHandle SideEffectDefinitionHandle)
{
	for (UGunnerActionProperty* Property : Properties)
	{
		Property->Operations.RemoveAll([SideEffectDefinitionHandle](const FGunnerActionPropertyOperation& Operation)
		{
			return Operation.SideEffectDefinitionHandle == SideEffectDefinitionHandle;
		});
		Property->MakePropertyDirty();
	}

}

void UGunnerPropertyComponent::OnSideEffectEnded(FGunnerActionSideEffectDefinitionHandle SideEffectDefinitionHandle)
{
	for (UGunnerActionProperty* Property : Properties)
	{
		Property->Operations.RemoveAll([SideEffectDefinitionHandle](const FGunnerActionPropertyOperation& Operation)
		{
			return Operation.SideEffectDefinitionHandle == SideEffectDefinitionHandle;
		});
		UE_LOG(LogGunner, Warning, TEXT("UGunnerPropertyComponent::OnSideEffectFailed: %f"), GetWorld()->GetTimeSeconds());
		Property->MakePropertyDirty();
	}
}