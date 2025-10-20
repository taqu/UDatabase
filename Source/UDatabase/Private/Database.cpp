#include "Database.h"
#include "DatabaseManager.h"

THIRD_PARTY_INCLUDES_START
#include <sqlite/sqlite3.h>
THIRD_PARTY_INCLUDES_END

DEFINE_LOG_CATEGORY(LogUDatabase)
#define LOCTEXT_NAMESPACE "DatabaseModule"
void FDatabaseModule::StartupModule()
{
    if(!DatabaseManager_) {
        DatabaseManager_ = new FDatabaseManager();
    }
}

void FDatabaseModule::ShutdownModule()
{
    if(DatabaseManager_) {
        delete DatabaseManager_;
        DatabaseManager_ = nullptr;
    }
}

FDatabaseManager* FDatabaseModule::GetManager()
{
    FDatabaseModule* DatabaseModule = FModuleManager::Get().GetModulePtr<FDatabaseModule>("UDatabase");
    return DatabaseModule ? DatabaseModule->DatabaseManager_ : nullptr;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDatabaseModule, UDatabase)
