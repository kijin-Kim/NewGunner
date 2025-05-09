// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerNativeGameplayTags.h"

namespace GunnerNativeGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Look, "Input.Look");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Move, "Input.Move");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Jump, "Input.Jump");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Crouch, "Input.Crouch");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Uncrouch, "Input.Uncrouch");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Drop, "Input.Drop");

	UE_DEFINE_GAMEPLAY_TAG(TAG_Action_Equip, "Action.Equip");


	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_ActivateSlot_Primary, "Input.ActivateSlot.Primary");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_ActivateSlot_Secondary, "Input.ActivateSlot.Secondary");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_ActivateSlot_Melee, "Input.ActivateSlot.Melee");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_ActivateSlot_Spike, "Input.ActivateSlot.Spike");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_ActivateSlot_BasicFirst, "Input.ActivateSlot.BasicFirst");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_ActivateSlot_BasicSecond, "Input.ActivateSlot.BasicSecond");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_ActivateSlot_Signature, "Input.ActivateSlot.Signature");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_ActivateSlot_Ultimate, "Input.ActivateSlot.Ultimate");

	UE_DEFINE_GAMEPLAY_TAG(TAG_GameEvent_Damaged, "GameEvent.Damaged");
	UE_DEFINE_GAMEPLAY_TAG(TAG_GameEvent_Death, "GameEvent.Death");
	UE_DEFINE_GAMEPLAY_TAG(TAG_GameEvent_CycleSlot, "GameEvent.CycleSlot");


	UE_DEFINE_GAMEPLAY_TAG(TAG_OperationValue_SlotIndex, "OperationValue.SlotIndex");
	UE_DEFINE_GAMEPLAY_TAG(TAG_OperationValue_Bullet, "OperationValue.Bullet");
	UE_DEFINE_GAMEPLAY_TAG(TAG_OperationValue_MagazineBullet, "OperationValue.MagazineBullet");
	UE_DEFINE_GAMEPLAY_TAG(TAG_OperationValue_MaxBulletPerMagazine, "OperationValue.MaxBulletPerMagazine");

	UE_DEFINE_GAMEPLAY_TAG(TAG_Property_Health, "Property.Health");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Property_Bullet, "Property.Bullet");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Property_MagazineBullet, "Property.MagazineBullet");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Property_MaxBulletPerMagazine, "Property.MaxBulletPerMagazine");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Property_SlotIndex, "Property.SlotIndex");
}

