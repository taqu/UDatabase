#include "DatabaseManager.h"

#include "Misc/Paths.h"
#include "Misc/PackageName.h"
THIRD_PARTY_INCLUDES_START
#include <sqlite/sqlite3.h>
THIRD_PARTY_INCLUDES_END

#include "Database.h"

FDatabaseStatement::FDatabaseStatement()
    : DB_(nullptr)
    , Stmt_(nullptr)
    , Begin_(false)
    , Result_(SQLITE_OK)
{
}

FDatabaseStatement::FDatabaseStatement(FDatabaseHandle* DB, sqlite3_stmt* Stmt, const char* TableName)
    : DB_(DB)
    , Stmt_(Stmt)
    , TableName_(TableName)
    , Begin_(true)
    , Result_(SQLITE_OK)
{
}

FDatabaseStatement::FDatabaseStatement(FDatabaseStatement&& Other)
    : DB_(Other.DB_)
    , Stmt_(Other.Stmt_)
    , TableName_(std::move(Other.TableName_))
    , Begin_(Other.Begin_)
    , Result_(Other.Result_)
{
    Other.DB_ = nullptr;
    Other.Stmt_ = nullptr;
    Other.Begin_ = false;
    Other.Result_ = SQLITE_OK;
}

FDatabaseStatement& FDatabaseStatement::operator=(FDatabaseStatement&& Other)
{
    if(this != &Other) {
        DB_ = Other.DB_;
        Stmt_ = Other.Stmt_;
        TableName_ = std::move(Other.TableName_);
        Result_ = Other.Result_;
        Begin_ = Other.Begin_;
        Other.DB_ = nullptr;
        Other.Stmt_ = nullptr;
        Other.Begin_ = false;
        Other.Result_ = SQLITE_OK;
    }
    return *this;
}

FDatabaseStatement::~FDatabaseStatement()
{
    DB_ = nullptr;
   if(nullptr != Stmt_){
        sqlite3_finalize(Stmt_);
       Stmt_ = nullptr;
    }
}

FDatabaseStatement::operator bool() const
{
    return nullptr != DB_ && nullptr != Stmt_;
}

bool FDatabaseStatement::Finalize()
{
    if(nullptr != Stmt_) {
        int32 Result = sqlite3_finalize(Stmt_);
        Stmt_ = nullptr;
        return SQLITE_OK == Result;
    }
    return true;
}

bool FDatabaseStatement::Upsert(FName Key, uint32 Size, const void* Value)
{
    FString StrKey = Key.ToString();
    return Upsert(TCHAR_TO_UTF8(*StrKey), Size, Value);
}

bool FDatabaseStatement::Upsert(const char* Key, uint32 Size, const void* Value)
{
    check(nullptr != DB_);
    check(nullptr != Key);
    check(nullptr != Value);

    sqlite3_reset(Stmt_);
    sqlite3_bind_text(Stmt_, 1, Key, -1, SQLITE_STATIC);
    sqlite3_bind_blob64(Stmt_, 2, Value, Size, SQLITE_STATIC);
    for(;;) {
        int32 Result = sqlite3_step(Stmt_);
        if(Result == SQLITE_DONE) {
            break;
        }
        if(Result == SQLITE_ERROR || Result == SQLITE_MISUSE) {
            return false;
        }
    }
    return true;
}

bool FDatabaseStatement::Select(FName Key, TArray<uint8>& Value)
{
    FString StrKey = Key.ToString();
    return Select(TCHAR_TO_UTF8(*StrKey), Value);
}

bool FDatabaseStatement::Select(const char* Key, TArray<uint8>& Value)
{
    check(nullptr != DB_);
    check(nullptr != Key);

    sqlite3_reset(Stmt_);
    sqlite3_bind_text(Stmt_, 1, Key, -1, SQLITE_STATIC);
    for(;;) {
        int32 Result = sqlite3_step(Stmt_);
        if(Result == SQLITE_DONE) {
            return false;
        }
        if(Result == SQLITE_ERROR || Result == SQLITE_MISUSE) {
            return false;
        }
        if(Result == SQLITE_ROW) {
            int32 Size = sqlite3_column_bytes(Stmt_, 0);
            Value.SetNum(Size);
            const uint8* Bytes = (const uint8*)sqlite3_column_blob(Stmt_, 0);
            FMemory::Memcpy(&Value[0], Bytes, (SIZE_T)Size);
            break;
        }
    }
    return true;
}

FDatabaseStatement::Result FDatabaseStatement::GetOne(FString& Key, TArray<uint8>& Value)
{
    for(;;) {
        int32 Result = sqlite3_step(Stmt_);
        if(Result == SQLITE_ERROR || Result == SQLITE_MISUSE) {
            return Result::Error;
        }
        if(Result == SQLITE_DONE){
            return Result::Done;
        }
        if(Result == SQLITE_ROW) {
            const char* id = (const char*)sqlite3_column_text(Stmt_, 0);
            Key = id;
            int32 Size = sqlite3_column_bytes(Stmt_, 1);
            Value.SetNum(Size);
            const uint8* Bytes = (const uint8*)sqlite3_column_blob(Stmt_, 1);
            FMemory::Memcpy(&Value[0], Bytes, (SIZE_T)Size);
            return  Result::Row;
        }
        return Result::Done;
    }
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

namespace
{
int CheckTable(void* param, int count, char** rows, char** col_names)
{
    *(int*)param = atoi(rows[0]);
    return 0;
}
} // namespace

bool FDatabaseHandle::Exists(FName TableName)
{
    FString StrName = TableName.ToString();
    return Exists(TCHAR_TO_UTF8(*StrName));
}

bool FDatabaseHandle::Exists(const char* TableName)
{
    check(nullptr != TableName);
    check(nullptr != DB_);
    FAnsiString Query = FAnsiString::Printf("SELECT COUNT(*) from sqlite_master WHERE type='table' AND name='%s';", TableName);
    int Count = 0;
    int32 Result = sqlite3_exec(DB_, *Query, CheckTable, (void*)&Count, nullptr);
    if(SQLITE_OK != Result) {
        return false;
    }
    return 0 < Count;
}

bool FDatabaseHandle::CreateIfNotExists(FName Name)
{
    FString StrName = Name.ToString();
    return CreateIfNotExists(TCHAR_TO_UTF8(*StrName));
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

bool FDatabaseHandle::Upsert(const char* TableName, const char* Key, uint32 Size, const void* Value)
{
    check(nullptr != DB_);
    check(nullptr != TableName);
    check(nullptr != Key);
    check(nullptr != Value);
    sqlite3_stmt* InsertStmt = nullptr;
    FAnsiString Query = FAnsiString::Printf("INSERT INTO %s (id,content) VALUES (?, ?) ON CONFLICT(id) DO UPDATE SET content = excluded.content;", TableName);
    int32 Result = sqlite3_prepare_v2(DB_, *Query,
                                      -1,
                                      &InsertStmt,
                                      nullptr);
    if(Result != SQLITE_OK){
        return false;
    }
    sqlite3_bind_text(InsertStmt, 1, Key, -1, SQLITE_STATIC);
    sqlite3_bind_blob64(InsertStmt, 2, Value, Size, SQLITE_STATIC);
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

bool FDatabaseHandle::Select(FName TableName, FName Key, TArray<uint8>& Value)
{
    check(nullptr != DB_);
    FString StrTableName = TableName.ToString();
    FString StrKey = Key.ToString();
    return Select(TCHAR_TO_UTF8(*StrTableName), TCHAR_TO_UTF8(*StrKey), Value);
}

bool FDatabaseHandle::Select(const char* TableName, const char* Key, TArray<uint8>& Value)
{
    check(nullptr != DB_);
    check(nullptr != TableName);
    check(nullptr != Key);

    sqlite3_stmt* SelectStmt = nullptr;
    FAnsiString Query = FAnsiString::Printf("SELECT content FROM %s WHERE id=?;", TableName);

    int32 Result = sqlite3_prepare_v2(DB_, *Query, -1, &SelectStmt, nullptr);
    if(SQLITE_OK != Result) {
        return false;
    }
    sqlite3_bind_text(SelectStmt, 1, Key, -1, SQLITE_STATIC);
    for(;;) {
        Result = sqlite3_step(SelectStmt);
        if(Result == SQLITE_DONE) {
            sqlite3_finalize(SelectStmt);
            return false;
        }
        if(Result == SQLITE_ERROR || Result == SQLITE_MISUSE) {
            sqlite3_finalize(SelectStmt);
            return false;
        }
        if(Result == SQLITE_ROW) {
            int32 Size = sqlite3_column_bytes(SelectStmt, 0);
            Value.SetNum(Size);
            const uint8* Bytes = (const uint8*)sqlite3_column_blob(SelectStmt, 0);
            FMemory::Memcpy(&Value[0], Bytes, (SIZE_T)Size);
            break;
        }
    }
    return SQLITE_OK == sqlite3_finalize(SelectStmt);
}

FDatabaseStatement FDatabaseHandle::BeginUpsert(FName TableName)
{
    FString StrTableName = TableName.ToString();
    return BeginUpsert(TCHAR_TO_UTF8(*StrTableName));
}

FDatabaseStatement FDatabaseHandle::BeginUpsert(const char* TableName)
{
    check(nullptr != DB_);
    check(nullptr != TableName);

    FAnsiString Query = FAnsiString::Printf("INSERT INTO %s (id,content) VALUES (?, ?) ON CONFLICT(id) DO UPDATE SET content = excluded.content;", TableName);
    sqlite3_stmt* InsertStmt = nullptr;
    int32 Result = sqlite3_prepare_v2(DB_, *Query,
                                      -1,
                                      &InsertStmt,
                                      nullptr);
    if(SQLITE_OK != Result) {
        return FDatabaseStatement();
    }
    return FDatabaseStatement(this, InsertStmt, TableName);
}

FDatabaseStatement FDatabaseHandle::BeginSelect(FName TableName)
{
    FString StrTableName = TableName.ToString();
    return BeginSelect(TCHAR_TO_UTF8(*StrTableName));
}

FDatabaseStatement FDatabaseHandle::BeginSelect(const char* TableName)
{
    check(nullptr != DB_);
    check(nullptr != TableName);

    sqlite3_stmt* SelectStmt = nullptr;
    FAnsiString Query = FAnsiString::Printf("SELECT content FROM %s WHERE id=?;", TableName);
    int32 Result = sqlite3_prepare_v2(DB_, *Query, -1, &SelectStmt, nullptr);
    if(SQLITE_OK != Result) {
        return FDatabaseStatement();
    }
    return FDatabaseStatement(this, SelectStmt, TableName);
}

FDatabaseStatement FDatabaseHandle::BeginGetAll(FName TableName)
{
    FString StrTableName = TableName.ToString();
    return BeginGetAll(TCHAR_TO_UTF8(*StrTableName));
}

FDatabaseStatement FDatabaseHandle::BeginGetAll(const char* TableName)
{
    check(nullptr != DB_);
    check(nullptr != TableName);

    sqlite3_stmt* SelectStmt = nullptr;
    FAnsiString Query = FAnsiString::Printf("SELECT * FROM %s ;", TableName);
    int32 Result = sqlite3_prepare_v2(DB_, *Query, -1, &SelectStmt, nullptr);
    if(SQLITE_OK != Result) {
        return FDatabaseStatement();
    }
    return FDatabaseStatement(this, SelectStmt, TableName);
}

bool FDatabaseHandle::Clear(FName TableName)
{
    FString StrTableName = TableName.ToString();
    return Clear(TCHAR_TO_UTF8(*StrTableName));
}

bool FDatabaseHandle::Clear(const char* TableName)
{
    check(nullptr != DB_);
    check(nullptr != TableName);
    FAnsiString Query = FAnsiString::Printf("DELETE FROM %s;", TableName);
    int32 Result = sqlite3_exec(DB_, *Query, nullptr, nullptr, nullptr);
    return SQLITE_OK == Result;
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

FDatabaseHandle UDatabaseManager::Open(const FName& Path, ESQLiteDatabaseOpenMode OpenMode)
{
    struct sqlite3** DB = Databases_.Find(Path);
    if(nullptr != DB) {
        return std::move(FDatabaseHandle(*DB));
    }

    FString ContentPath = FPaths::ConvertRelativePathToFull(FPackageName::LongPackageNameToFilename(Path.ToString()));

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

    struct sqlite3* NewDB = nullptr;
    if(sqlite3_open_v2(TCHAR_TO_UTF8(*ContentPath), &NewDB, OpenFlags, nullptr) != SQLITE_OK) {
        if(NewDB) {
            UE_LOG(LogUDatabase, Warning, TEXT("Failed to open database '%s'"), *ContentPath);
            sqlite3_close(NewDB);
        }
        return std::move(FDatabaseHandle(nullptr));
    }
    //auto decrypted = cthash::chacha20_encrypt(key, encrypted);
    //int32 Result = sqlite3_key(NewDB, &decrypted[0], (int32)decrypted.size());
    Databases_.Add(Path, NewDB);
    return std::move(FDatabaseHandle(NewDB));
}

void UDatabaseManager::Close(const FName& Path)
{
    struct sqlite3** DB = Databases_.Find(Path);
    if(nullptr == DB) {
        return;
    }
    sqlite3_close(*DB);
    Databases_.Remove(Path);
}

