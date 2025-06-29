// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstance/ElderCharacterBaseAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "Character/ElderCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

void UElderCharacterBaseAnimInstance::NativeInitializeAnimation()
{
	OwningCharacterBase = Cast<AElderCharacterBase>(TryGetPawnOwner());

	if (OwningCharacterBase)
	{
		OwningMovementComponent = OwningCharacterBase->GetCharacterMovement();
	}
}

void UElderCharacterBaseAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	if (!OwningCharacterBase || !OwningMovementComponent)
	{
		return;
	}

	GroundSpeed = OwningCharacterBase->GetVelocity().Size2D();

	bHasAcceleration = OwningMovementComponent->GetCurrentAcceleration().SizeSquared2D() > 0.f;
	
	LocomotionDirection = UKismetAnimationLibrary::CalculateDirection(OwningCharacterBase->GetVelocity(), OwningCharacterBase->GetActorRotation());
}
