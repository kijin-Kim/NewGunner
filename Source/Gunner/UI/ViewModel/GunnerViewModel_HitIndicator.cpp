// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerViewModel_HitIndicator.h"

#include "Blueprint/UserWidget.h"
#include "Gunner/_Core/Player/GunnerPlayerController.h"

void UGunnerViewModel_HitIndicator::OnCreateViewModel(const UUserWidget* UserWidget)
{
	Super::OnCreateViewModel(UserWidget);
	PlayerController = UserWidget->GetOwningPlayer<AGunnerPlayerController>();
	if (PlayerController)
	{
		PlayerController->OnGunnerPlayerControlRotationChangedDelegate.AddUObject(this, &UGunnerViewModel_HitIndicator::OnPlayerControlRotationChanged);
	}
	VictimActor = UserWidget->GetOwningPlayerPawn();
	OnPlayerControlRotationChanged(PlayerController->GetControlRotation());
}

void UGunnerViewModel_HitIndicator::OnDestroyViewModel(const UObject* Object, const UMVVMView* View) const
{
	Super::OnDestroyViewModel(Object, View);
	if (PlayerController)
	{
		PlayerController->OnGunnerPlayerControlRotationChangedDelegate.RemoveAll(this);
	}
}

void UGunnerViewModel_HitIndicator::OnPlayerControlRotationChanged(const FRotator& Rotator)
{
	if (ensure(VictimActor))
	{
		FVector VictimLocation = VictimActor->GetActorLocation();
		const FVector ToCauser = (CauserLocation - VictimLocation).GetSafeNormal2D();


		FVector2D ToCauser2D(ToCauser.X, ToCauser.Y);

		FVector Forward = Rotator.Vector().GetSafeNormal2D();
		FVector2D VictimForward2D = { Forward.X, Forward.Y };
		

		FVector VictimForward = Rotator.Vector();
		const float DotFront = ToCauser2D.Dot(VictimForward2D);
		const float CrossZ = VictimForward.Cross(ToCauser).Z;

		const float Angle = (180.0) / UE_DOUBLE_PI * FMath::Acos(DotFront);
		RenderTransformAngle = (CrossZ < 0.0f) ? -Angle : Angle;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(RenderTransformAngle);
	}
}
