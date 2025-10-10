#pragma once
#include <CoreMinimal.h>
#include <SQLiteDatabase.h>
#include "DatabaseManager.generated.h"

struct sqlite3;

class FDatabaseHandle;

class FDatabaseTransaction
{
public:
    FDatabaseTransaction();
    FDatabaseTransaction(FDatabaseTransaction&& Other);
    FDatabaseTransaction& operator=(FDatabaseTransaction&& Other);

    ~FDatabaseTransaction();

    operator bool() const;
    bool IsOK() const;
    bool Commit();
    bool Rollback();
private:
    friend class FDatabaseHandle;

    FDatabaseTransaction(const FDatabaseTransaction&) = delete;
    FDatabaseTransaction& operator=(FDatabaseTransaction&) = delete;

    FDatabaseTransaction(FDatabaseHandle* DB);

    FDatabaseHandle* DB_;
    bool Begin_;
    int32 Result_;
};

class FDatabaseHandle
{
public:
    FDatabaseHandle();
    FDatabaseHandle(FDatabaseHandle&& Other);
    FDatabaseHandle& operator=(FDatabaseHandle&& Other);

    ~FDatabaseHandle();

    operator bool() const;

    bool CreateIfNotExists(FName Name);
    bool CreateIfNotExists(const char* Name);
    bool DropTable(FName Name);
    bool DropTable(const char* Name);
    void Vacuum();

    bool Upsert(FName TableName, FName Key, uint32 Size, const void* Value);
    bool Upsert(const char* TableName, const char* Key, uint32 Size, const void* Value);

    bool Select(const char* TableName, const char* Key, uint32& Size, void* Value);
private:
    friend class UDatabaseManager;
    friend class FDatabaseTransaction;

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

    FDatabaseHandle Open(FName Name, ESQLiteDatabaseOpenMode OpenMode);

private:
    TMap<FName, struct sqlite3*> Databases_;
};

