#include "DatabaseManager.h"

#include <Containers/AnsiString.h>
#include <sqlite/sqlite3.h>

#include "Database.h"

FDatabaseTransaction::FDatabaseTransaction()
    : DB_(nullptr)
    , Begin_(false)
    , Result_(SQLITE_OK)
{
}

FDatabaseTransaction::FDatabaseTransaction(FDatabaseTransaction&& Other)
    : DB_(Other.DB_)
    , Begin_(Other.Begin_)
    , Result_(Other.Result_)
{
    Other.DB_ = nullptr;
    Other.Begin_ = false;
    Other.Result_ = SQLITE_OK;
}

FDatabaseTransaction& FDatabaseTransaction::operator=(FDatabaseTransaction&& Other)
{
    if(this != &Other) {
        DB_ = Other.DB_;
        Result_ = Other.Result_;
        Begin_ = Other.Begin_;
        Other.DB_ = nullptr;
        Other.Begin_ = false;
        Other.Result_ = SQLITE_OK;
    }
    return *this;
}

FDatabaseTransaction::~FDatabaseTransaction()
{
    DB_ = nullptr;
}

FDatabaseTransaction::operator bool() const
{
    return nullptr != DB_;
}

bool FDatabaseTransaction::IsOK() const
{
    return Result_ == SQLITE_OK;
}

bool FDatabaseTransaction::Commit()
{
    check(nullptr != DB_);
    check(nullptr != DB_->DB_);

    check(nullptr != DB_);
    check(nullptr != DB_->DB_);
    if(!Begin_) {
        return false;
    }
    if(Result_ != SQLITE_OK){
        return false;
    }
    int32 Result = sqlite3_exec(DB_, "COMMIT;", nullptr, nullptr, nullptr);
    if(SQLITE_OK != Result) {
        return false;
    }
    Begin_ = false;
    return true;
}

bool FDatabaseTransaction::Rollback()
{
    check(nullptr != DB_);
    check(nullptr != DB_->DB_);
    if(!Begin_){
        return false;
    }
    int32 Result = sqlite3_exec(DB_, "ROLLBACK;", nullptr, nullptr, nullptr);
    if(SQLITE_OK != Result) {
        return false;
    }
    Begin_ = false;
    return true;
}

FDatabaseHandle::FDatabaseHandle()
    : DB_(nullptr)
{
}

FDatabaseHandle::FDatabaseHandle(FDatabaseHandle&& Other)
    : DB_(Other.DB_)
{
    Other.DB_ = nullptr;
}

FDatabaseHandle& FDatabaseHandle::operator=(FDatabaseHandle&& Other)
{
    if(this != &Other){
        DB_ = Other.DB_;
        Other.DB_ = nullptr;
    }
    return *this;
}

FDatabaseHandle::~FDatabaseHandle()
{
    DB_ = nullptr;
}

FDatabaseHandle::operator bool() const
{
    return nullptr != DB_;
}

bool FDatabaseHandle::CreateIfNotExists(FName Name)
{
    FString StrName = Name.ToString();
    return CreateIfNotExists(TCHAR_TO_UTF8(*StrName));
}

namespace
{
int CheckTable(void* param, int count, char** rows, char** col_names)
{
    *(int*)param = atoi(rows[0]);
    return 0;
}
}

bool FDatabaseHandle::CreateIfNotExists(const char* Name)
{
    check(nullptr != Name);
    check(nullptr != DB_);
    FAnsiString Query = FAnsiString::Printf("SELECT COUNT(*) from sqlite_master WHERE type='table' AND name='%s';", Name);
    int Count = 0;
    int32 Result = sqlite3_exec(DB_, *Query, CheckTable, (void*)&Count, nullptr);
    if(SQLITE_OK != Result){
        return false;
    }
    if(0 < Count){
        return true;
    }
    Query = FAnsiString::Printf("CREATE TABLE %s(id TEXT PRIMARY KEY, content BLOB);", Name);
    Result = sqlite3_exec(DB_, *Query, nullptr, nullptr, nullptr);
    if(SQLITE_OK != Result) {
        return false;
    }
    return true;
}

bool FDatabaseHandle::DropTable(FName Name)
{
    FString StrName = Name.ToString();
    return DropTable(TCHAR_TO_UTF8(*StrName));
}

bool FDatabaseHandle::DropTable(const char* Name)
{
    check(nullptr != Name);
    check(nullptr != DB_);
    FAnsiString Query = FAnsiString::Printf("DROP TABLE IF EXISTS %s;", Name);
    int32 Result = sqlite3_exec(DB_, *Query, nullptr, nullptr, nullptr);
    return SQLITE_OK == Result;
}

void FDatabaseHandle::Vacuum()
{
    check(nullptr != DB_);
    sqlite3_exec(DB_, "VACUUM;", nullptr, nullptr, nullptr);
}

bool FDatabaseHandle::Upsert(FName TableName, FName Key, uint32 Size, const void* Value)
{
    check(nullptr != DB_);
    check(nullptr != Value);
    FString StrTableName = TableName.ToString();
    FString StrKey = Key.ToString();
    return Upsert(TCHAR_TO_UTF8(*StrTableName), TCHAR_TO_UTF8(*StrKey), Size, Value);
}

bool FDatabaseHandle::Upsert(const char* TableName, cconst char* Key, uint32 Size, const void* Value)
{
    check(nullptr != DB_);
    check(nullptr != TableName);
    check(nullptr != Key);
    check(nullptr != Value);
    sqlite3_stmt* InsertStmt = nullptr;
    int32 Result = sqlite3_prepare_v2(DB_, "INSERT INTO ? (id,content) VALUES (?, ?) ON CONFLICT(id) DO UPDATE SET content = excluded.content;",
                                      -1,
                                      &InsertStmt,
                                      nullptr);
    if(Result != SQLITE_OK){
        return false;
    }
    sqlite3_bind_text(InsertStmt, 1, TableName, -1, SQLITE_STATIC);
    sqlite3_bind_text(InsertStmt, 2, Key, -1, SQLITE_STATIC);
    sqlite3_bind_blob64(InsertStmt, 3, Value, Size, SQLITE_STATIC);
    for(;;){
        Result = sqlite3_step(InsertStmt);
        if(Result == SQLITE_DONE){
            break;
        }
        if(Result == SQLITE_ERROR || Result == SQLITE_MISUSE) {
            sqlite3_finalize(InsertStmt);
            return false;
        }
    }
    return SQLITE_OK == sqlite3_finalize(InsertStmt);
}

    FDatabaseHandle::FDatabaseHandle(struct sqlite3* DB)
    : DB_(DB)
{
}

UDatabaseManager::UDatabaseManager(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UDatabaseManager::BeginDestroy()
{
    Super::BeginDestroy();
    for(auto Itr : Databases_){
        if(Itr.Value){
            sqlite3_close(Itr.Value);
        }
    }
    Databases_.Empty();
}

FDatabaseHandle UDatabaseManager::Open(FName Name, ESQLiteDatabaseOpenMode OpenMode)
{
    struct sqlite3** DB = Databases_.Find(Name);
    if(nullptr != DB){
        return std::move(FDatabaseHandle(*DB));
    }

    int32 OpenFlags = 0;
    switch(OpenMode) {
    case ESQLiteDatabaseOpenMode::ReadOnly:
        OpenFlags = SQLITE_OPEN_READONLY;
        break;
    case ESQLiteDatabaseOpenMode::ReadWrite:
        OpenFlags = SQLITE_OPEN_READWRITE;
        break;
    case ESQLiteDatabaseOpenMode::ReadWriteCreate:
        OpenFlags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
        break;
    }
    checkf(OpenFlags != 0, TEXT("SQLite open flags were zero! Unhandled ESQLiteDatabaseOpenMode?"));

    FString Finemane = Name.ToString();
    struct sqlite3* NewDB = nullptr;
    if(sqlite3_open_v2(TCHAR_TO_UTF8(*Finemane), &NewDB, OpenFlags, nullptr) != SQLITE_OK) {
        if(NewDB) {
            UE_LOG(LogUDatabase, Warning, TEXT("Failed to open database '%s'"), *Finemane);
            sqlite3_close(NewDB);
        }
        return std::move(FDatabaseHandle(nullptr));
    }
    Databases_.Add(Name, NewDB);
    return std::move(FDatabaseHandle(NewDB));
}

