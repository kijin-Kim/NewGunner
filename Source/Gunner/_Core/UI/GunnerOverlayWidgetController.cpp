// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerOverlayWidgetController.h"
#include "Action/NexusActionComponent.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/_Core/GunnerNativeGameplayTags.h"


void UGunnerOverlayWidgetController::InitWidgetController(APlayerState* InPlayerState)
{
	Super::InitWidgetController(InPlayerState);

	APlayerController* PlayerController = PlayerState->GetPlayerController();
	check(PlayerController && PlayerController->GetPawn());

	UNexusActionComponent* ActionComponent = UNexusActionComponent::GetActionComponentFromActor(PlayerController->GetPawn());
	check(ActionComponent);

	UNexusProperty* SlotIndexProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_SlotIndex);
	check(SlotIndexProperty);
	SlotIndexProperty->OnDirtyDelegate.AddDynamic(this, &UGunnerOverlayWidgetController::OnSlotIndexValueDirty);
	OnSlotIndexValueDirty(SlotIndexProperty->GetDynamicValue(), SlotIndexProperty->GetDynamicValue());
	
	UNexusProperty* BulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_Bullet);
	check(BulletProperty);
	BulletProperty->OnDirtyDelegate.AddDynamic(this, &UGunnerOverlayWidgetController::OnBulletValueDirty);
	OnBulletValueDirty(BulletProperty->GetDynamicValue(), BulletProperty->GetDynamicValue());

	UNexusProperty* MagazineBulletProperty = ActionComponent->GetProperty(GunnerNativeGameplayTags::TAG_Property_MagazineBullet);
	check(MagazineBulletProperty);
	MagazineBulletProperty->OnDirtyDelegate.AddDynamic(this, &UGunnerOverlayWidgetController::OnMagazineBulletValueDirty);
	OnMagazineBulletValueDirty(MagazineBulletProperty->GetDynamicValue(), MagazineBulletProperty->GetDynamicValue());
}

void UGunnerOverlayWidgetController::OnSlotIndexValueDirty(float OldValue, float NewValue)
{
	OnSlotIndexValueDirtyDelegate.Broadcast(OldValue, NewValue);
}

void UGunnerOverlayWidgetController::OnBulletValueDirty(float OldValue, float NewValue)
{
	OnBulletValueDirtyDelegate.Broadcast(OldValue, NewValue);
}

void UGunnerOverlayWidgetController::OnMagazineBulletValueDirty(float OldValue, float NewValue)
{
	OnMagazineBulletValueDirtyDelegate.Broadcast(OldValue, NewValue);
}
