// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunnerButtonWidget.generated.h"

class USizeBox;
class UBorder;
class UButton;
/**
 * 
 */
UCLASS()
class GUNNER_API UGunnerButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;

	
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnButtonPressed();
	UFUNCTION()
	void OnButtonReleased();


public:
	
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	USizeBox* SizeBox;
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UBorder* Border;
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* Button;
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor BackgroundColor;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor InnerBackgroundColor;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor OutlineColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor HighlightColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Width;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Height;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float OutlineWidth = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<enum ESlateBrushRoundingType::Type> RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector4 CornerRadii{4.0f, 4.0f, 4.0f, 4.0f};

	
};
