#pragma once
#include <CoreMinimal.h>
#include <SQLiteDatabase.h>
#include <Containers/AnsiString.h>
THIRD_PARTY_INCLUDES_START
#include <cthash.h>
THIRD_PARTY_INCLUDES_END

#include "DatabaseManager.generated.h"

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
private:
    friend class UDatabaseManager;
    friend class FDatabaseStatement;

    FDatabaseHandle(const FDatabaseHandle&) = delete;
    FDatabaseHandle& operator=(FDatabaseHandle&) = delete;

    FDatabaseHandle(struct sqlite3* DB);

    struct sqlite3* DB_;
};

UCLASS()
class UDATABASE_API UDatabaseManager: public UObject
{
    GENERATED_UCLASS_BODY()
public:
    virtual void BeginDestroy() override;

    FDatabaseHandle Open(const FName& Path, ESQLiteDatabaseOpenMode OpenMode);
    void Close(const FName& Path);
private:
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
#if 0
#ifdef UDATABASE_PASSWORD
    static constexpr std::array<uint8_t, 32> key = cthash::sha256_string((const uint8_t[])PREPROCESSOR_TO_STRING(UDATABASE_PASSWORD));
    static constexpr std::array<uint8_t, 32> pass = cthash::blake3_encrypt((const uint8_t[])PREPROCESSOR_TO_STRING(UDATABASE_PASSWORD));
#else
    static constexpr std::array<uint8_t, 32> key = cthash::sha256_string((const uint8_t[])"Hello World!");
    static constexpr std::array<uint8_t, 32> pass = cthash::blake3_encrypt((const uint8_t[]) "Hello World!");
#endif
    static constexpr auto encrypted = cthash::chacha20_encrypt(key, to_char_array(pass));
#endif
    TMap<FName, struct sqlite3*> Databases_;
};

