#pragma once

#include <Engine/DataAsset.h>
#include <Engine/DataTable.h>

#include "DatabaseDataAsset.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UDATABASE_API UDatabaseDataAsset: public UPrimaryDataAsset
{
public:
    GENERATED_BODY()

    /**
     * Database file path.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    FName DatabasePath;

    /**
     * Table name.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    FName TableName;

#if WITH_EDITORONLY_DATA
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    TSoftObjectPtr<UDataTable> DataTable;
#endif

#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
