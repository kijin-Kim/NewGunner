// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerFogOfWarComponent.h"

#include "CanvasItem.h"
#include "GunnerMiniMapData.h"
#include "Engine/Canvas.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Gunner/Gunner.h"
#include "Gunner/_Core/GunnerFogOfWarData.h"
#include "Gunner/_Core/GunnerGameState.h"
#include "Gunner/_Core/GunnerWorldSettings.h"
#include "Gunner/_Core/Character/GunnerCharacter.h"
#include "Kismet/GameplayStatics.h"


UGunnerFogOfWarComponent::UGunnerFogOfWarComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGunnerFogOfWarComponent::SetupFogOfWar(APlayerState* PlayerState)
{
	check(PlayerState);
	IGunnerTeamAgentInterface* TeamAgentInterface = Cast<IGunnerTeamAgentInterface>(PlayerState);
	check(TeamAgentInterface);
	if (ensure(TeamAgentInterface && TeamAgentInterface->GetOnTeamSetDelegate()))
	{
		TeamAgentInterface->GetOnTeamSetDelegate()->AddWeakLambda(this, [this, PlayerState](FGenericTeamId OldTeamId, FGenericTeamId NewTeamId)
		{
			AGunnerGameState* GameState = GetWorld()->GetGameStateChecked<AGunnerGameState>();
			const FGunnerFogOfWarRenderTargets& OldRenderTargets = GameState->FindOrAddPlayerFogOfWarRenderTargets(OldTeamId);
			OldRenderTargets.VisionConeRenderTarget->OnCanvasRenderTargetUpdate.RemoveDynamic(this, &UGunnerFogOfWarComponent::DrawVision);
			OldRenderTargets.InformationRenderTarget->OnCanvasRenderTargetUpdate.RemoveDynamic(this, &UGunnerFogOfWarComponent::DrawInformation);
			const FGunnerFogOfWarRenderTargets& NewRenderTargets = GameState->FindOrAddPlayerFogOfWarRenderTargets(NewTeamId);
			NewRenderTargets.VisionConeRenderTarget->OnCanvasRenderTargetUpdate.AddUniqueDynamic(this, &UGunnerFogOfWarComponent::DrawVision);
			NewRenderTargets.InformationRenderTarget->OnCanvasRenderTargetUpdate.AddUniqueDynamic(this, &UGunnerFogOfWarComponent::DrawInformation);
		});


		AGunnerGameState* GameState = GetWorld()->GetGameStateChecked<AGunnerGameState>();
		const FGunnerFogOfWarRenderTargets& RenderTargets = GameState->FindOrAddPlayerFogOfWarRenderTargets(TeamAgentInterface->GetGenericTeamId());


		RenderTargets.VisionConeRenderTarget->OnCanvasRenderTargetUpdate.AddUniqueDynamic(this, &UGunnerFogOfWarComponent::DrawVision);
		RenderTargets.InformationRenderTarget->OnCanvasRenderTargetUpdate.AddUniqueDynamic(this, &UGunnerFogOfWarComponent::DrawInformation);
	}
}

static bool LineSegmentIntersection(
	const FVector2D& A, const FVector2D& B,
	const FVector2D& C, const FVector2D& D,
	FVector2D& OutIntersection)
{
	const FVector2D AB = B - A;
	const FVector2D CD = D - C;
	const FVector2D AC = C - A;

	const float Denominator = AB.X * CD.Y - AB.Y * CD.X;

	// 평행 또는 거의 평행하면 교차 없음
	if (FMath::IsNearlyZero(Denominator))
	{
		return false;
	}

	const float InvDenom = 1.0f / Denominator;

	const float S = (AC.X * CD.Y - AC.Y * CD.X) * InvDenom;
	const float T = (AC.X * AB.Y - AC.Y * AB.X) * InvDenom;

	constexpr float Epsilon = 1e-5f;

	if (S >= -Epsilon && S <= 1.f + Epsilon &&
		T >= -Epsilon && T <= 1.f + Epsilon)
	{
		OutIntersection = A + S * AB;
		return true;
	}

	return false;
}


void UGunnerFogOfWarComponent::DrawVision(UCanvas* Canvas, int32 Width, int32 Height)
{
	check(Width == Height);

	AGunnerWorldSettings* WorldSettings = Cast<AGunnerWorldSettings>(GetWorld()->GetWorldSettings());
	if (!WorldSettings)
	{
		return;
	}

	UGunnerFogOfWarData* FogOfWarData = WorldSettings->GetFogOfWarData();
	if (!Canvas || !FogOfWarData || !FogOfWarData->GeometryAsset)
	{
		return;
	}


	FVector PlayerEyeLocation;
	FRotator PlayerEyeRotation;
	GetOwner()->GetActorEyesViewPoint(PlayerEyeLocation, PlayerEyeRotation);
	FVector ForwardVector = PlayerEyeRotation.Vector();
	ForwardVector.Z = 0.0f;

	float VisionAngleDeg = 71.0f;
	const float HalfFOVRad = FMath::DegreesToRadians(VisionAngleDeg / 2.0f);


	// 차단 Edge 수집
	TArray<FGunnerEdgeSegment> EdgeSegments;
	for (const FGunnerGeometryGroup& Group : FogOfWarData->GeometryAsset->Groups)
	{
		for (const FGunnerGeometryLine& Line : Group.Lines)
		{
			if (Group.Vertices.IsValidIndex(Line.Start) && Group.Vertices.IsValidIndex(Line.End))
			{
				FVector2D A(Group.Vertices[Line.Start].X, Group.Vertices[Line.Start].Y);
				FVector2D B(Group.Vertices[Line.End].X, Group.Vertices[Line.End].Y);
				EdgeSegments.Add({A, B, Group.ZHeight});
			}
		}
	}


	FVector2D ViewOrigin = {PlayerEyeLocation.Y, -PlayerEyeLocation.X};
	ViewOrigin = (ViewOrigin * 0.12f + Width / 2.0f) / Width;

	float ActorZ = 0.0f;
	if (UPawnMovementComponent* PawnMovementComponent = GetOwner()->GetComponentByClass<UPawnMovementComponent>())
	{
		ActorZ = PawnMovementComponent->GetFeetLocation().Z;
	}
	else
	{
		ActorZ = GetOwner()->GetActorLocation().Z;
	}


	TArray<FVector2D> VisionPoints;
	const int32 RayCount = 64;
	// 시야 Ray 발사
	for (int32 i = 0; i < RayCount; ++i)
	{
		float LocalAngle = -HalfFOVRad + (static_cast<float>(i) / (RayCount - 1)) * (2 * HalfFOVRad);
		const float BaseAngle = FMath::Atan2(ForwardVector.Y, ForwardVector.X) - 90.0f * PI / 180.0f;
		float TotalAngle = BaseAngle + LocalAngle;
		FVector2D Dir(FMath::Cos(TotalAngle), FMath::Sin(TotalAngle));
		FVector2D End = ViewOrigin + Dir * 1.0f;

		FVector2D Closest = End;
		float MinDistSqr = 1.0f * 1.0f;

		for (const auto& Edge : EdgeSegments)
		{
			if (FMath::Abs(ActorZ - Edge.ZHeight) > 125.0f)
			{
				continue;
			}

			FVector2D Hit;
			if (LineSegmentIntersection(ViewOrigin, End, Edge.From, Edge.To, Hit))
			{
				float Dist = FVector2D::DistSquared(ViewOrigin, Hit);
				if (Dist < MinDistSqr)
				{
					Closest = Hit;
					MinDistSqr = Dist;
				}
			}
		}
		VisionPoints.Add(Closest);
	}

	for (int32 i = 0; i < VisionPoints.Num() - 1; ++i)
	{
		const FVector2D A = ViewOrigin * FVector2D(Width, Height);
		const FVector2D B = VisionPoints[i] * FVector2D(Width, Height);
		const FVector2D C = VisionPoints[(i + 1) % VisionPoints.Num()] * FVector2D(Width, Height);

		FCanvasTriangleItem TriangleItem(B, C, A, GWhiteTexture);
		TriangleItem.SetColor(FLinearColor::White);
		Canvas->DrawItem(TriangleItem);
	}
}

void UGunnerFogOfWarComponent::DrawPlayerIcon(UCanvas* Canvas, int32 Width, FVector2D ViewOrigin, FVector ForwardVector, UMaterialInterface* Material)
{
	FVector2D Scale(2.0f, 2.0f);
	Canvas->K2_DrawMaterial(
		Material,
		ViewOrigin * Width - FVector2D(34.0f, 43.0f) * 0.5f * Scale - FVector2D(0.0f, 4.0f) * Scale,
		FVector2D(34.0f, 43.0f) * Scale,
		FVector2D(0.0f, 0.0f),
		FVector2D(1.0f, 1.0f),
		ForwardVector.Rotation().Yaw,
		FVector2D(0.5f, 0.5f) + FVector2D(0.0f, 4.0f / 43.0f)
	);


	Canvas->K2_DrawMaterial(
		PlayerIconMaterial,
		ViewOrigin * Width - 28.0f / 2.0f * Scale,
		FVector2D(28.0f, 28.0f) * Scale,
		FVector2D(0.0f, 0.0f),
		FVector2D(1.0f, 1.0f)
	);
}


void UGunnerFogOfWarComponent::DrawInformation(UCanvas* Canvas, int32 Width, int32 Height)
{
	if (!PlayerIconMaterial || !SelfPortraitContainerMaterial)
	{
		return;
	}

	FVector PlayerEyeLocation;
	FRotator PlayerEyeRotation;
	GetOwner()->GetActorEyesViewPoint(PlayerEyeLocation, PlayerEyeRotation);
	FVector2D ViewOrigin = {PlayerEyeLocation.Y, -PlayerEyeLocation.X};
	ViewOrigin = (ViewOrigin * 0.12f + Width / 2.0f) / Width;
	FVector ForwardVector = PlayerEyeRotation.Vector();
	ForwardVector.Z = 0.0f;
	ForwardVector.Normalize();

	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (PawnOwner && PawnOwner->IsLocallyControlled())
	{
		DrawPlayerIcon(Canvas, Width, ViewOrigin, ForwardVector, SelfPortraitContainerMaterial);
	}
	else
	{
		DrawPlayerIcon(Canvas, Width, ViewOrigin, ForwardVector, AllyPortraitContainerMaterial);
	}


	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGunnerCharacter::StaticClass(), AllActors);
	TArray<AActor*> AllActorsWithInterface;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UGunnerTeamAgentInterface::StaticClass(), AllActorsWithInterface);
	AllActorsWithInterface = AllActorsWithInterface.FilterByPredicate([PawnOwner](AActor* Actor)
	{
		return Cast<IGunnerTeamAgentInterface>(Actor)->GetTeamAttitudeTowards(*PawnOwner) == ETeamAttitude::Friendly;
	});
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());
	CollisionParams.AddIgnoredActors(AllActorsWithInterface);

	for (AActor* Actor : AllActors)
	{
		FHitResult Hit;
		FVector Toward = (Actor->GetActorLocation() - PlayerEyeLocation).GetSafeNormal();

		FVector TowardProject = Toward;
		Toward.Z = 0.0f;
		TowardProject.Normalize();
		float Dot = FVector::DotProduct(ForwardVector, TowardProject);
		if (Dot < FMath::Cos(FMath::DegreesToRadians(71.0f) / 2.0f))
		{
			continue;
		}

		FCollisionResponseParams ResponseParams;
		GetWorld()->LineTraceSingleByChannel(
			Hit,
			PlayerEyeLocation,
			Actor->GetActorLocation() + Toward * 100.0f,
			ECC_Visibility,
			CollisionParams,
			ResponseParams
		);

		if (Hit.GetActor() == Actor)
		{
			FVector2D ActorViewOrigin = {Hit.Location.Y, -Hit.Location.X};
			ActorViewOrigin = (ActorViewOrigin * 0.12f + Width / 2.0f) / Width;

			FVector ActorEyeLocation;
			FRotator ActorEyeRotation;
			Actor->GetActorEyesViewPoint(ActorEyeLocation, ActorEyeRotation);
			FVector ActorForward = ActorEyeRotation.Vector();
			ActorForward.Z = 0.0f;
			DrawPlayerIcon(Canvas, Width, ActorViewOrigin, ActorForward, EnemyPortraitContainerMaterial);
		}
	}
}
;
