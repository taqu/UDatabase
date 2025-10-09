#include "DatabaseDataAsset.h"

#if WITH_EDITOR
void UDatabaseDataAsset::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
    if(PropertyChangedEvent.GetMemberPropertyName().IsEqual("DataTable")) {
        if(DataTable.IsNull()) {
            return;
        }
        if(!TableName.IsNone()){
            return;
        }
        TableName = FName(DataTable.GetAssetName());
    }
}
#endif

