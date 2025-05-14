// Fill out your copyright notice in the Description page of Project Settings.


#include "GunnerFogOfWarComponent.h"

#include "CanvasItem.h"
#include "GunnerMiniMapData.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Canvas.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"


UGunnerFogOfWarComponent::UGunnerFogOfWarComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
}

void UGunnerFogOfWarComponent::InitializeComponent()
{
	Super::InitializeComponent();
	FogRenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(
		GetWorld(), UCanvasRenderTarget2D::StaticClass(), 1024, 1024);

	FogRenderTarget->OnCanvasRenderTargetUpdate.AddDynamic(this, &UGunnerFogOfWarComponent::DrawVision);
}


void UGunnerFogOfWarComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (FogRenderTarget)
	{
		FogRenderTarget->UpdateResource();
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
	if (!Canvas || !GeometryAsset)
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
	for (const FGunnerGeometryGroup& Group : GeometryAsset->Groups)
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
