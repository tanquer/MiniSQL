#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include"BufferManager.h"
#include"Structures.h"
using namespace std;
#define MAX_TABLE_NUM 32
#define MAX_INDEX_NUM 480

class CatalogManager {
public:
	CatalogManager();
	~CatalogManager();
	int createTable(string tableName, vector<Attribute> attributes, string primaryKey, int attributeNum);//-1--confliction in name 0--no space 1--success -2--can't open the file
	int createIndex(string indexName, string tableName, string attributeName);//-1--confliction in name 0--no space 1--success -2--no such table -3--no such attr
	int searchTable(string tableName); //-1--not found n--location
	int searchIndex(string indexName);//-1--not found n--location
	int dropTable(string tableName);//-1--no such table 1--success
	int dropIndex(string indexName);//-1--no such index 1--success
	Table getTable(const string tableName);
	Index getIndex(const string indexName);
	Table getTablefromlog(const string tableName);
	Index getIndexfromlog(const string indexName);
	//string getIndexName(const string tableName, const string attributeName);
	//string getFileNama(const string indexName, const string fileName);
	int isAttribute(string tableName, string attributeName);//-2--no such table, -3--no such attribute, 1--no problem
	bool isIndex(string tableName, string indexName);
	void printALLTableInfo();
	void printAllIndex();
	int printTableIndex(string tableName);//-1--no such table  1--success
	int printAttributes(string tableName);//-1--no such table  1--success
	void createTableLog(Table table, ofstream& out);
private:
	Table mtables[MAX_TABLE_NUM];
	Index mindex[MAX_INDEX_NUM];
	int tablelen;
	int indexlen;
};
