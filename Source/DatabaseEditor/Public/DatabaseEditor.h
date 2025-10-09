#pragma once

#include "Modules/ModuleManager.h"
#include <Delegates/IDelegateInstance.h>

struct FAssetData;

class FDatabaseEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void OnModuleLoaded(FName Name, EModuleChangeReason ModuleChangeReason);

	static TSharedRef<FExtender> GetContentBrowserContextMenuExtender(const TArray<FAssetData>& SelectedAssets);
	static void HandleAddDataTableExtenderToMenu(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets);
	static void HandleAddDataAssetExtenderToMenu(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets);

	FDelegateHandle ModuleLoadedDelegateHandle_;
	FDelegateHandle DataTableEditorExtenderHandle_;
};

