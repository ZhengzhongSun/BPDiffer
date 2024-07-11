// Copyright Epic Games, Inc. All Rights Reserved.

#include "BPDiffer.h"
#include "BPDifferStyle.h"
#include "BPDifferCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

#include <IDesktopPlatform.h>
#include <DesktopPlatformModule.h>
#include <IAssetTools.h>
#include <AssetToolsModule.h>

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
static UObject* GetLastOpenedObject()
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

static UObject* LoadDiffAsset(FString SrcFilePath)
{
	if (SrcFilePath.IsEmpty())
	{
		return nullptr;
	}

	UObject* AssetObj = nullptr;
	FString DestFilePath = FPaths::Combine(*FPaths::DiffDir(), FPaths::GetCleanFilename(SrcFilePath));

	// UE cannot open files with certain special characters in them (like 
	// the # symbol), so we make a copy of the file with a more UE digestible 
	// path (since this may be a perforce temp file)
	IFileManager::Get().Copy(*DestFilePath, *SrcFilePath);
	if (UPackage* AssetPkg = LoadPackage(/*Outer =*/nullptr, *DestFilePath, LOAD_None))
	{
		AssetObj = AssetPkg->FindAssetInPackage();
	}

	return AssetObj;
}

void FBPDifferModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	// Open file dialog to select blueprint asset
	TArray<FString> AbsoluteOpenFileNames;
	FString ExtensionStr = TEXT("Blueprint|*.uasset");
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	DesktopPlatform->OpenFileDialog(nullptr, TEXT("Please select the blueprint you want to diff:"), FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), TEXT(""), *ExtensionStr, EFileDialogFlags::None, AbsoluteOpenFileNames);
	if (AbsoluteOpenFileNames.IsEmpty()) {
		UE_LOG(LogBPDiffer, Warning, TEXT("No Selected File! Canceling Diff Operation..."));
		return;
	}

	// Load select asset
	UObject* NewAsset = LoadDiffAsset(AbsoluteOpenFileNames[0]);
	if (NewAsset == nullptr)
	{
		FText DialogText = FText::Format(
			LOCTEXT("AssetLoadError", "Failed to load select asset in {0}."),
			FText::FromString(AbsoluteOpenFileNames[0])
		);
		UE_LOG(LogBPDiffer, Error, TEXT("%s"), *DialogText.ToString());
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		return;
	}

	UObject* OldAsset = GetLastOpenedObject();

	// Verify asset types
	if (OldAsset->GetClass() != NewAsset->GetClass())
	{
		FText DialogText = FText::Format(
			LOCTEXT("TypeMismatchError", "{0}"),
			FText::FromString(TEXT("Cannot compare files of different asset types."))
		);
		UE_LOG(LogBPDiffer, Error, TEXT("%s"), *DialogText.ToString());
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);		
		return;
	}

	// Start blueprint diff
	UE_LOG(LogBPDiffer, Log, TEXT("Execute Blueprint Diff Command..."));
	IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.DiffAssets(OldAsset, NewAsset, FRevisionInfo::InvalidRevision(), FRevisionInfo::InvalidRevision());
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