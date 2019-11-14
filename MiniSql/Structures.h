#pragma once
#include<string>	
#include<iostream>
#include<vector>
using namespace std;
#define MAX_ATTRIBUTE_NUM 15
#define PRIMARY_KEY 1
#define NULL_KEY 0
#define UNIQUE_KEY 2
#define INT_ATTR 0
#define FLOAT_ATTR -1

class Index;

class Condition
{

public:
	const static int OPERATOR_EQUAL = 0; // "="
	const static int OPERATOR_NOT_EQUAL = 1; // "<>"
	const static int OPERATOR_LESS = 2; // "<"
	const static int OPERATOR_MORE = 3; // ">"
	const static int OPERATOR_LESS_EQUAL = 4; // "<="
	const static int OPERATOR_MORE_EQUAL = 5; // ">="

	Condition(string attribute, string value, int operate);

	string attributeName;
	string value;           // the value to be compared
	int operate;            // the type to be compared

	bool ifRight(int content);
	bool ifRight(float content);
	bool ifRight(string content);
};

class Conditionp {
public:
	Conditionp()
	{}
public:
	enum com { E, NE, L, G, LE,  GE } comparator;
	string Attribute_value;
	int type;
	string Constant_value;
};


class Attribute {
public:
	Attribute();
	Attribute(string name,int type,int key_type);
	//Attribute(string name, int type, int key_type,string value);
	void print(ostream& out);
	//void outAttribute(ostream&out);
	int typeSize();
public:
	int id;
	string name;
	//string value;
	int type;  //0-int ,-1-float, n- char(n)(string)
	int key_type;//1-primary 0-NULL 2-unique
	int width;
	//int spacelen;
};

class Table {
public:
	Table();
	~Table();
	string tableName;
	Attribute attributes[MAX_ATTRIBUTE_NUM];
	int PrimaryKeyID;
	int AttributeID(string attributeName);
	int spacelen;
	int attributenum;
	int blocknumber;
	vector<Index> Index_on_table;
};

class Index {
public:
	string indexName;
	string tableName;
	string attributeName;
	int spacelen;
};


class RowData
{
public:
	vector<string> attribute;
};

class Data
{
public:
	vector<RowData> Row;
};

class outputrow
{
public:
	vector<char*> attribute;
};

class outputdata
{
public:
	vector<outputrow> Row;
};