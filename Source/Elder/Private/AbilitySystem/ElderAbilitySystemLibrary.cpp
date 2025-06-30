// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ElderAbilitySystemLibrary.h"

#include "AbilitySystem/ElderAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "ElderGameplayTags.h"
#include "Engine/OverlapResult.h"
#include "Interaction/CombatInterface.h"
#include "ElderAbilityTypes.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Game/ElderGameModeBase.h"
#include "Kismet/GameplayStatics.h"


UElderAbilitySystemComponent* UElderAbilitySystemLibrary::NativeGetElderASCFromActor(AActor* InActor)
{
	check(InActor);

	return CastChecked<UElderAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor));
}

bool UElderAbilitySystemLibrary::NativeDoesActorHaveTag(AActor* InActor, FGameplayTag TagToCheck)
{
	UElderAbilitySystemComponent* ASC = NativeGetElderASCFromActor(InActor);

	return ASC->HasMatchingGameplayTag(TagToCheck);
}


void UElderAbilitySystemLibrary::AddGameplayTagToActorIfNone(AActor* InActor, FGameplayTag TagToAdd)
{
	UElderAbilitySystemComponent* ASC = NativeGetElderASCFromActor(InActor);

	if (!ASC->HasMatchingGameplayTag(TagToAdd))
	{
		ASC->AddLooseGameplayTag(TagToAdd);
	}
}

void UElderAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject,
	ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	AActor* AvatarActor = ASC->GetAvatarActor();

	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	FGameplayEffectContextHandle PrimaryAttributesContextHandle = ASC->MakeEffectContext();
	PrimaryAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle PrimaryAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttributesSpecHandle.Data.Get());

	FGameplayEffectContextHandle SecondaryAttributesContextHandle = ASC->MakeEffectContext();
	SecondaryAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, SecondaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());

	FGameplayEffectContextHandle VitalAttributesContextHandle = ASC->MakeEffectContext();
	VitalAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, VitalAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
}

void UElderAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC,
                                                      ECharacterClass CharacterClass)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return;
	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}
	const FCharacterClassDefaultInfo& DefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultInfo.StartupAbilities)
	{
		if (ASC->GetAvatarActor()->Implements<UCombatInterface>())
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, ICombatInterface::Execute_GetPlayerLevel(ASC->GetAvatarActor()));
			ASC->GiveAbility(AbilitySpec);
		}
	}
}

UCharacterClassInfo* UElderAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	const AElderGameModeBase* ElderGameMode = Cast<AElderGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (ElderGameMode == nullptr) return nullptr;
	return ElderGameMode->CharacterClassInfo;
}

void UElderAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject,
                                                            TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius,
                                                            const FVector& SphereOrigin)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);
	
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);
		for (FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Overlap.GetActor()))
			{
				OutOverlappingActors.AddUnique(ICombatInterface::Execute_GetAvatar(Overlap.GetActor()));
			}
		}
	}
}

void UElderAbilitySystemLibrary::GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors,
	TArray<AActor*>& OutClosestTargets, const FVector& Origin)
{
	if (Actors.Num() <= MaxTargets)
	{
		OutClosestTargets = Actors;
		return;
	}

	TArray<AActor*> ActorsToCheck = Actors;
	int32 NumTargetsFound = 0;

	while (NumTargetsFound < MaxTargets)
	{
		if (ActorsToCheck.Num() == 0) break;
		double ClosestDistance = TNumericLimits<double>::Max();
		AActor* ClosestActor;
		for (AActor* PotentialTarget : ActorsToCheck)
		{
			const double Distance = (PotentialTarget->GetActorLocation() - Origin).Length();
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestActor = PotentialTarget;
			}
		}
		ActorsToCheck.Remove(ClosestActor);
		OutClosestTargets.AddUnique(ClosestActor);
		++NumTargetsFound;
	}
}

bool UElderAbilitySystemLibrary::IsNotFriend(AActor* FirstActor, AActor* SecondActor)
{
	const bool bBothArePlayers = FirstActor->ActorHasTag(FName("Player")) && SecondActor->ActorHasTag(FName("Player"));
	const bool bBothAreEnemies = FirstActor->ActorHasTag(FName("Enemy")) && SecondActor->ActorHasTag(FName("Enemy"));
	const bool bFriends = bBothArePlayers || bBothAreEnemies;
	return !bFriends;
}

FGameplayEffectContextHandle UElderAbilitySystemLibrary::ApplyDamageEffect(
	const FDamageEffectParams& DamageEffectParams)
{
	const FElderGameplayTags& GameplayTags = FElderGameplayTags::Get();
	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();
	
	FGameplayEffectContextHandle EffectContexthandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContexthandle.AddSourceObject(SourceAvatarActor);
	SetDeathImpulse(EffectContexthandle, DamageEffectParams.DeathImpulse);
	SetKnockbackForce(EffectContexthandle, DamageEffectParams.KnockbackForce);

	SetIsRadialDamage(EffectContexthandle, DamageEffectParams.bIsRadialDamage);
	SetRadialDamageInnerRadius(EffectContexthandle, DamageEffectParams.RadialDamageInnerRadius);
	SetRadialDamageOuterRadius(EffectContexthandle, DamageEffectParams.RadialDamageOuterRadius);
	SetRadialDamageOrigin(EffectContexthandle, DamageEffectParams.RadialDamageOrigin);
	
	const FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(DamageEffectParams.DamageGameplayEffectClass, DamageEffectParams.AbilityLevel, EffectContexthandle);

	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageEffectParams.DamageType, DamageEffectParams.BaseDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Chance, DamageEffectParams.DebuffChance);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Damage, DamageEffectParams.DebuffDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Duration, DamageEffectParams.DebuffDuration);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Frequency, DamageEffectParams.DebuffFrequency);
	
	DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
	return EffectContexthandle;
}

TArray<FRotator> UElderAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis,
	float Spread, int32 NumRotators)
{
	TArray<FRotator> Rotators;
	
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	if (NumRotators > 1)
	{
		const float DeltaSpread = Spread / (NumRotators - 1);
		for (int32 i = 0; i < NumRotators; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			Rotators.Add(Direction.Rotation());
		}
	}
	else
	{
		Rotators.Add(Forward.Rotation());
	}
	return Rotators;
}

TArray<FVector> UElderAbilitySystemLibrary::EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis,
	float Spread, int32 NumVectors)
{
	TArray<FVector> Vectors;
	
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	if (NumVectors > 1)
	{
		const float DeltaSpread = Spread / (NumVectors - 1);
		for (int32 i = 0; i < NumVectors; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			Vectors.Add(Direction);
		}
	}
	else
	{
		Vectors.Add(Forward);
	}
	return Vectors;
}

int32 UElderAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject,
	ECharacterClass CharacterClass, int32 CharacterLevel)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return 0;

	const FCharacterClassDefaultInfo& Info = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	const float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);

	return static_cast<int32>(XPReward);
}

bool UElderAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FElderGameplayEffectContext* ElderEffectContext = static_cast<const FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return ElderEffectContext->IsBlockedHit();
	}
	return false;
}

bool UElderAbilitySystemLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FElderGameplayEffectContext* ElderEffectContext = static_cast<const FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return ElderEffectContext->IsSuccessfulDebuff();
	}
	return false;
}

float UElderAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FElderGameplayEffectContext* ElderEffectContext = static_cast<const FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return ElderEffectContext->GetDebuffDamage();
	}
	return 0.f;
}

float UElderAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FElderGameplayEffectContext* ElderEffectContext = static_cast<const FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return ElderEffectContext->GetDebuffDuration();
	}
	return 0.f;
}

float UElderAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FElderGameplayEffectContext* ElderEffectContext = static_cast<const FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return ElderEffectContext->GetDebuffFrequency();
	}
	return 0.f;
}

FGameplayTag UElderAbilitySystemLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FElderGameplayEffectContext* ElderEffectContext = static_cast<const FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		if (ElderEffectContext->GetDamageType().IsValid())
		{
			return *ElderEffectContext->GetDamageType();
		}
	}
	return FGameplayTag();
}

FVector UElderAbilitySystemLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FElderGameplayEffectContext* ElderEffectContext = static_cast<const FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return ElderEffectContext->GetDeathImpulse();
	}
	return FVector::ZeroVector;
}

FVector UElderAbilitySystemLibrary::GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FElderGameplayEffectContext* ElderEffectContext = static_cast<const FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return ElderEffectContext->GetKnockbackForce();
	}
	return FVector::ZeroVector;
}

bool UElderAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FElderGameplayEffectContext* ElderEffectContext = static_cast<const FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return ElderEffectContext->IsCriticalHit();
	}
	return false;
}

bool UElderAbilitySystemLibrary::IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FElderGameplayEffectContext* ElderEffectContext = static_cast<const FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return ElderEffectContext->IsRadialDamage();
	}
	return false;
}

float UElderAbilitySystemLibrary::GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FElderGameplayEffectContext* ElderEffectContext = static_cast<const FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return ElderEffectContext->GetRadialDamageInnerRadius();
	}
	return 0.f;
}

float UElderAbilitySystemLibrary::GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FElderGameplayEffectContext* ElderEffectContext = static_cast<const FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return ElderEffectContext->GetRadialDamageOuterRadius();
	}
	return 0.f;
}

FVector UElderAbilitySystemLibrary::GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FElderGameplayEffectContext* ElderEffectContext = static_cast<const FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return ElderEffectContext->GetRadialDamageOrigin();
	}
	return FVector::ZeroVector;
}

void UElderAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle,
                                                 bool bInIsBlockedHit)
{
	if (FElderGameplayEffectContext* ElderEffectContext = static_cast<FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		ElderEffectContext->SetIsBlockedHit(bInIsBlockedHit);
	}
}

void UElderAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInIsCriticalHit)
{
	if (FElderGameplayEffectContext* ElderEffectContext = static_cast<FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		ElderEffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

void UElderAbilitySystemLibrary::SetIsSuccessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInSuccessfulDebuff)
{
	if (FElderGameplayEffectContext* ElderEffectContext = static_cast<FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		ElderEffectContext->SetIsSuccessfulDebuff(bInSuccessfulDebuff);
	}
}

void UElderAbilitySystemLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle, float InDamage)
{
	if (FElderGameplayEffectContext* ElderEffectContext = static_cast<FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		ElderEffectContext->SetDebuffDamage(InDamage);
	}
}

void UElderAbilitySystemLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle, float InDuration)
{
	if (FElderGameplayEffectContext* ElderEffectContext = static_cast<FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		ElderEffectContext->SetDebuffDuration(InDuration);
	}
}

void UElderAbilitySystemLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle,
	float InFrequency)
{
	if (FElderGameplayEffectContext* ElderEffectContext = static_cast<FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		ElderEffectContext->SetDebuffFrequency(InFrequency);
	}
}

void UElderAbilitySystemLibrary::SetDamageType(FGameplayEffectContextHandle& EffectContextHandle,
	const FGameplayTag& InDamageType)
{
	if (FElderGameplayEffectContext* ElderEffectContext = static_cast<FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		const TSharedPtr<FGameplayTag> DamageType = MakeShared<FGameplayTag>(InDamageType);
		ElderEffectContext->SetDamageType(DamageType);
	}
}

void UElderAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InImpulse)
{
	if (FElderGameplayEffectContext* ElderEffectContext = static_cast<FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		ElderEffectContext->SetDeathImpulse(InImpulse);
	}
}

void UElderAbilitySystemLibrary::SetKnockbackForce(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InForce)
{
	if (FElderGameplayEffectContext* ElderEffectContext = static_cast<FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		ElderEffectContext->SetKnockbackForce(InForce);
	}
}

void UElderAbilitySystemLibrary::SetIsRadialDamage(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInIsRadialDamage)
{
	if (FElderGameplayEffectContext* ElderEffectContext = static_cast<FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		ElderEffectContext->SetIsRadialDamage(bInIsRadialDamage);
	}
}

void UElderAbilitySystemLibrary::SetRadialDamageInnerRadius(FGameplayEffectContextHandle& EffectContextHandle,
	float InInnerRadius)
{
	if (FElderGameplayEffectContext* ElderEffectContext = static_cast<FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		ElderEffectContext->SetRadialDamageInnerRadius(InInnerRadius);
	}
}

void UElderAbilitySystemLibrary::SetRadialDamageOuterRadius(FGameplayEffectContextHandle& EffectContextHandle,
	float InOuterRadius)
{
	if (FElderGameplayEffectContext* ElderEffectContext = static_cast<FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		ElderEffectContext->SetRadialDamageOuterRadius(InOuterRadius);
	}
}

void UElderAbilitySystemLibrary::SetRadialDamageOrigin(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InOrigin)
{
	if (FElderGameplayEffectContext* ElderEffectContext = static_cast<FElderGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		ElderEffectContext->SetRadialDamageOrigin(InOrigin);
	}
}
