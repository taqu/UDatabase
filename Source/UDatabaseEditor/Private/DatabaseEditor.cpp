#include "DatabaseEditor.h"
#include <IPersonaToolkit.h>
#include <ContentBrowserModule.h>
#include <Styling/SlateTypes.h>

#include "DatabaseDataAsset.h"
#include "DataTableEditorModule.h"
#include "DatabaseDialog.h"

#define LOCTEXT_NAMESPACE "DatabaseEditor"

void FDatabaseEditorModule::StartupModule()
{
	ModuleLoadedDelegateHandle_ = FModuleManager::Get().OnModulesChanged().AddRaw(this, &FDatabaseEditorModule::OnModuleLoaded);
}

void FDatabaseEditorModule::ShutdownModule()
{
	FDataTableEditorModule* DataTableEditorModule = FModuleManager::Get().GetModulePtr<FDataTableEditorModule>("DataTableEditor");
	if (DataTableEditorModule) {
		//SkeletalMeshEditorModule->GetAllSkeletalMeshEditorToolbarExtenders().RemoveAll([SkeletalMeshEditorExtenderHandle = SkeletalMeshEditorExtenderHandle_](const ISkeletalMeshEditorModule::FSkeletalMeshEditorToolbarExtender& Delegate) { return Delegate.GetHandle() == SkeletalMeshEditorExtenderHandle; });
	}

	if (ModuleLoadedDelegateHandle_.IsValid()) {
		FModuleManager::Get().OnModulesChanged().Remove(ModuleLoadedDelegateHandle_);
		ModuleLoadedDelegateHandle_.Reset();
	}
}

void FDatabaseEditorModule::OnModuleLoaded(FName Name, EModuleChangeReason ModuleChangeReason)
{
	if (ModuleChangeReason != EModuleChangeReason::ModuleLoaded) {
		return;
	}
#if 0
	if ("SkeletalMeshEditor" == Name) {
		ISkeletalMeshEditorModule& SkeletalMeshEditorModule = FModuleManager::Get().LoadModuleChecked<ISkeletalMeshEditorModule>(Name);
		TArray<ISkeletalMeshEditorModule::FSkeletalMeshEditorToolbarExtender>& ToolbarExtenders = SkeletalMeshEditorModule.GetAllSkeletalMeshEditorToolbarExtenders();

		ToolbarExtenders.Add(
			ISkeletalMeshEditorModule::FSkeletalMeshEditorToolbarExtender::CreateStatic(&FAnimToTextureEditorModule::GetSkeletalMeshEditorToolbarExtender));
		SkeletalMeshEditorExtenderHandle_ = ToolbarExtenders.Last().GetHandle();
	}
#endif

	if ("ContentBrowser" == Name) {
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuAssetExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
		CBMenuAssetExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&FDatabaseEditorModule::GetContentBrowserContextMenuExtender));
	}
}

#if 0
TSharedRef<FExtender> FDatabaseEditorModule::GetSkeletalMeshEditorToolbarExtender(const TSharedRef<FUICommandList> CommandList, TSharedRef<ISkeletalMeshEditor> SkeletalMeshEditor)
{
	TSharedRef<FExtender> Extender = MakeShareable(new FExtender);
	UDebugSkelMeshComponent* MeshComponent = SkeletalMeshEditor->GetPersonaToolkit()->GetPreviewMeshComponent();
	Extender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		CommandList,
		FToolBarExtensionDelegate::CreateRaw(this, &FAnimToTextureEditorModule::HandleAddSkeletalMeshActionExtenderToToolbar, MeshComponent)
	);
	return Extender;
}
#endif

TSharedRef<FExtender> FDatabaseEditorModule::GetContentBrowserContextMenuExtender(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();
	for (const FAssetData& AssetData : SelectedAssets) {
        if(AssetData.GetClass()  && !AssetData.IsRedirector() && AssetData.AssetClassPath.GetAssetName() != NAME_Class && !(AssetData.PackageFlags & PKG_FilterEditorOnly)) {
			if (AssetData.GetClass()->IsChildOf(USkeletalMesh::StaticClass())) {
				Extender->AddMenuExtension(
					"GetAssetActions",
					EExtensionHook::After,
					nullptr,
					FMenuExtensionDelegate::CreateStatic(&FDatabaseEditorModule::HandleAddDataTableExtenderToMenu, SelectedAssets));
			}
			else if (AssetData.GetClass()->IsChildOf(UDatabaseDataAsset::StaticClass())) {
				Extender->AddMenuExtension(
					"GetAssetActions",
					EExtensionHook::After,
					nullptr,
					FMenuExtensionDelegate::CreateStatic(&FDatabaseEditorModule::HandleAddDataAssetExtenderToMenu, SelectedAssets));
			}
		}
	}
	return Extender;
}

#if 0
void FDatabaseEditorModule::HandleAddSkeletalMeshActionExtenderToToolbar(FToolBarBuilder& ParentToolbarBuilder, UDebugSkelMeshComponent* MeshComponent)
{
	ParentToolbarBuilder.AddToolBarButton(
		FUIAction(FExecuteAction::CreateLambda([this, MeshComponent]() {
			FAnimToTextureDialog::OpenBakeDialog(MeshComponent);
			})),
		NAME_None,
		LOCTEXT("BakeVAT", "Bake VAT"),
		LOCTEXT("BakeVATTooltip", "Bake animation to a texture as a vertex animation"),
		FSlateIcon("EditorStyle", "Persona.TogglePreviewAsset", "Persona.TogglePreviewAsset.Small")
	);
}
#endif

void FDatabaseEditorModule::HandleAddDataTableExtenderToMenu(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets)
{
	//MenuBuilder.BeginSection("CustomSection", LOCTEXT("CustomSection", "Custom Section"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ConvertToStatic", "Convert To Static Mesh"),
			LOCTEXT("ConvertToStaticTooltip", "Convert skeletal mesh to static mesh"),
			FSlateIcon("EditorStyle", "Persona.TogglePreviewAsset", "Persona.TogglePreviewAsset.Small"),
			FUIAction(FExecuteAction::CreateLambda([SelectedAssets]() {
				if (SelectedAssets.Num() <= 0 || !SelectedAssets[0].IsValid()) {
					return;
				}
				//FConvertSkeletalMeshDialog::Open(SelectedAssets[0]);
				})),
			NAME_None,
			EUserInterfaceActionType::Button);
	}
	//MenuBuilder.EndSection();
}

void FDatabaseEditorModule::HandleAddDataAssetExtenderToMenu(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets)
{
	//MenuBuilder.BeginSection("CustomSection", LOCTEXT("CustomSection", "Custom Section"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("BuildTable", "Build Table"),
			LOCTEXT("BuildTableTooltip", "Build a table to database"),
			FSlateIcon("EditorStyle", "Persona.TogglePreviewAsset", "Persona.TogglePreviewAsset.Small"),
			FUIAction(FExecuteAction::CreateLambda([SelectedAssets]() {
				if (SelectedAssets.Num() <= 0 || !SelectedAssets[0].IsValid()) {
					return;
				}
				FDatabaseDialog::Open(SelectedAssets[0]);
				})),
			NAME_None,
			EUserInterfaceActionType::Button);
	}
	//MenuBuilder.EndSection();
}
#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDatabaseEditorModule, DatabaseEditor)