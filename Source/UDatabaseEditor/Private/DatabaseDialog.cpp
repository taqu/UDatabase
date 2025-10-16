#include "DatabaseDialog.h"

#include <Engine/AssetManager.h>
#include <AssetRegistry/AssetData.h>
#include <ContentBrowserModule.h>
#include <IContentBrowserSingleton.h>
#include <Widgets/Layout/SUniformGridPanel.h>

#include <AssetToolsModule.h>

#if 5 <= ENGINE_MAJOR_VERSION && 1 <= ENGINE_MINOR_VERSION
#include <EditorStyleSet.h>
#endif

#include <Serialization/MemoryWriter.h>

#include "DatabaseEditor.h"
#include "DatabaseDataAsset.h"
#include "Database.h"
#include "DatabaseManager.h"

#define LOCTEXT_NAMESPACE "DatabaseEditor"

void FDatabaseDialog::Open(const FAssetData& DataAsset)
{
	TSharedPtr<SDatabaseDialog> Confirm =
        SNew(SDatabaseDialog)
			.Title(LOCTEXT("CreateDatabaseDialog", "Create Data Base Dialog"))
			.InitialDataAsset(DataAsset);

	if(Confirm->ShowModal() == EAppReturnType::Cancel) {
		return;
	}
}

void SDatabaseDialog::Construct(const FArguments& Args)
{
    ClearBeforeUpsert_ = true;

	// clang-format off
    TSharedPtr <SVerticalBox> AssetPicker0 = SNew(SVerticalBox)
        + SVerticalBox::Slot().FillHeight(0.1f).FillHeight(0.1f)
        [
            SNew(STextBlock).Text(LOCTEXT("DatabaseDataAsset", "Database Data Asset"))
        ];
	// clang-format on

	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

		FAssetPickerConfig AssetPickerConfig;
#if 5 <= ENGINE_MAJOR_VERSION && 1 <= ENGINE_MINOR_VERSION
		AssetPickerConfig.Filter.ClassPaths.Add(UDatabaseDataAsset::StaticClass()->GetClassPathName());
#else
        AssetPickerConfig.Filter.ClassNames.Add(UDatabaseDataAsset::StaticClass()->GetFName());
#endif
		AssetPickerConfig.SelectionMode = ESelectionMode::Single;

		AssetPickerConfig.GetCurrentSelectionDelegates.Add(&GetCurrentSelectionDelegate_AssetData);

		AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &SDatabaseDialog::Filter);
		AssetPickerConfig.bAllowNullSelection = true;
		AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
		AssetPickerConfig.InitialAssetSelection = Args._InitialDataAsset;
		AssetPickerConfig.ThumbnailScale = 1.f;
        AssetPickerConfig.OnAssetSelected.BindRaw(this, &SDatabaseDialog::OnAssetSelected);

		// clang-format off
        AssetPicker0->AddSlot().FillHeight(1).Padding(3)
		[
            ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
        ];
		// clang-format on
	}

	// clang-format off
	SWindow::Construct(SWindow::FArguments()
		.Title(Args._Title)
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		.ClientSize(FVector2D(1080, 720))
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().Padding(2, 2, 2, 4).FillHeight(4).HAlign(HAlign_Fill)
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
		+ SVerticalBox::Slot().MaxHeight(64.0f).Padding(3).FillHeight(1)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 10, 0).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ClearTable", "Clear table before upserting"))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 10, 0).VAlign(VAlign_Center)
			[
				SNew(SCheckBox)
				.ToolTipText(LOCTEXT("ClearTooltip", "Clear before upserting"))
				.IsChecked(ECheckBoxState::Checked)
				.OnCheckStateChanged(this, &SDatabaseDialog::OnCheckClearTable)
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
				.Text(LOCTEXT("Build", "Build"))
				.HAlign(HAlign_Center)
#if 5 <= ENGINE_MAJOR_VERSION && 1 <= ENGINE_MINOR_VERSION
				.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
#else
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
#endif
				.OnClicked(this, &SDatabaseDialog::OnClickBuild, EAppReturnType::Ok)
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Load", "Load"))
				.HAlign(HAlign_Center)
#if 5 <= ENGINE_MAJOR_VERSION && 1 <= ENGINE_MINOR_VERSION
				.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
#else
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
#endif
				.OnClicked(this, &SDatabaseDialog::OnClickLoad, EAppReturnType::Ok)
			]
			+ SUniformGridPanel::Slot(2, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Drop", "Drop"))
				.HAlign(HAlign_Center)
#if 5 <= ENGINE_MAJOR_VERSION && 1 <= ENGINE_MINOR_VERSION
				.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
#else
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
#endif
				.OnClicked(this, &SDatabaseDialog::OnClickDrop, EAppReturnType::Ok)
			]
			+ SUniformGridPanel::Slot(3, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Vaccum", "Vaccum"))
				.HAlign(HAlign_Center)
#if 5 <= ENGINE_MAJOR_VERSION && 1 <= ENGINE_MINOR_VERSION
				.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
#else
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
#endif
				.OnClicked(this, &SDatabaseDialog::OnClickVaccum, EAppReturnType::Ok)
			]
			+ SUniformGridPanel::Slot(4, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Close", "Close"))
				.HAlign(HAlign_Center)
#if 5 <= ENGINE_MAJOR_VERSION && 1 <= ENGINE_MINOR_VERSION
				.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
#else
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
#endif
				.OnClicked(this, &SDatabaseDialog::OnClickClose, EAppReturnType::Cancel)
			]
		]
	]);
	// clang-format on
	OnAssetSelected(Args._InitialDataAsset);
}

EAppReturnType::Type SDatabaseDialog::ShowModal()
{
	GEditor->EditorAddModalWindow(SharedThis(this));
	return Result_;
}

TObjectPtr<UDatabaseDataAsset> SDatabaseDialog::GetSelectedDataAsset() const
{
	TArray<FAssetData> SelectionArray = GetCurrentSelectionDelegate_AssetData.Execute();

	if(SelectionArray.Num() <= 0) {
		return nullptr;
	}
	for(int32 i = 0; i < SelectionArray.Num(); ++i) {
		SelectionArray[i].GetPackage()->FullyLoad();
		if(SelectionArray[i].IsAssetLoaded()) {
            UDatabaseDataAsset* DataAsset = Cast<UDatabaseDataAsset>(SelectionArray[i].GetAsset());
			if(nullptr != DataAsset) {
				return DataAsset;
			}
		}
	}
	return nullptr;
}

bool SDatabaseDialog::Filter(const FAssetData& AssetData)
{
	if(!AssetData.IsAssetLoaded()) {
		AssetData.GetPackage()->FullyLoad();
	}
	return false;
}

int32 SDatabaseDialog::ValidateDataAsset() const
{
    UDatabaseDataAsset* DataAsset = GetSelectedDataAsset();
	if(nullptr == DataAsset) {
		return 1;
	}
#if 0
		// Invalid Offsets or Normals Texture
		if ((!DataAsset->AutoSize) && (DataAsset->OverrideSize_Vert.GetMax() < 8)) return 4;
		// Profile has not Anims
		if ((DataAsset->Anims_Vert.Num() == 0) && (DataAsset->Anims_Bone.Num() == 0)) return 5;
		
		
		USkeleton* Skeleton = NULL;
		if (DataAsset->Anims_Vert.Num())
			if (Profile->Anims_Vert[0].SequenceRef != NULL) Skeleton = DataAsset->Anims_Vert[0].SequenceRef->GetSkeleton();
		if (DataAsset->Anims_Bone.Num())
			if (DataAsset->Anims_Bone[0].SequenceRef != NULL) Skeleton = DataAsset->Anims_Bone[0].SequenceRef->GetSkeleton();

		{
			for (int32 i = 0; i < Profile->Anims_Vert.Num(); i++)
			{
				// Invalid Sequence Ref
				if (DataAsset->Anims_Vert[i].SequenceRef == NULL) return 6;
				
				if (DataAsset->Anims_Vert[i].SequenceRef->GetSkeleton() != Skeleton)
				{
					// Anims have different Skeletons
					return 7;
				}
				if ((DataAsset->Anims_Vert[i].NumFrames < 1))
				{
					// Anim has Num Frames less than 1
					return 8;
				}
			}
		}
#endif
	return 0;
}

void SDatabaseDialog::OnCheckClearTable(ECheckBoxState NewCheckedState)
{
    ClearBeforeUpsert_ = NewCheckedState == ECheckBoxState::Checked;
}

FReply SDatabaseDialog::OnClickBuild(EAppReturnType::Type ButtonID)
{
    UDatabaseManager* DatabaseManager = FDatabaseModule::GetManager();
    if(nullptr == DatabaseManager) {
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Cannot get UDatabaseManager."));
        return FReply::Handled();
    }

    TObjectPtr<UDatabaseDataAsset> DatabaseDataAsset = GetSelectedDataAsset();
    if(nullptr == DatabaseDataAsset) {
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Needs to select UdatabaseDataAsset."));
		return FReply::Handled();
	}
    if(DatabaseDataAsset->DataTable.IsNull() || DatabaseDataAsset->DatabasePath.IsNone() || DatabaseDataAsset->TableName.IsNone()){
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Needs to set UdatabaseDataAsset properties."));
		return FReply::Handled();
	}
    FDatabaseHandle Database = DatabaseManager->Open(DatabaseDataAsset->DatabasePath, ESQLiteDatabaseOpenMode::ReadWriteCreate);
    if(!Database){
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Cannot open database %s."), *DatabaseDataAsset->DatabasePath.ToString());
		return FReply::Handled();
	}
    if(!Database.CreateIfNotExists(DatabaseDataAsset->TableName)) {
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Cannot open table %s."), *DatabaseDataAsset->TableName.ToString());
        DatabaseManager->Close(DatabaseDataAsset->DatabasePath);
        return FReply::Handled();
	}

	if(ClearBeforeUpsert_){
        Database.Clear(DatabaseDataAsset->TableName);
	}

    UDataTable* DataTable = DatabaseDataAsset->DataTable.LoadSynchronous();
    if(!DataTable || !DataTable->RowStruct) {
        DatabaseManager->Close(DatabaseDataAsset->DatabasePath);
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Cannot load UDataTable."));
		return FReply::Handled();
	}
    const TMap<FName, uint8*>& RowMap = DataTable->GetRowMap();
	{
        FDatabaseStatement Statement = Database.BeginUpsert(DatabaseDataAsset->TableName);
		if(!Statement){
            UE_LOG(LogUDatabaseEditor, Warning, TEXT("Cannot begin prepared statement."));
			return FReply::Handled();
		}
        TArray<uint8> Bytes;
        FMemoryWriter MemoryWriter(Bytes);
        for(auto&& RowIt = RowMap.CreateConstIterator(); RowIt; ++RowIt) {
            Bytes.Reset();
			MemoryWriter.Reset();
            FName RowName = RowIt.Key();
            uint8* RowData = RowIt.Value();
			Statement.Upsert(RowName, DataTable->RowStruct->GetStructureSize(), RowData);
        }
        Statement.Finalize();
    }
    DatabaseManager->Close(DatabaseDataAsset->DatabasePath);
	return FReply::Handled();
}

FReply SDatabaseDialog::OnClickLoad(EAppReturnType::Type ButtonID)
{
    UDatabaseManager* DatabaseManager = FDatabaseModule::GetManager();
    if(nullptr == DatabaseManager) {
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Cannot get UDatabaseManager."));
        return FReply::Handled();
    }

    TObjectPtr<UDatabaseDataAsset> DatabaseDataAsset = GetSelectedDataAsset();
    if(nullptr == DatabaseDataAsset) {
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Needs to select UdatabaseDataAsset."));
        return FReply::Handled();
    }
    if(DatabaseDataAsset->DataTable.IsNull() || DatabaseDataAsset->DatabasePath.IsNone() || DatabaseDataAsset->TableName.IsNone()) {
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Needs to set UdatabaseDataAsset properties."));
        return FReply::Handled();
    }
    FDatabaseHandle Database = DatabaseManager->Open(DatabaseDataAsset->DatabasePath, ESQLiteDatabaseOpenMode::ReadWriteCreate);
    if(!Database) {
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Cannot open database %s."), *DatabaseDataAsset->DatabasePath.ToString());
        DatabaseManager->Close(DatabaseDataAsset->DatabasePath);
        return FReply::Handled();
    }
    if(!Database.Exists(DatabaseDataAsset->TableName)) {
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("The table %s doesn't exists."), *DatabaseDataAsset->TableName.ToString());
        DatabaseManager->Close(DatabaseDataAsset->DatabasePath);
        return FReply::Handled();
    }

    UDataTable* DataTable = DatabaseDataAsset->DataTable.LoadSynchronous();
    if(!DataTable || !DataTable->RowStruct) {
        DatabaseManager->Close(DatabaseDataAsset->DatabasePath);
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Cannot load UDataTable."));
        return FReply::Handled();
    }
    {
        FDatabaseStatement Statement = Database.BeginGetAll(DatabaseDataAsset->TableName);
        if(!Statement) {
            UE_LOG(LogUDatabaseEditor, Warning, TEXT("Cannot begin prepared statement."));
            return FReply::Handled();
        }
        DataTable->EmptyTable();
		FString Key;
        TArray<uint8> Bytes;
        UScriptStruct& EmptyUsingStruct = *DataTable->RowStruct;
		for(;;){
			Key.Empty();
            Bytes.Reset();
            FDatabaseStatement::Result Result = Statement.GetOne(Key, Bytes);
            if(FDatabaseStatement::Result::Error == Result){
                UE_LOG(LogUDatabaseEditor, Warning, TEXT("Fail to get row."));
				break;
			}
            if(FDatabaseStatement::Result::Done == Result) {
				break;
			}
            if(Bytes.Num() <= 0) {
                continue;
            }
			uint8* RowData = (uint8*)FMemory::Malloc(DataTable->RowStruct->GetStructureSize());
            EmptyUsingStruct.InitializeStruct(RowData);
            EmptyUsingStruct.CopyScriptStruct(RowData, &Bytes[0]);
			DataTable->AddRow(FName(Key), (const FTableRowBase&)Bytes[0]);
        }
        Statement.Finalize();
    }
    DatabaseManager->Close(DatabaseDataAsset->DatabasePath);
    DataTable->Modify();
    return FReply::Handled();
}

FReply SDatabaseDialog::OnClickDrop(EAppReturnType::Type ButtonID)
{
    UDatabaseManager* DatabaseManager = FDatabaseModule::GetManager();
    if(nullptr == DatabaseManager) {
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Cannot get UDatabaseManager."));
        return FReply::Handled();
    }

    TObjectPtr<UDatabaseDataAsset> DatabaseDataAsset = GetSelectedDataAsset();
    if(nullptr == DatabaseDataAsset) {
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Needs to select UdatabaseDataAsset."));
        return FReply::Handled();
    }
    if(DatabaseDataAsset->DataTable.IsNull() || DatabaseDataAsset->DatabasePath.IsNone() || DatabaseDataAsset->TableName.IsNone()) {
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Needs to set UdatabaseDataAsset properties."));
        return FReply::Handled();
    }
    FDatabaseHandle Database = DatabaseManager->Open(DatabaseDataAsset->DatabasePath, ESQLiteDatabaseOpenMode::ReadWriteCreate);
    if(!Database) {
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Cannot open database %s."), *DatabaseDataAsset->DatabasePath.ToString());
        return FReply::Handled();
    }
    if(!Database.DropTable(DatabaseDataAsset->TableName)){
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Fail to drop table %s."), *DatabaseDataAsset->TableName.ToString());
    }
    DatabaseManager->Close(DatabaseDataAsset->DatabasePath);
    return FReply::Handled();
}

FReply SDatabaseDialog::OnClickVaccum(EAppReturnType::Type ButtonID)
{
    UDatabaseManager* DatabaseManager = FDatabaseModule::GetManager();
    if(nullptr == DatabaseManager) {
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Cannot get UDatabaseManager."));
        return FReply::Handled();
    }

    TObjectPtr<UDatabaseDataAsset> DatabaseDataAsset = GetSelectedDataAsset();
    if(nullptr == DatabaseDataAsset) {
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Needs to select UdatabaseDataAsset."));
        return FReply::Handled();
    }
    if(DatabaseDataAsset->DataTable.IsNull() || DatabaseDataAsset->DatabasePath.IsNone() || DatabaseDataAsset->TableName.IsNone()) {
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Needs to set UdatabaseDataAsset properties."));
        return FReply::Handled();
    }
    FDatabaseHandle Database = DatabaseManager->Open(DatabaseDataAsset->DatabasePath, ESQLiteDatabaseOpenMode::ReadWriteCreate);
    if(!Database) {
        UE_LOG(LogUDatabaseEditor, Warning, TEXT("Cannot open database %s."), *DatabaseDataAsset->DatabasePath.ToString());
        return FReply::Handled();
    }
	Database.Vacuum();
    DatabaseManager->Close(DatabaseDataAsset->DatabasePath);
    return FReply::Handled();
}

FReply SDatabaseDialog::OnClickClose(EAppReturnType::Type ButtonID)
{
	Result_ = EAppReturnType::Cancel;
	RequestDestroyWindow();
	return FReply::Handled();
}

void SDatabaseDialog::OnAssetSelected(const FAssetData& AssetData)
{
	if(!AssetData.IsValid()){
		return;
	}
	AssetData.GetPackage()->FullyLoad();
	if(!AssetData.IsAssetLoaded()) {
		return;
	}
	#if 0
	UAnimToTextureDataAsset* DataAsset = Cast<UAnimToTextureDataAsset>(AssetData.GetAsset());
	if(nullptr == DataAsset) {
		return;
	}
	if(DataAsset->StaticMesh.IsValid()) {
		FString StaticMeshName = DataAsset->StaticMesh.GetAssetName();
		EditStaticMeshName_->SetText(FText::FromString(StaticMeshName));

	} else if(DataAsset->SkeletalMesh.IsValid()) {
		FString StaticMeshName = DataAsset->SkeletalMesh.GetAssetName();
		StaticMeshName = ReplaceTop(StaticMeshName, TEXT("SK_"), TEXT("VA_"));
		EditStaticMeshName_->SetText(FText::FromString(StaticMeshName));
	}

	FString TextureBaseName;
	if(FindTextureBaseName(TextureBaseName, DataAsset)) {
		EditTextureBaseName_->SetText(FText::FromString(TextureBaseName));

	} else if(DataAsset->SkeletalMesh.IsValid()) {
		TextureBaseName = DataAsset->SkeletalMesh.GetAssetName();
		TextureBaseName = ReplaceTop(TextureBaseName, TEXT("SK_"), TEXT("T_"));
		EditTextureBaseName_->SetText(FText::FromString(TextureBaseName));

	} else if(DataAsset->StaticMesh.IsValid()) {
		TextureBaseName = DataAsset->StaticMesh.GetAssetName();
		TextureBaseName = ReplaceTop(TextureBaseName, TEXT("SM_"), TEXT("T_"));
		TextureBaseName = ReplaceTop(TextureBaseName, TEXT("VA_"), TEXT("T_"));
		EditTextureBaseName_->SetText(FText::FromString(TextureBaseName));
	}
	#endif
}

bool SDatabaseDialog::ValidatePackage()
{
#if 0
	TObjectPtr<UAnimToTextureDataAsset> DataAsset = GetSelectedDataAsset();

    FText Reason;
	FString Path = DataAsset->GetPathName();

    if(!FPackageName::IsValidLongPackageName(Path, false, &Reason)
       || !FName(*AssetName_.ToString()).IsValidObjectName(Reason)) {
        FMessageDialog::Open(EAppMsgType::Ok, Reason);
        return false;
    }

	FString PackagePath = AssetPath_.ToString() + "/" + AssetName_.ToString() + "." + AssetName_.ToString();
    if(FPackageName::DoesPackageExist(GetFullAssetPath().ToString()) || FindObject<UObject>(nullptr, *PackagePath) != nullptr) {
        if(nullptr != GetSelectedProfile()) {
            if(GetSelectedProfile()->StaticMesh == FindObject<UObject>(nullptr, *PackagePath)) {
                return true;
            }
        }
        FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("AssetAlreadyExists", "Asset {0} already exists."), GetFullAssetPath()));
        return false;
    }
#endif
	return true;
}

#undef LOCTEXT_NAMESPACE
