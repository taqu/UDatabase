#include "Database.h"
#include "DatabaseManager.h"

DEFINE_LOG_CATEGORY(LogUDatabase)
#define LOCTEXT_NAMESPACE "DatabaseModule"

void FDatabaseModule::StartupModule()
{
    DatabaseManager_ = NewObject<UDatabaseManager>();
    if(DatabaseManager_){
        DatabaseManager_->AddToRoot();
    }
}

void FDatabaseModule::ShutdownModule()
{
    if(DatabaseManager_){
        DatabaseManager_->RemoveFromRoot();
        DatabaseManager_ = nullptr;
    }
}

UDatabaseManager* FDatabaseModule::GetManager()
{
    FDatabaseModule* DatabaseModule = FModuleManager::Get().GetModulePtr<FDatabaseModule>("Database");
    return DatabaseModule ? DatabaseModule->DatabaseManager_ : nullptr; 
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDatabaseModule, Database)
