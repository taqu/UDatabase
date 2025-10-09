#pragma once

#include "Modules/ModuleManager.h"

class FDatabaseModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

