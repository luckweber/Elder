// Fill out your copyright notice in the Description page of Project Settings.


#include "ElderAssetManager.h"

#include "AbilitySystemGlobals.h"
#include "ElderGameplayTags.h"

UElderAssetManager& UElderAssetManager::Get()
{
	check(GEngine);
	
	UElderAssetManager* ElderAssetManager = Cast<UElderAssetManager>(GEngine->AssetManager);
	return *ElderAssetManager;
}

void UElderAssetManager::StartInitialLoading()
{

	Super::StartInitialLoading();
	FElderGameplayTags::InitializeNativeGameplayTags();

	// This is required to use Target Data!
	UAbilitySystemGlobals::Get().InitGlobalData();
}
