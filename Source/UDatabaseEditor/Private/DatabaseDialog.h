#pragma once
#include <CoreMinimal.h>
#include <Engine/DataAsset.h>
#include <Widgets/SWindow.h>
#include <ContentBrowserDelegates.h>

#include "DatabaseDataAsset.h"

struct UDATABASEEDITOR_API FDatabaseDialog
{
    static void Open(const FAssetData& DataAsset);
};


class UDATABASEEDITOR_API SDatabaseDialog : public SWindow
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
    void OnCheckClearTable(ECheckBoxState NewCheckedState);
	FReply OnClickBuild(EAppReturnType::Type ButtonID);
	FReply OnClickLoad(EAppReturnType::Type ButtonID);
    FReply OnClickDrop(EAppReturnType::Type ButtonID);
    FReply OnClickVaccum(EAppReturnType::Type ButtonID);
	FReply OnClickPrint(EAppReturnType::Type ButtonID);
	FReply OnClickClose(EAppReturnType::Type ButtonID);
	void OnAssetSelected(const FAssetData& AssetData);

	bool ValidatePackage();

	EAppReturnType::Type Result_ = EAppReturnType::Cancel;
	bool ClearBeforeUpsert_;

	TSharedPtr<SEditableText> EditStaticMeshName_;
	TSharedPtr<SEditableText> EditTextureBaseName_;

	FGetCurrentSelectionDelegate GetCurrentSelectionDelegate_AssetData;
};

