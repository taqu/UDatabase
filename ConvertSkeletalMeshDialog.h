#pragma once
#include <CoreMinimal.h>
#include <AssetRegistry/AssetData.h>
#include <Widgets/SWindow.h>
#include <ContentBrowserDelegates.h>
#include <Widgets/Input/STextComboBox.h>

struct ANIMTOTEXTUREEDITOR_API FConvertSkeletalMeshDialog
{
    static void Open(const FAssetData& AssetData);
};

class ANIMTOTEXTUREEDITOR_API SConvertSkeletalMeshDialog : public SWindow
{
public:
	SLATE_BEGIN_ARGS(SConvertSkeletalMeshDialog)
	{
	}
		SLATE_ARGUMENT(FAssetData, InitialAssetData)
	SLATE_END_ARGS()
	SConvertSkeletalMeshDialog()
	{
	}

	void Construct(const FArguments& InArgs);

	/** Displays the dialog in a blocking fashion */
	EAppReturnType::Type ShowModal();

	TObjectPtr<USkeletalMesh> GetSelectedDataAsset() const;
	const FText& GetStaticMeshName() const;
	int32 GetSelectedLod() const;
private:
	void OnAssetSelected(const FAssetData& AssetData);
	void OnStaticMeshNameChanged(const FText& Name, ETextCommit::Type TextCommit);
	FReply OnButtonClick(EAppReturnType::Type ButtonID);

	static FString GetStaticMeshName(const FString& Name);
	static FString ReplaceTop(const FString& Str, const TCHAR* From, const TCHAR* To);
	static void GetLodDropDown(TArray<TSharedPtr<FString>>& LodDropDown, const FAssetData& AssetData);
	static int32 GetMaxLod(const FAssetData& AssetData);

	EAppReturnType::Type Result_ = EAppReturnType::Cancel;
	FText StaticMeshName_;
	TSharedPtr<SEditableText> EditStaticMeshName_;
	TSharedPtr<STextComboBox> EditLod_;

	TArray<TSharedPtr<FString>> LodOptions_;

	FGetCurrentSelectionDelegate GetCurrentSelectionDelegate_AssetData;
};

