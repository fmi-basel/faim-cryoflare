#include "collectionstartingcondition.h"
#include "collectiondefinition.h"
#include "collection.h"

Collection::Collection(QObject *parent) : QObject(parent)
{

}

void Collection::checkStartingCondition()
{
    if(starting_condition_->fullfilled()){
        start();
    }
}

void Collection::start()
{
    assembleData_();
    submitJob();
}

void Collection::assembleData_()
{
    data_=definition_->getData();
}

void Collection::submitJob()
{

}
