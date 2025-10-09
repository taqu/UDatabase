#include "ConvertSkeletalMeshDialog.h"

#include <Engine/AssetManager.h>
#include <Engine/SkeletalMesh.h>
#include <ContentBrowserModule.h>
#include <IContentBrowserSingleton.h>
#include <Widgets/Layout/SUniformGridPanel.h>
#include <AssetToolsModule.h>

#include "AnimToTextureBPLibrary.h"

#define LOCTEXT_NAMESPACE "AnimToTexture"

void FConvertSkeletalMeshDialog::Open(const FAssetData& AssetData)
{
	TSharedPtr<SConvertSkeletalMeshDialog> ConfirmDialog =
		SNew(SConvertSkeletalMeshDialog)
			.InitialAssetData(AssetData);

	if(ConfirmDialog->ShowModal() == EAppReturnType::Cancel) {
		return;
	}
	TObjectPtr<USkeletalMesh> SkeletalMesh = ConfirmDialog->GetSelectedDataAsset();
	if(nullptr == SkeletalMesh) {
		return;
	}
	int32 SelectedLod = ConfirmDialog->GetSelectedLod();
	SelectedLod = FMath::Clamp(SelectedLod, -1, SkeletalMesh->GetLODNum() - 1);

	FString StaticMeshName = ConfirmDialog->GetStaticMeshName().ToString();
	if(StaticMeshName.IsEmpty()) {
		return;
	}
	FString PackageName = SkeletalMesh->GetPackage()->GetPathName();
	PackageName = FPaths::Combine(FPaths::GetPath(MoveTemp(PackageName)), StaticMeshName);
	UAnimToTextureBPLibrary::ConvertSkeletalMeshToStaticMesh(SkeletalMesh, PackageName, SelectedLod);
}

void SConvertSkeletalMeshDialog::Construct(const FArguments& Args)
{
	{
		FString BaseName = FPaths::GetBaseFilename(Args._InitialAssetData.AssetName.ToString());
		StaticMeshName_ = FText::FromString(GetStaticMeshName(BaseName));
	}

	// clang-format off
    TSharedPtr <SVerticalBox> AssetPicker0 = SNew(SVerticalBox)
        + SVerticalBox::Slot().FillHeight(0.1f).FillHeight(0.1f)
        [
            SNew(STextBlock).Text(LOCTEXT("SkeletalMesh", "SkeletalMesh"))
        ];
	// clang-format on

	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

		FAssetPickerConfig AssetPickerConfig;
#if 5 <= ENGINE_MAJOR_VERSION && 1 <= ENGINE_MINOR_VERSION
		AssetPickerConfig.Filter.ClassPaths.Add(USkeletalMesh::StaticClass()->GetClassPathName());
#else
		AssetPickerConfig.Filter.ClassNames.Add(USkeletalMesh::StaticClass()->GetFName());
#endif
		AssetPickerConfig.SelectionMode = ESelectionMode::Single;

		AssetPickerConfig.GetCurrentSelectionDelegates.Add(&GetCurrentSelectionDelegate_AssetData);

		AssetPickerConfig.bAllowNullSelection = true;
		AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
		AssetPickerConfig.InitialAssetSelection = Args._InitialAssetData;
		AssetPickerConfig.ThumbnailScale = 1.f;
		AssetPickerConfig.OnAssetSelected.BindRaw(this, &SConvertSkeletalMeshDialog::OnAssetSelected);

		// clang-format off
        AssetPicker0->AddSlot().FillHeight(1).Padding(3)
		[
            ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
        ];
		// clang-format on
	}
	GetLodDropDown(LodOptions_, Args._InitialAssetData);

	// clang-format off
	SWindow::Construct(SWindow::FArguments()
		.Title(FText::FromString(TEXT("Convert Skeletal To Static")))
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		.ClientSize(FVector2D(600, 400))
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().Padding(2, 2, 2, 4).HAlign(HAlign_Fill).FillHeight(4)
		[
#if 5 <= ENGINE_MAJOR_VERSION && 1 <= ENGINE_MINOR_VERSION
			SNew(SBorder).BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
#else
			SNew(SBorder).BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
#endif

			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().FillHeight(1).Padding(3).HAlign(HAlign_Fill)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().FillHeight(2).Padding(3)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1).HAlign(HAlign_Fill).VAlign(VAlign_Fill)
						[
							AssetPicker0.ToSharedRef()
						]
					]
				]
			]
		]
	    + SVerticalBox::Slot().Padding(2, 2, 2, 2).MaxHeight(64.0f).FillHeight(1)
		[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 10, 0).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("StaticMeshName", "Static Mesh Name"))
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 10, 0).VAlign(VAlign_Center)
				[
					SAssignNew(EditStaticMeshName_, SEditableText)
					.Text(StaticMeshName_).OnTextCommitted(this, &SConvertSkeletalMeshDialog::OnStaticMeshNameChanged).MinDesiredWidth(250)
				]
		]
		+ SVerticalBox::Slot().MaxHeight(64.0f).Padding(1).VAlign(VAlign_Center).FillHeight(1)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 10, 0).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SelectLOD", "Select LOD to convert"))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 10, 0).VAlign(VAlign_Center)
			[
				SAssignNew(EditLod_, STextComboBox)
				.OptionsSource(&LodOptions_)
			]
		]
		+ SVerticalBox::Slot().MaxHeight(64.0f).HAlign(HAlign_Right).VAlign(VAlign_Bottom).FillHeight(1)
		[
			SNew(SUniformGridPanel)
#if 5 <= ENGINE_MAJOR_VERSION && 1 <= ENGINE_MINOR_VERSION
			.SlotPadding(FAppStyle::GetMargin("StandardDialog.SlotPadding"))
			.MinDesiredSlotWidth(FAppStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
			.MinDesiredSlotHeight(FAppStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
#else
			.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
			.MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
			.MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
#endif
			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("OK", "OK"))
				.HAlign(HAlign_Center)
#if 5 <= ENGINE_MAJOR_VERSION && 1 <= ENGINE_MINOR_VERSION
				.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
#else
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
#endif
				.OnClicked(this, &SConvertSkeletalMeshDialog::OnButtonClick, EAppReturnType::Ok)
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Cancel", "Cancel"))
				.HAlign(HAlign_Center)
#if 5 <= ENGINE_MAJOR_VERSION && 1 <= ENGINE_MINOR_VERSION
				.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
#else
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
#endif
				.OnClicked(this, &SConvertSkeletalMeshDialog::OnButtonClick, EAppReturnType::Cancel)
			]
		]
	]);
	// clang-format on

	EditLod_->SetSelectedItem(LodOptions_[1]);
}

EAppReturnType::Type SConvertSkeletalMeshDialog::ShowModal()
{
	GEditor->EditorAddModalWindow(SharedThis(this));
	return Result_;
}

TObjectPtr<USkeletalMesh> SConvertSkeletalMeshDialog::GetSelectedDataAsset() const
{
	TArray<FAssetData> SelectionArray = GetCurrentSelectionDelegate_AssetData.Execute();

	if(SelectionArray.Num() <= 0) {
		return nullptr;
	}
	for(int32 i = 0; i < SelectionArray.Num(); ++i) {
		SelectionArray[i].GetPackage()->FullyLoad();
		if(SelectionArray[i].IsAssetLoaded()) {
			USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(SelectionArray[i].GetAsset());
			if(nullptr != SkeletalMesh) {
				return SkeletalMesh;
			}
		}
	}
	return nullptr;
}

const FText& SConvertSkeletalMeshDialog::GetStaticMeshName() const
{
	return StaticMeshName_;
}

int32 SConvertSkeletalMeshDialog::GetSelectedLod() const
{
	int32 Count = 0;
	for(int32 i = 0; i < LodOptions_.Num(); ++i) {
		if(LodOptions_[i] == EditLod_->GetSelectedItem()) {
			return i - 1;
		}
	}
	return -1;
}

void SConvertSkeletalMeshDialog::OnAssetSelected(const FAssetData& AssetData)
{
	if(!AssetData.IsValid()) {
		return;
	}
	FString BaseName = FPaths::GetBaseFilename(AssetData.AssetName.ToString());
	FString StaicMeshName = GetStaticMeshName(BaseName);
	EditStaticMeshName_->SetText(FText::FromString(StaicMeshName));
}

void SConvertSkeletalMeshDialog::OnStaticMeshNameChanged(const FText& Name, ETextCommit::Type TextCommit)
{
	StaticMeshName_ = Name;
}

FReply SConvertSkeletalMeshDialog::OnButtonClick(EAppReturnType::Type ButtonID)
{
	Result_ = EAppReturnType::Cancel;
	if(EAppReturnType::Ok != ButtonID) {
		RequestDestroyWindow();
		return FReply::Handled();
	}
	RequestDestroyWindow();
	Result_ = EAppReturnType::Ok;
	return FReply::Handled();
}

FString SConvertSkeletalMeshDialog::GetStaticMeshName(const FString& Name)
{
	return ReplaceTop(Name, TEXT("SK_"), TEXT("SM_"));
}

FString SConvertSkeletalMeshDialog::ReplaceTop(const FString& Str, const TCHAR* From, const TCHAR* To)
{
	if(!Str.StartsWith(From)) {
		return Str;
	}
	int32 Length = TCString<TCHAR>::Strlen(From);
	FString Prefix(To);
	return Prefix + Str.Mid(Length);
}

void SConvertSkeletalMeshDialog::GetLodDropDown(TArray<TSharedPtr<FString>>& LodDropDown, const FAssetData& AssetData)
{
	LodDropDown.Add(MakeShared<FString>(TEXT("All")));
	static const int32 LodNum = 8;
	for(int32 i = 0; i < LodNum; ++i) {
		LodDropDown.Add(MakeShared<FString>(FString::FromInt(i)));
	}
}

int32 SConvertSkeletalMeshDialog::GetMaxLod(const FAssetData& AssetData)
{
	if(!AssetData.IsValid()) {
		return 0;
	}
	USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(AssetData.GetAsset());
	if(nullptr == SkeletalMesh) {
		return 0;
	}
	return SkeletalMesh->GetLODNum() - 1;
}
#undef LOCTEXT_NAMESPACE
