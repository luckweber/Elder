// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ElderAbilitySystemGlobals.h"

#include "ElderAbilityTypes.h"

FGameplayEffectContext* UElderAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FElderGameplayEffectContext();
}
