#pragma once
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include "BufferManager.h"
#include "sql.h"
#include "Structures.h"
#define RECORD_MAX_SIZE 256
#define COTENT_MAX_SIZE 100
using namespace std;
extern BufferManager bm;

class RecordManager{
public:
	RecordManager() {}
	~RecordManager() {}
	//BufferManager bm;
	
	int tableCreate(string tableName);// create a table
    int tableDrop(string tableName);// drop a table
    
    int indexCreate(string indexName);// create a index
    int indexDrop(string indexName);// drop a index
        
    int recordInsert(string tableName, BYTE* record, int recordSize);// insert a record 
    
    int recordAllShow(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM], vector<Condition>* conditionVector);// show the records of the table in certain condition
    int recordBlockShow(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM], vector<Condition>* conditionVector, int blockOffset);// show the records of the block in certain condition
    
    int recordAllFind(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM], vector<Condition>* conditionVector);// find the records of the table in certain condition
    
    int recordAllDelete(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM], vector<Condition>* conditionVector);// delete some records
    int recordBlockDelete(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM], vector<Condition>* conditionVector, int blockOffset);
    
    int recordChosePrint(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM],Attribute attributesPrint[MAX_ATTRIBUTE_NUM]); //chose some certain attributes to print records   
	int recordBlockChosePrint(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM], Attribute attributesPrint[MAX_ATTRIBUTE_NUM], int blockOffset);
    
    string tableFileNameGet(string tableName);// get the filename of the table
    string indexFileNameGet(string indexName);// get the filename of the index
private:
    int recordBlockShow(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM], vector<Condition>* conditionVector, BlockNode* block);
    int recordBlockFind(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM], vector<Condition>* conditionVector, BlockNode* block);
    int recordBlockDelete(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM], vector<Condition>* conditionVector, BlockNode* block);
    int recordBlockChosePrint(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM],Attribute attributesPrint[MAX_ATTRIBUTE_NUM], BlockNode* block);
    int recordSize(Attribute attributes[MAX_ATTRIBUTE_NUM]);
    
    bool recordConditionFit(BYTE* recordBegin,int recordSize,Attribute attributes[MAX_ATTRIBUTE_NUM],vector<Condition>* conditionVector);
    void recordPrint(BYTE* recordBegin, int recordSize,Attribute attributes[MAX_ATTRIBUTE_NUM],Attribute attributesPrint[MAX_ATTRIBUTE_NUM]);
    bool contentConditionFit(char* content, int type, Condition* condition);
    void contentPrint(char* content, int type);

			
};
