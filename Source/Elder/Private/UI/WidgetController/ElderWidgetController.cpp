// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/ElderWidgetController.h"

#include "Player/ElderPlayerController.h"
#include "Player/ElderPlayerState.h"
#include "AbilitySystem/ElderAbilitySystemComponent.h"
#include "AbilitySystem/ElderAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"

void UElderWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
}

void UElderWidgetController::BroadcastInitialValues()
{
	
}

void UElderWidgetController::BindCallbacksToDependencies()
{
	
}

void UElderWidgetController::BroadcastAbilityInfo()
{
	if (!GetElderASC()->bStartupAbilitiesGiven) return;

	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
	{
		FElderAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(ElderAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec));
		Info.InputTag = ElderAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
		Info.StatusTag = ElderAbilitySystemComponent->GetStatusFromSpec(AbilitySpec);
		AbilityInfoDelegate.Broadcast(Info);
	});
	GetElderASC()->ForEachAbility(BroadcastDelegate);
}

AElderPlayerController* UElderWidgetController::GetElderPC()
{
	if (ElderPlayerController == nullptr)
	{
		ElderPlayerController = Cast<AElderPlayerController>(PlayerController);
	}
	return ElderPlayerController;
}

AElderPlayerState* UElderWidgetController::GetElderPS()
{
	if (ElderPlayerState == nullptr)
	{
		ElderPlayerState = Cast<AElderPlayerState>(PlayerState);
	}
	return ElderPlayerState;
}

UElderAbilitySystemComponent* UElderWidgetController::GetElderASC()
{
	if (ElderAbilitySystemComponent == nullptr)
	{
		ElderAbilitySystemComponent = Cast<UElderAbilitySystemComponent>(AbilitySystemComponent);
	}
	return ElderAbilitySystemComponent;
}

UElderAttributeSet* UElderWidgetController::GetElderAS()
{
	if (ElderAttributeSet == nullptr)
	{
		ElderAttributeSet = Cast<UElderAttributeSet>(AttributeSet);
	}
	return ElderAttributeSet;
}