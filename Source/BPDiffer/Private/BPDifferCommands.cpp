// Copyright Epic Games, Inc. All Rights Reserved.

#include "BPDifferCommands.h"

#define LOCTEXT_NAMESPACE "FBPDifferModule"

void FBPDifferCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "BPDiffer", "Execute BPDiffer action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
