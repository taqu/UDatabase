#include "Database.h"
#include "DatabaseManager.h"

THIRD_PARTY_INCLUDES_START
#include <sqlite/sqlite3.h>
THIRD_PARTY_INCLUDES_END

DEFINE_LOG_CATEGORY(LogUDatabase)
#define LOCTEXT_NAMESPACE "DatabaseModule"

void FDatabaseModule::StartupModule()
{
    DatabaseManager_ = NewObject<UDatabaseManager>();
    if(DatabaseManager_){
        DatabaseManager_->AddToRoot();
    }

    struct Person
    {
        char id_[8];
        char name_[64];
        int32_t age_;
    };

    Person people[3] = {
        {"0", "John", 32},
        {"1", "Bob", 31},
        {"2", "Alice", 20},
    };

    const char* name = "test.db";

    sqlite3* db = nullptr;

    int result = sqlite3_open_v2(name, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if(SQLITE_OK != result) {
        return;
    }
#if 0
    result = sqlite3_key(db, "testkey", sizeof("testkey")-1);
    if(SQLITE_OK != result){
        printf("faile to set key\n");
        return 0;
    }
#endif
    result = sqlite3_exec(db, "DROP TABLE IF EXISTS people;", nullptr, nullptr, nullptr);
    if(SQLITE_OK != result) {
        sqlite3_close(db);
        return;
    }

    result = sqlite3_exec(db, "CREATE TABLE people(id TEXT PRIMARY KEY, content BLOB);", nullptr, nullptr, nullptr);
    if(SQLITE_OK != result) {
        sqlite3_close(db);
        return;
    }

    result = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    if(SQLITE_OK != result) {
        sqlite3_close(db);
        return;
    }

    sqlite3_stmt* insert_stmt = nullptr;
    result = sqlite3_prepare_v2(db, "INSERT INTO people (id,content) VALUES (?, ?) ON CONFLICT(id) DO UPDATE SET content = excluded.content;",
                                -1,
                                &insert_stmt,
                                nullptr);
    if(SQLITE_OK != result) {
        sqlite3_close(db);
        return;
    }

    for(int32_t i = 0; i < 3; ++i) {
        sqlite3_reset(insert_stmt);
        sqlite3_bind_text(insert_stmt, 1, people[i].id_, -1, SQLITE_STATIC);
        sqlite3_bind_blob(insert_stmt, 2, &people[i], sizeof(Person), SQLITE_STATIC);
        while(SQLITE_DONE != sqlite3_step(insert_stmt));
    }
    result = sqlite3_finalize(insert_stmt);

    if(SQLITE_OK != result) {
        sqlite3_close(db);
        return;
    }

    result = sqlite3_exec(db, "COMMIT TRANSACTION;", nullptr, nullptr, nullptr);
    if(SQLITE_OK != result) {
        sqlite3_close(db);
        return;
    }

    sqlite3_stmt* select_stmt = nullptr;
    for(int32_t i = 0; i < 3; ++i) {
        char buffer[16];
        sprintf(buffer, "%d", i);
        result = sqlite3_prepare_v2(db, "SELECT * FROM people WHERE id=?;", -1, &select_stmt, nullptr);
        if(SQLITE_OK != result) {
            printf("fail to prepare statement\n");
            sqlite3_close(db);
            return;
        }
        sqlite3_bind_text(select_stmt, 1, buffer, -1, SQLITE_STATIC);
        result = sqlite3_step(select_stmt);
        if(SQLITE_ROW == result) {
            const char* id = (const char*)sqlite3_column_text(select_stmt, 0);
            Person* person = (Person*)sqlite3_column_blob(select_stmt, 1);
            //UE_LOG(LogUDatabase, Warning, TEXT("%s: %s, %d\n"), id, person->name_, person->age_);
        } else {
            UE_LOG(LogUDatabase, Warning, TEXT("fail to select %d\n"), i);
        }
        sqlite3_finalize(select_stmt);
    }
    sqlite3_close(db);
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
