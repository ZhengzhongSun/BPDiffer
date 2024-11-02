// Copyright Epic Games, Inc. All Rights Reserved.

#include "BPDifferStyle.h"
#include "BPDiffer.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Brushes/SlateImageBrush.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FBPDifferStyle::StyleInstance = nullptr;

void FBPDifferStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FBPDifferStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FBPDifferStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("BPDifferStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef<FSlateStyleSet> FBPDifferStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("BPDifferStyle"));

	// Set content root to the plugin's Resources directory
	FString PluginResourcesDir = IPluginManager::Get().FindPlugin("BPDiffer")->GetBaseDir() / TEXT("Resources");

	// Construct the full path to the image file in the plugin's Resources folder
	FString ImagePath = PluginResourcesDir / TEXT("Icon128.png"); // Ensure the file format matches your texture

	// Create a new FSlateImageBrush with the external file path and image size
	FSlateBrush* NewBrush = new FSlateImageBrush(ImagePath, Icon20x20);

	// Register the brush in the style set with the desired key
	Style->Set("BPDiffer.PluginAction", NewBrush);

	return Style;
}


void FBPDifferStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FBPDifferStyle::Get()
{
	return *StyleInstance;
}
