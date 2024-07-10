// Copyright Epic Games, Inc. All Rights Reserved.

#include "BPDiffer.h"
#include "BPDifferStyle.h"
#include "BPDifferCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

#include <Kismet/KismetSystemLibrary.h>
#include <IDesktopPlatform.h>
#include <DesktopPlatformModule.h>

static const FName BPDifferTabName("BPDiffer");

#define LOCTEXT_NAMESPACE "FBPDifferModule"

DEFINE_LOG_CATEGORY_STATIC(LogBPDiffer, Log, All);

void FBPDifferModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FBPDifferStyle::Initialize();
	FBPDifferStyle::ReloadTextures();

	FBPDifferCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FBPDifferCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FBPDifferModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBPDifferModule::RegisterMenus));
}

void FBPDifferModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FBPDifferStyle::Shutdown();

	FBPDifferCommands::Unregister();
}

// Get the object of the last opened asset
UObject* GetLastOpenedObject()
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();
	TMap<UObject*, IAssetEditorInstance*> OpenedAssetsMap;

	for (UObject* EditedAsset : EditedAssets)
	{
		OpenedAssetsMap.Add(EditedAsset, AssetEditorSubsystem->FindEditorForAsset(EditedAsset, false));
	}

	UObject* LastOpenedObject = nullptr;
	double MaxLastActivationTime = 0.0;

	for (auto& OpenedAssetPair : OpenedAssetsMap) {
		if (OpenedAssetPair.Value && OpenedAssetPair.Value->GetLastActivationTime() > MaxLastActivationTime)
		{
			MaxLastActivationTime = OpenedAssetPair.Value->GetLastActivationTime();
			LastOpenedObject = OpenedAssetPair.Key;
		}
	}

	return LastOpenedObject;
}

void FBPDifferModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	FString EditorPath = FPaths::Combine(FPlatformProcess::BaseDir(), TEXT("UnrealEditor.exe"));

	// Combine params
	FString Params = TEXT("");
	Params.Append(FPaths::GetProjectFilePath());
	Params.Append(TEXT(" -diff "));

	FString AbsoluteCurrentAssetPath = UKismetSystemLibrary::GetSystemPath(GetLastOpenedObject());
	Params.Append(AbsoluteCurrentAssetPath);

	// Open file dialog to select blueprint
	TArray<FString> AbsoluteOpenFileNames;
	FString ExtensionStr = TEXT("Blueprint|*.uasset");
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	DesktopPlatform->OpenFileDialog(nullptr, TEXT("Please select the blueprint you want to diff:"), FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), TEXT(""), *ExtensionStr, EFileDialogFlags::None, AbsoluteOpenFileNames);
	if (AbsoluteOpenFileNames.IsEmpty()) {
		UE_LOG(LogBPDiffer, Warning, TEXT("No Selected File! Canceling Diff Operation..."));
		return;
	}

	Params.Append(" ");
	Params.Append(AbsoluteOpenFileNames[0]);

	// Start blueprint diff
	UE_LOG(LogBPDiffer, Log, TEXT("Blueprint Diff Command: %s %s"), *EditorPath, *Params);
	FPlatformProcess::CreateProc(*EditorPath, *Params, true, false, false, nullptr, 0, nullptr, nullptr);
}

void FBPDifferModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("AssetEditor.DefaultToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FBPDifferCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBPDifferModule, BPDiffer)