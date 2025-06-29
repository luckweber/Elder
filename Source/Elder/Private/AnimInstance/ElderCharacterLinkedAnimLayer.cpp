// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstance/ElderCharacterLinkedAnimLayer.h"

#include "AnimInstance/ElderCharacterAnimInstance.h"

UElderCharacterAnimInstance* UElderCharacterLinkedAnimLayer::GetElderAnimInstance() const
{
	return Cast<UElderCharacterAnimInstance>(GetOwningComponent()->GetAnimInstance());
}
