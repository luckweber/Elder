// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstance/ElderAnimInstanceBase.h"

#include "AbilitySystem/ElderAbilitySystemLibrary.h"

bool UElderAnimInstanceBase::DoesOwnerHaveTag(FGameplayTag TagToCheck) const
{
	if (APawn* OwningPawn = TryGetPawnOwner())
	{
		return UElderAbilitySystemLibrary::NativeDoesActorHaveTag(OwningPawn, TagToCheck);
	}

	return false;
}
