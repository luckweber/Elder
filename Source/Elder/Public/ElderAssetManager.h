// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "ElderAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class ELDER_API UElderAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:

	static UElderAssetManager& Get();

protected:

	virtual void StartInitialLoading() override;
};
