#pragma once
#include <CoreMinimal.h>
#include <Engine/DataAsset.h>
#include <Widgets/SWindow.h>
#include <ContentBrowserDelegates.h>

#include "DatabaseDataAsset.h"

struct DATABASEEDITOR_API FDatabaseDialog
{
    static void Open(const FAssetData& DataAsset);
};


class DATABASEEDITOR_API SDatabaseDialog : public SWindow
{
public:
	SLATE_BEGIN_ARGS(SDatabaseDialog)
	{
	}
	SLATE_ARGUMENT(FText, Title)
		SLATE_ARGUMENT(FAssetData, InitialDataAsset)
		SLATE_END_ARGS()

		SDatabaseDialog()
	{
	}

	void Construct(const FArguments& InArgs);

	/** Displays the dialog in a blocking fashion */
	EAppReturnType::Type ShowModal();

	const FText& GetStaticMeshName() const;

	const FText& GetTextureBaseName() const;

	TObjectPtr<UDatabaseDataAsset> GetSelectedDataAsset() const;

	bool Filter(const FAssetData& AssetData);

	int32 ValidateDataAsset() const;

private:
	void OnStaticMeshNameChanged(const FText& Name, ETextCommit::Type TextCommit);
	void OnTextureBaseNameChanged(const FText& Name, ETextCommit::Type TextCommit);
	FReply OnClickBuild(EAppReturnType::Type ButtonID);
	FReply OnClickUpdateMaterials(EAppReturnType::Type ButtonID);
	FReply OnClickClose(EAppReturnType::Type ButtonID);
	void OnAssetSelected(const FAssetData& AssetData);
	//bool FindTextureBaseName(FString& Name, TObjectPtr<UDatabaseDataAsset> DataAsset) const;
	//static void GetBaseName(FString& Name, const FString& Src);
	//static FString ReplaceTop(const FString& Str, const TCHAR* From, const TCHAR* To);

	bool ValidatePackage();

	EAppReturnType::Type Result_ = EAppReturnType::Cancel;

	FText StaticMeshName_;
	FText TextureBaseName_;
	TSharedPtr<SEditableText> EditStaticMeshName_;
	TSharedPtr<SEditableText> EditTextureBaseName_;

	FGetCurrentSelectionDelegate GetCurrentSelectionDelegate_AssetData;
};

