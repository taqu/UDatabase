#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUDatabase, Log, All);

class FDatabaseManager;

class UDATABASE_API FDatabaseModule: public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FDatabaseManager* GetManager();

private:
    TObjectPtr<FDatabaseManager> DatabaseManager_ = nullptr;
};

