// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/ElderCharacter.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/ElderPlayerState.h"

AElderCharacter::AElderCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 800.0f;
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bInheritYaw = false;
	
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>("TopDownCameraComponent");
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
}

void AElderCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// Init ability actor info for the Server
	InitAbilityActorInfo();
}

void AElderCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Init ability actor info for the Client
	InitAbilityActorInfo();
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
}

void AElderCharacter::InitAbilityActorInfo()
{
	AElderPlayerState* ElderPlayerState = GetPlayerState<AElderPlayerState>();
	check(ElderPlayerState);
	ElderPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(ElderPlayerState, this);
	AbilitySystemComponent = ElderPlayerState->GetAbilitySystemComponent();
	AttributeSet = ElderPlayerState->GetAttributeSet();
}

void AElderCharacter::MulticastLevelUpParticles_Implementation() const
{
}
