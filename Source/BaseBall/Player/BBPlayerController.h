#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BBPlayerController.generated.h"

class UBBChatInput;
class UUserWidget;

UCLASS()
class BASEBALL_API ABBPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABBPlayerController();
	
	virtual void BeginPlay() override;

	void SetChatMessageString(const FString& InChatMessageString);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void PrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Client, Reliable)
	void ClientRPCPrintChatMessageString(const FString& InChatMessageString);

private:
	UFUNCTION(Server, Reliable)
	void ServerRPCSendChatMessage(const FString& InChatMessage);

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UBBChatInput> ChatInputWidgetClass;

	UPROPERTY()
	TObjectPtr<UBBChatInput> ChatInputWidgetInstance;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> NotificationTextWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> NotificationTextWidgetInstance;
	
public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	FText NotificationText;
};