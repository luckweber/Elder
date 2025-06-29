// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ElderAnimInstanceBase.h"
#include "ElderCharacterLinkedAnimLayer.generated.h"

class UElderCharacterAnimInstance;
/**
 * 
 */
UCLASS()
class ELDER_API UElderCharacterLinkedAnimLayer : public UElderAnimInstanceBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	UElderCharacterAnimInstance* GetElderAnimInstance() const;
};
