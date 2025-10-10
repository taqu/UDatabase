#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUDatabase, Log, All);

class UDatabaseManager;

class FDatabaseModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static UDatabaseManager* GetManager();

private:
    TObjectPtr<UDatabaseManager> DatabaseManager_ = nullptr;
};

