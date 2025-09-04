#include "Player/BBPlayerController.h"
#include "BaseBall.h"
#include "UI/BBChatInput.h"
#include "Kismet/GameplayStatics.h"
#include "Game/BBGameModeBase.h"
#include "Player/BBPlayerState.h"
#include "Net/UnrealNetwork.h"

ABBPlayerController::ABBPlayerController()
{
	bReplicates = true;
}

void ABBPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);
	bShowMouseCursor = true;

	if (IsValid(ChatInputWidgetClass) == true)
	{
		ChatInputWidgetInstance = CreateWidget<UBBChatInput>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance) == true)
		{
			ChatInputWidgetInstance->AddToViewport();
		}
	}

	if (IsValid(NotificationTextWidgetClass) == true)
	{
		NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance) == true)
		{
			NotificationTextWidgetInstance->AddToViewport();
		}
	}
}

void ABBPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	if (IsLocalController() == true)
	{
		ServerRPCSendChatMessage(InChatMessageString);
	}
}

void ABBPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, InChatMessageString);
}

void ABBPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, InChatMessageString);
}

void ABBPlayerController::ServerRPCSendChatMessage_Implementation(const FString& InChatMessage)
{
	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM) == true)
	{
		ABBGameModeBase* BBGM = Cast<ABBGameModeBase>(GM);
		if (IsValid(BBGM) == true)
		{
			BBGM->ProcessPlayerChat(this, InChatMessage);
		}
	}
}

void ABBPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
}