#pragma once
#include <CoreMinimal.h>
#include <SQLiteDatabase.h>
#include <Containers/AnsiString.h>
THIRD_PARTY_INCLUDES_START
#include <cthash.h>
#include <sqlite/sqlite3.h>
THIRD_PARTY_INCLUDES_END

struct sqlite3;
struct sqlite3_stmt;

class FDatabaseHandle;

class UDATABASE_API FDatabaseStatement
{
public:
    enum class Result
    {
        Done,
        Row,
        Error,
    };

    FDatabaseStatement();
    FDatabaseStatement(FDatabaseStatement&& Other);
    FDatabaseStatement& operator=(FDatabaseStatement&& Other);

    ~FDatabaseStatement();

    operator bool() const;
    bool Finalize();

    bool Upsert(FName Key, uint32 Size, const void* Value);
    bool Upsert(const char* Key, uint32 Size, const void* Value);

    bool Select(FName Key, TArray<uint8>& Value);
    bool Select(const char* Key, TArray<uint8>& Value);

    Result GetOne(FString& Key, TArray<uint8>& Value);

    template<class T>
    bool Select(T& Value, FName Key);
    template<class T>
    bool Select(T& Value, const char* Key);
private:
    friend class FDatabaseHandle;

    FDatabaseStatement(const FDatabaseStatement&) = delete;
    FDatabaseStatement& operator=(FDatabaseStatement&) = delete;

    FDatabaseStatement(FDatabaseHandle* DB, sqlite3_stmt* Stmt, const char* TableName);

    FDatabaseHandle* DB_;
    sqlite3_stmt* Stmt_;
    FAnsiString TableName_;
    bool Begin_;
    int32 Result_;
};

template<class T>
bool FDatabaseStatement::Select(T& Value, FName Key)
{
    FString StrKey = Key.ToString();
    return Select(Value, TCHAR_TO_UTF8(*StrKey));
}

template<class T>
bool FDatabaseStatement::Select(T& Value, const char* Key)
{
    check(nullptr != DB_);
    check(nullptr != Key);

    sqlite3_reset(Stmt_);
    sqlite3_bind_text(Stmt_, 1, Key, -1, SQLITE_STATIC);
    for(;;) {
        int32 Result = sqlite3_step(Stmt_);
        if(Result == SQLITE_ERROR || Result == SQLITE_MISUSE) {
            return false;
        }
        if(Result == SQLITE_ROW || Result == SQLITE_DONE) {
            int32 Size = sqlite3_column_bytes(Stmt_, 0);
            const uint8* Bytes = (const uint8*)sqlite3_column_blob(Stmt_, 0);
            if(Size != sizeof(T)){
                return false;
            }
            FMemory::Memcpy(&Value, Bytes, (SIZE_T)Size);
            break;
        }
    }
    return true;
}

class UDATABASE_API FDatabaseHandle
{
public:
    FDatabaseHandle();
    FDatabaseHandle(FDatabaseHandle&& Other);
    FDatabaseHandle& operator=(FDatabaseHandle&& Other);

    ~FDatabaseHandle();

    operator bool() const;

    bool Exists(FName TableName);
    bool Exists(const char* TableName);

    bool CreateIfNotExists(FName Name);
    bool CreateIfNotExists(const char* Name);
    bool DropTable(FName Name);
    bool DropTable(const char* Name);
    void Vacuum();

    bool Upsert(FName TableName, FName Key, uint32 Size, const void* Value);
    bool Upsert(const char* TableName, const char* Key, uint32 Size, const void* Value);

    bool Select(FName TableName, FName Key, TArray<uint8>& Value);
    bool Select(const char* TableName, const char* Key, TArray<uint8>& Value);

    FDatabaseStatement BeginUpsert(FName TableName);
    FDatabaseStatement BeginUpsert(const char* TableName);

    FDatabaseStatement BeginSelect(FName TableName);
    FDatabaseStatement BeginSelect(const char* TableName);

    FDatabaseStatement BeginGetAll(FName TableName);
    FDatabaseStatement BeginGetAll(const char* TableName);

    bool Clear(FName TableName);
    bool Clear(const char* TableName);

    template<class T>
    bool Select(T& Value, FName TableName, FName Key);

    template<class T>
    bool Select(T& Value, const char* TableName, const char* Key);
private:
    friend class FDatabaseManager;
    friend class FDatabaseStatement;

    FDatabaseHandle(const FDatabaseHandle&) = delete;
    FDatabaseHandle& operator=(FDatabaseHandle&) = delete;

    FDatabaseHandle(struct sqlite3* DB);

    struct sqlite3* DB_;
};

template<class T>
bool FDatabaseHandle::Select(T& Value, FName TableName, FName Key)
{
    FString StrTableName = TableName.ToString();
    FString StrKey = Key.ToString();
    return Select(Value, TCHAR_TO_UTF8(*StrTableName), TCHAR_TO_UTF8(*StrKey));
}

template<class T>
bool FDatabaseHandle::Select(T& Value, const char* TableName, const char* Key)
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
        if(Result == SQLITE_ERROR || Result == SQLITE_MISUSE) {
            sqlite3_finalize(SelectStmt);
            return false;
        }
        if(Result == SQLITE_ROW || Result == SQLITE_DONE) {
            int32 Size = sqlite3_column_bytes(SelectStmt, 0);
            const uint8* Bytes = (const uint8*)sqlite3_column_blob(SelectStmt, 0);
            if(Size != sizeof(T)){
                return false;
            }
            FMemory::Memcpy(&Value, Bytes, (SIZE_T)Size);
            break;
        }
    }
    return SQLITE_OK == sqlite3_finalize(SelectStmt);
}

class UDATABASE_API FDatabaseManager
{
public:
    FDatabaseManager();
    ~FDatabaseManager();

    FDatabaseHandle Open(const FName& Path, ESQLiteDatabaseOpenMode OpenMode = ESQLiteDatabaseOpenMode::ReadWriteCreate);
    void Close(const FName& Path);
private:
    FDatabaseManager(const FDatabaseManager&) = delete;
    FDatabaseManager& operator=(const FDatabaseManager&) = delete;
    template<size_t N>
    static constexpr std::array<uint8_t, 64> to_char_array(const std::array<uint8_t, N>& x)
    {
        constexpr uint8_t hex_chars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        std::array<uint8_t, N * 2> str;
        for(size_t i = 0; i < N; ++i) {
            str[2 * i + 0] = hex_chars[x[i] & 0xFU];
            str[2 * i + 1] = hex_chars[(x[i] >> 4) & 0xFU];
        }
        return str;
    }
#ifdef UDATABASE_PASSWORD
    static constexpr std::array<uint8_t, 32> key = cthash::sha256_string((const uint8_t[])PREPROCESSOR_TO_STRING(UDATABASE_PASSWORD));
    static constexpr std::array<uint8_t, 32> pass = cthash::blake3_encrypt((const uint8_t[])PREPROCESSOR_TO_STRING(UDATABASE_PASSWORD));
#else
    static constexpr std::array<uint8_t, 32> key = cthash::sha256_string((const uint8_t[])"Hello World!");
    static constexpr std::array<uint8_t, 32> pass = cthash::blake3_encrypt((const uint8_t[]) "Hello World!");
#endif
    static constexpr auto encrypted = cthash::chacha20_encrypt(key, to_char_array(pass));
    TMap<FName, struct sqlite3*> Databases_;
};

