// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "BPDifferStyle.h"

class FBPDifferCommands : public TCommands<FBPDifferCommands>
{
public:

	FBPDifferCommands()
		: TCommands<FBPDifferCommands>(TEXT("BPDiffer"), NSLOCTEXT("Contexts", "BPDiffer", "BPDiffer Plugin"), NAME_None, FBPDifferStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
