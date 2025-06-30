// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/ElderCharacter.h"

#include "AbilitySystemComponent.h"
#include "ElderGameplayTags.h"
#include "AbilitySystem/ElderAbilitySystemComponent.h"
#include "AbilitySystem/ElderAbilitySystemLibrary.h"
#include "AbilitySystem/ElderAttributeSet.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"
#include "Camera/CameraComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Game/ElderGameModeBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ElderPlayerController.h"
#include "Player/ElderPlayerState.h"

AElderCharacter::AElderCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 800.0f;
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bInheritYaw = false;
	
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>("TopDownCameraComponent");
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;
	
	LevelUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("LevelUpNiagaraComponent");
	LevelUpNiagaraComponent->SetupAttachment(GetRootComponent());
	LevelUpNiagaraComponent->bAutoActivate = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CharacterClass = ECharacterClass::Elementalist;
}

void AElderCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Init ability actor info for the Server
	InitAbilityActorInfo();
	LoadProgress();

	if (AElderGameModeBase* ElderGameMode = Cast<AElderGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		ElderGameMode->LoadWorldState(GetWorld());
	}
}

void AElderCharacter::LoadProgress()
{
	AElderGameModeBase* ElderGameMode = Cast<AElderGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (ElderGameMode)
	{
		ULoadScreenSaveGame* SaveData = ElderGameMode->RetrieveInGameSaveData();
		if (SaveData == nullptr) return;

		if (SaveData->bFirstTimeLoadIn)
		{
			InitializeDefaultAttributes();
			AddCharacterAbilities();
		}
		else
		{
			if (UElderAbilitySystemComponent* ElderASC = Cast<UElderAbilitySystemComponent>(AbilitySystemComponent))
			{
				ElderASC->AddCharacterAbilitiesFromSaveData(SaveData);
			}
			
			if (AElderPlayerState* ElderPlayerState = Cast<AElderPlayerState>(GetPlayerState()))
			{
				ElderPlayerState->SetLevel(SaveData->PlayerLevel);
				ElderPlayerState->SetXP(SaveData->XP);
				ElderPlayerState->SetAttributePoints(SaveData->AttributePoints);
				ElderPlayerState->SetSpellPoints(SaveData->SpellPoints);
			}
			
			// UElderAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(this, AbilitySystemComponent, SaveData);
		}
	}
}

void AElderCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Init ability actor info for the Client
	InitAbilityActorInfo();
}

void AElderCharacter::AddToXP_Implementation(int32 InXP)
{
	AElderPlayerState* ElderPlayerState = GetPlayerState<AElderPlayerState>();
	check(ElderPlayerState);
	ElderPlayerState->AddToXP(InXP);
}

void AElderCharacter::LevelUp_Implementation()
{
	MulticastLevelUpParticles();
}

void AElderCharacter::MulticastLevelUpParticles_Implementation() const
{
	if (IsValid(LevelUpNiagaraComponent))
	{
		const FVector CameraLocation = TopDownCameraComponent->GetComponentLocation();
		const FVector NiagaraSystemLocation = LevelUpNiagaraComponent->GetComponentLocation();
		const FRotator ToCameraRotation = (CameraLocation - NiagaraSystemLocation).Rotation();
		LevelUpNiagaraComponent->SetWorldRotation(ToCameraRotation);
		LevelUpNiagaraComponent->Activate(true);
	}
}

int32 AElderCharacter::GetXP_Implementation() const
{
	const AElderPlayerState* ElderPlayerState = GetPlayerState<AElderPlayerState>();
	check(ElderPlayerState);
	return ElderPlayerState->GetXP();
}

int32 AElderCharacter::FindLevelForXP_Implementation(int32 InXP) const
{
	const AElderPlayerState* ElderPlayerState = GetPlayerState<AElderPlayerState>();
	check(ElderPlayerState);
	return ElderPlayerState->LevelUpInfo->FindLevelForXP(InXP);
}

int32 AElderCharacter::GetAttributePointsReward_Implementation(int32 Level) const
{
	const AElderPlayerState* ElderPlayerState = GetPlayerState<AElderPlayerState>();
	check(ElderPlayerState);
	return ElderPlayerState->LevelUpInfo->LevelUpInformation[Level].AttributePointAward;
}

int32 AElderCharacter::GetSpellPointsReward_Implementation(int32 Level) const
{
	const AElderPlayerState* ElderPlayerState = GetPlayerState<AElderPlayerState>();
	check(ElderPlayerState);
	return ElderPlayerState->LevelUpInfo->LevelUpInformation[Level].SpellPointAward;
}

void AElderCharacter::AddToPlayerLevel_Implementation(int32 InPlayerLevel)
{
	AElderPlayerState* ElderPlayerState = GetPlayerState<AElderPlayerState>();
	check(ElderPlayerState);
	ElderPlayerState->AddToLevel(InPlayerLevel);

	if (UElderAbilitySystemComponent* ElderASC = Cast<UElderAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ElderASC->UpdateAbilityStatuses(ElderPlayerState->GetPlayerLevel());
	}
}

void AElderCharacter::AddToAttributePoints_Implementation(int32 InAttributePoints)
{
	AElderPlayerState* ElderPlayerState = GetPlayerState<AElderPlayerState>();
	check(ElderPlayerState);
	ElderPlayerState->AddToAttributePoints(InAttributePoints);
}

void AElderCharacter::AddToSpellPoints_Implementation(int32 InSpellPoints)
{
	AElderPlayerState* ElderPlayerState = GetPlayerState<AElderPlayerState>();
	check(ElderPlayerState);
	ElderPlayerState->AddToSpellPoints(InSpellPoints);
}

int32 AElderCharacter::GetAttributePoints_Implementation() const
{
	AElderPlayerState* ElderPlayerState = GetPlayerState<AElderPlayerState>();
	check(ElderPlayerState);
	return ElderPlayerState->GetAttributePoints();
}

int32 AElderCharacter::GetSpellPoints_Implementation() const
{
	AElderPlayerState* ElderPlayerState = GetPlayerState<AElderPlayerState>();
	check(ElderPlayerState);
	return ElderPlayerState->GetSpellPoints();
}

void AElderCharacter::ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial)
{
	if (AElderPlayerController* ElderPlayerController = Cast<AElderPlayerController>(GetController()))
	{
		ElderPlayerController->ShowMagicCircle(DecalMaterial);
		ElderPlayerController->bShowMouseCursor = false;
	}
}

void AElderCharacter::HideMagicCircle_Implementation()
{
	if (AElderPlayerController* ElderPlayerController = Cast<AElderPlayerController>(GetController()))
	{
		ElderPlayerController->HideMagicCircle();
		ElderPlayerController->bShowMouseCursor = true;
	}
}

void AElderCharacter::SaveProgress_Implementation(const FName& CheckpointTag)
{
	AElderGameModeBase* ElderGameMode = Cast<AElderGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (ElderGameMode)
	{
		ULoadScreenSaveGame* SaveData = ElderGameMode->RetrieveInGameSaveData();
		if (SaveData == nullptr) return;

		SaveData->PlayerStartTag = CheckpointTag;

		if (AElderPlayerState* ElderPlayerState = Cast<AElderPlayerState>(GetPlayerState()))
		{
			SaveData->PlayerLevel = ElderPlayerState->GetPlayerLevel();
			SaveData->XP = ElderPlayerState->GetXP();
			SaveData->AttributePoints = ElderPlayerState->GetAttributePoints();
			SaveData->SpellPoints = ElderPlayerState->GetSpellPoints();
		}
		SaveData->Strength = UElderAttributeSet::GetStrengthAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Intelligence = UElderAttributeSet::GetIntelligenceAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Resilience = UElderAttributeSet::GetResilienceAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Vigor = UElderAttributeSet::GetVigorAttribute().GetNumericValue(GetAttributeSet());

		SaveData->bFirstTimeLoadIn = false;

		if (!HasAuthority()) return;

		UElderAbilitySystemComponent* ElderASC = Cast<UElderAbilitySystemComponent>(AbilitySystemComponent);
		FForEachAbility SaveAbilityDelegate;
		SaveData->SavedAbilities.Empty();
		// SaveAbilityDelegate.BindLambda([this, ElderASC, SaveData](const FGameplayAbilitySpec& AbilitySpec)
		// {
		// 	const FGameplayTag AbilityTag = ElderASC->GetAbilityTagFromSpec(AbilitySpec);
		// 	UAbilityInfo* AbilityInfo = UElderAbilitySystemLibrary::GetAbilityInfo(this);
		// 	FElderAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
		//
		// 	FSavedAbility SavedAbility;
		// 	SavedAbility.GameplayAbility = Info.Ability;
		// 	SavedAbility.AbilityLevel = AbilitySpec.Level;
		// 	SavedAbility.AbilitySlot = ElderASC->GetSlotFromAbilityTag(AbilityTag);
		// 	SavedAbility.AbilityStatus = ElderASC->GetStatusFromAbilityTag(AbilityTag);
		// 	SavedAbility.AbilityTag = AbilityTag;
		// 	SavedAbility.AbilityType = Info.AbilityType;
		//
		// 	SaveData->SavedAbilities.AddUnique(SavedAbility);
		//
		// });
		ElderASC->ForEachAbility(SaveAbilityDelegate);
		
		ElderGameMode->SaveInGameProgressData(SaveData);
	}
}

int32 AElderCharacter::GetPlayerLevel_Implementation()
{
	const AElderPlayerState* ElderPlayerState = GetPlayerState<AElderPlayerState>();
	check(ElderPlayerState);
	return ElderPlayerState->GetPlayerLevel();
}

void AElderCharacter::Die(const FVector& DeathImpulse)
{
	Super::Die(DeathImpulse);

	FTimerDelegate DeathTimerDelegate;
	DeathTimerDelegate.BindLambda([this]()
	{
		AElderGameModeBase* ElderGM = Cast<AElderGameModeBase>(UGameplayStatics::GetGameMode(this));
		if (ElderGM)
		{
			ElderGM->PlayerDied(this);
		}
	});
	GetWorldTimerManager().SetTimer(DeathTimer, DeathTimerDelegate, DeathTime, false);
	TopDownCameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
}

void AElderCharacter::OnRep_Stunned()
{
	if (UElderAbilitySystemComponent* ElderASC = Cast<UElderAbilitySystemComponent>(AbilitySystemComponent))
	{
		const FElderGameplayTags& GameplayTags = FElderGameplayTags::Get();
		FGameplayTagContainer BlockedTags;
		BlockedTags.AddTag(GameplayTags.Player_Block_CursorTrace);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputHeld);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputPressed);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputReleased);
		if (bIsStunned)
		{
			ElderASC->AddLooseGameplayTags(BlockedTags);
			StunDebuffComponent->Activate();
		}
		else
		{
			ElderASC->RemoveLooseGameplayTags(BlockedTags);
			StunDebuffComponent->Deactivate();
		}
	}
}

void AElderCharacter::OnRep_Burned()
{
	if (bIsBurned)
	{
		BurnDebuffComponent->Activate();
	}
	else
	{
		BurnDebuffComponent->Deactivate();
	}
}

void AElderCharacter::InitAbilityActorInfo()
{
	AElderPlayerState* ElderPlayerState = GetPlayerState<AElderPlayerState>();
	check(ElderPlayerState);
	ElderPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(ElderPlayerState, this);
	Cast<UElderAbilitySystemComponent>(ElderPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = ElderPlayerState->GetAbilitySystemComponent();
	AttributeSet = ElderPlayerState->GetAttributeSet();
	OnAscRegistered.Broadcast(AbilitySystemComponent);
	AbilitySystemComponent->RegisterGameplayTagEvent(FElderGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AElderCharacter::StunTagChanged);

	if (AElderPlayerController* ElderPlayerController = Cast<AElderPlayerController>(GetController()))
	{
		// if (AElderHUD* ElderHUD = Cast<AElderHUD>(ElderPlayerController->GetHUD()))
		// {
		// 	ElderHUD->InitOverlay(ElderPlayerController, ElderPlayerState, AbilitySystemComponent, AttributeSet);
		// }
	}
}