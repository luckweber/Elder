// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ElderAttributeSet.h"

#include "Net/UnrealNetwork.h"

UElderAttributeSet::UElderAttributeSet()
{
}

void UElderAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Primary Attributes
	
	DOREPLIFETIME_CONDITION_NOTIFY(UElderAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UElderAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UElderAttributeSet, Agility, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UElderAttributeSet, Vitality, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UElderAttributeSet, Wisdom, COND_None, REPNOTIFY_Always);

	// Vital Attributes
	
	DOREPLIFETIME_CONDITION_NOTIFY(UElderAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UElderAttributeSet, Mana, COND_None, REPNOTIFY_Always);
}

void UElderAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UElderAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void UElderAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

void UElderAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UElderAttributeSet, Strength, OldStrength);
}

void UElderAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldSIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UElderAttributeSet, Intelligence, OldSIntelligence);
}

void UElderAttributeSet::OnRep_Agility(const FGameplayAttributeData& OldAgility) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UElderAttributeSet, Agility, OldAgility);
}

void UElderAttributeSet::OnRep_Vitality(const FGameplayAttributeData& OldVitality) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UElderAttributeSet, Vitality, OldVitality);
}

void UElderAttributeSet::OnRep_Wisdom(const FGameplayAttributeData& OldWisdom) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UElderAttributeSet, Wisdom, OldWisdom);
}

void UElderAttributeSet::OnRep_Health(const FGameplayAttributeData& Oldealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UElderAttributeSet, Health, Oldealth);
}

void UElderAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UElderAttributeSet, Mana, OldMana);
}
