#include"stdafx.h"
#include"CatalogManager.h"
#include<cstdio>

extern void error(string content);
const char * TableCatalog = "Catalog_Table.txt";
const char * IndexCatalog = "Catalog_Index.txt";

CatalogManager::CatalogManager()
{
	tablelen = indexlen = 0;
	ifstream in1(TableCatalog);
	if (!in1) {//初始化两个文件
		ofstream newtables(TableCatalog);
		newtables.close();
		ofstream newindex(IndexCatalog);
		newindex.close();
		//cout << "Initialize catalog manager successfully." << endl;
		return;
	}
	string tmptablename;
	in1 >> tmptablename;
	while (in1.peek()!=EOF)
	{
		Table newtable = getTablefromlog(tmptablename);
		if (newtable.tableName == "") {
			error("Error with reading the table " + tmptablename + "!\nStop initializing the Catalog Manager!\n");
			return;
		}
		mtables[tablelen++] = newtable;
		in1 >> tmptablename;
	}
	in1.close();
	ifstream in2(IndexCatalog);
	if (!in2) {
		ofstream newindex(IndexCatalog);
		newindex.close();
		return;
	}
	string tmpindexname, tmptable, tmpattr;
	int spacelen;
	in2 >> tmpindexname;
	while (in2.peek()!=EOF)
	{
		in2 >> tmptable >> tmpattr >> spacelen;
		Index newindex = getIndexfromlog(tmpindexname);
		if (newindex.indexName == "") {
			error("Error with reading the index " + tmpindexname + "!\nStop initializing the Catalog Manager!\n");
			return;
		}
		mindex[indexlen++] = newindex;
		in2 >> tmpindexname;
	}
	in2.close();
	cout << "Initialize catalog manager successfully." << endl;
}

CatalogManager::~CatalogManager() 
{
	for (int i = 0; i < MAX_TABLE_NUM; i++)
		mtables[i].tableName = "";
	for (int i = 0; i < MAX_INDEX_NUM; i++)
		mindex[i].indexName = "";
	tablelen = indexlen = 0;
}

int CatalogManager::isAttribute(string tableName, string attributeName)
{
	Table tmp;
	for (int i = 0; i < tablelen; i++) {
		tmp = mtables[i];
		if (tmp.tableName == tableName) {
			for (int j = 0; j < tmp.attributenum; j++) {
				if (tmp.attributes[j].name == attributeName) return true;
			}
		}
	}
	return false;
}

bool CatalogManager::isIndex(string tableName, string indexName)
{
	Table tmp;
	for (int i = 0; i < tablelen; i++) {
		tmp = mtables[i];
		if (tmp.tableName == tableName) {
			for (vector<Index>::iterator iter = tmp.Index_on_table.begin(); iter != tmp.Index_on_table.end();iter++) {
				if (iter->indexName == indexName) return 1;
			}
		}
	}
	return 0;
}

void CatalogManager::createTableLog(Table table, ofstream& out)
{
	out << table.tableName << endl;
	out << table.attributes[table.PrimaryKeyID].name << endl;
	out << table.attributenum << endl;
	for (int i = 0; i < table.attributenum; i++) {
		table.attributes[i].print(out);
	}
	out << endl;
	for (int i = 0; i < table.Index_on_table.size(); i++) {
		out << table.Index_on_table[i].indexName << " ";
	}
}

int CatalogManager::createTable(string tableName, vector<Attribute> attributes, string primaryKey, int attributeNum)
{
	if (tablelen == MAX_TABLE_NUM) {
		error("No space!\n");
		return 0;  //no space
	}
	for (int i = 0; i < tablelen; i++) {//check whether there is already a table with the same tableName
		if (mtables[i].tableName == tableName) {
			error("Tables conflict in name!\n");
			return -1;
		}
	}
	Table newtable;
	newtable.tableName = tableName;
	newtable.attributenum = attributeNum;
	for (int i = 0; i < attributeNum; i++) {
		if (attributes[i].name == primaryKey)
			newtable.PrimaryKeyID = i;
		newtable.attributes[i] = attributes[i];		
		//newtable.spacelen += newtable.attributes[i].spacelen;
	}
	//newtable.spacelen += tableName.length() + 1 + primaryKey.length() + 1;
	mtables[tablelen++] = newtable;

	ofstream tableout;
	tableout.open(TableCatalog, ios::app);
	if (!tableout) {
		error("Error with opening the catalog file\n");
		return -2;
	}
	tableout << tableName << endl;
	tableout.close();

	string TB_catalog = tableName + "_table.log";
	ofstream newlog;
	newlog.open(TB_catalog, ios::app);
	createTableLog(newtable, newlog);
	//newlog << to_string(0) << " ";//刚创建的时候没有index,0个index
	newlog.close();
	return 1;
}

int CatalogManager::createIndex(string indexName, string tableName, string attributeName)
{
	int isattr = isAttribute(tableName, attributeName);
	if (isattr == 0) {
		error("No such attribute!\n");
		cout << attributeName << endl;
		return isattr;
	}
	bool isind = isIndex(tableName, indexName);
	if (isind == true) {
		error("Index confliction in name!Create index aborted.\n");
		return -1;
	}
	if (indexlen == MAX_INDEX_NUM) return 0; //no space
	for (int i = 0; i < indexlen; i++) {  //check whether there is already an index with the same indexName
		if (mindex[i].indexName == indexName && mindex[i].tableName == tableName) {
			error("Index confliction in name!\n");
			cout << indexName << endl;
			return -1;//confliction in name
		}
	}
	Table tmp;
	Index newindex;
	newindex.indexName = indexName;
	newindex.tableName = tableName;
	newindex.attributeName = attributeName;
	//newindex.spacelen = indexName.length() + tableName.length() + attributeName.length() + 3;
	mindex[indexlen++] = newindex;

	for (int i = 0; i < tablelen; i++) {
		if (mtables[i].tableName == tableName)
		{
			mtables[i].Index_on_table.push_back(newindex);
			for (int j = 0; j < mtables[i].attributenum; j++)
				if (mtables[i].attributes[j].name == mindex[indexlen - 1].attributeName)
					mindex[indexlen - 1].spacelen = mtables[i].attributes[j].typeSize();
		}
	}

	ofstream indexout;
	indexout.open(IndexCatalog, ios::app);
	if (!indexout) {
		error("Error with opening the catalog file\n");
		return -2;
	}
	indexout << indexName << " ";
	indexout << tableName << " ";
	indexout << attributeName << " ";
	indexout << mindex[indexlen - 1].spacelen << " ";
	indexout.close();

	string tablelog = tableName + "_table.log";
	ofstream tableout;
	tableout.open(tablelog, ios::app);
	tableout << indexName << " ";
	//tableout << attributeName << endl;
	tableout.close();
	return 1;
}

int CatalogManager::searchTable(string tableName)
{
	int i;
	for (i = 0; i < tablelen; i++) {
		if (mtables[i].tableName == tableName)
			return i;
	}
	return -1;
}

int CatalogManager::searchIndex(string indexName)//-1--not found n--location
{
	int i;
	for (i = 0; i < indexlen; i++) {
		if (mindex[i].indexName == indexName)
			return i;
	}
	return -1;
}

int CatalogManager::dropTable(string tableName)
{
	string newtablelog = "";
	ifstream in1(TableCatalog);
	if (!in1) {
		error("Error with opening the catalog file\n");
		return -2;
	}
	for (int i = 0; i < tablelen; i++) {
		string tmp;
		in1 >> tmp;
		if (tmp != tableName) newtablelog += tmp + " ";
		else continue;
	}
	in1.close();
	ofstream tableout;
	tableout.open(TableCatalog, ios::out);
	tableout << newtablelog;
	tableout.close();

	string tablelog = tableName + "_table.log";
	remove(tablelog.c_str());

	string newindexlog = "";
	ifstream in2(IndexCatalog);
	if (!in2) {
		error("Error with opening the catalog file\n");
		return -2;
	}
	for (int i = 0; i < indexlen; i++) {
		string tmp1,tmp2,tmp3;
		in2 >> tmp1 >> tmp2 >> tmp3;
		if (tmp2 != tableName) newindexlog += tmp1 + " " + tmp2 + " " + tmp3 + " ";
		else continue;
	}
	in2.close();
	ofstream indexout;
	indexout.open(IndexCatalog, ios::out);
	indexout << newindexlog;
	indexout.close();

	for (int i = 0; i < tablelen; ) {
		if (mtables[i].tableName == tableName) {
			int j;
			for (j = i; j < tablelen - 1; j++) {
				mtables[j] = mtables[j + 1];
			}
			tablelen--;
			return 1;
		}
		else {
			i++;
		}
	}

	return 1;
}

int CatalogManager::dropIndex(string indexName)//-1--no such index 1--success
{
	Table deleted;
	string tableName;
	if (indexlen == 0) {
		error("Index not found!\n");
		return -1;
	}
	for (int i = 0; i < indexlen;) {
		if (mindex[i].indexName == indexName) {
			tableName = mindex[i].tableName;
			if (indexlen == 1) {
				mindex[0].tableName = mindex[0].indexName = mindex[0].attributeName = "";
			}
			for (int j = i; j < indexlen; j++) {
				mindex[j] = mindex[j + 1];
			}
			indexlen--;
			break;
			//return 1;
		}
		else {
			i++;
			if (i == indexlen) {
				error("Index not found!\n");
				return -1;
			}
		}
	}
	for (int i = 0; i < tablelen; i++) {
		if (mtables[i].tableName == tableName) {
			vector<Index> tmpindex;
			for (int j = 0; j < mtables[i].Index_on_table.size(); j++) {
				if (mtables[i].Index_on_table[j].indexName == indexName)
					continue;
				else tmpindex.push_back(mtables[i].Index_on_table[j]);
			}
			mtables[i].Index_on_table = tmpindex;
			deleted = mtables[i];
			break;
		}
	}

	string newindexlog = "";
	ifstream in(IndexCatalog);
	if (!in) {
		error("Error with opening the catalog file\n");
		return -2;
	}
	for (int i = 0; i < indexlen; i++) {
		string tmp1, tmp2, tmp3;
		in >> tmp1 >> tmp2 >> tmp3;
		if (tmp1 != indexName) newindexlog += tmp1 + " " + tmp2 + " " + tmp3 + " ";
		else continue;
	}
	in.close();
	ofstream indexout;
	indexout.open(IndexCatalog, ios::out);
	indexout << newindexlog;
	indexout.close();

	string tableinfo = tableName + "_table.log";
	string newtablelog = "";
	ifstream in2(tableinfo);
	if (!in2) {
		error("Error with opening the table catalog file\n");
		return -2;
	}
	string TB_catalog = tableName + "_table.log";
	ofstream newlog;
	newlog.open(TB_catalog, ios::out);
	createTableLog(deleted, newlog);
	newlog.close();

	return 1;
}

void CatalogManager::printALLTableInfo()
{
	for (int i = 0; i < tablelen; i++) {
		cout << std::left << setw(30) << "Table " + to_string(i+1) + " : " + mtables[i].tableName << std::left << setw(30) << "Primary Key : " + mtables[i].attributes[mtables[i].PrimaryKeyID].name << endl;
		cout << "Attributes : " << endl;
		for (int j = 0; j < mtables[i].attributenum; j++) {
			cout << std::left << setw(15) << mtables[i].attributes[j].name << endl;
		}
		cout << "Index on table : " << endl;
		if (mtables[i].Index_on_table.size() == 0) {
			cout << "No Index" << endl;
			continue;
		}
		for (int j = 0; j < mtables[i].Index_on_table.size(); j++) {
			cout << mtables[i].Index_on_table[j].indexName << endl;
		}
	}
}

void CatalogManager::printAllIndex()
{
	for (int i = 0; i < indexlen; i++) {
		cout << mindex[i].indexName << " " << mindex[i].tableName << " " << mindex[i].attributeName << endl;
	}
	return;
}

int CatalogManager::printTableIndex(string tableName)
{
	cout << "Index on " << tableName << " :" << endl;
	int i;
	cout << tablelen << endl;
	for (i = 0; i < tablelen; i++) {
		if (mtables[i].tableName == tableName) {
			for (vector<Index>::iterator iter = mtables[i].Index_on_table.begin(); iter != mtables[i].Index_on_table.end(); iter++) {
				cout << iter->indexName << endl;
				//cout << "aaaa" << to_string(mtables[i].Index_on_table.size()) << endl;
			}
		}
	}
	return 1;
}

int CatalogManager::printAttributes(string tableName)
{
	cout << "Attributes on " << tableName << " :" << endl;
	int i;
	for (i = 0; i < tablelen; i++) {
		if (mtables[i].tableName == tableName) {
			for (int j = 0; j < mtables[i].attributenum; j++) {
				cout << mtables[i].attributes[j].name << endl;
			}
		}
	}
	return 1;
}

Table CatalogManager::getTable(const string tableName)
{
	Table newtable;
	for (int i = 0; i < tablelen; i++) {
		if (mtables[i].tableName == tableName) {
			return mtables[i];
		}
	}
	newtable.tableName = "";
	return newtable;
}

Index CatalogManager::getIndex(const string indexName)
{
	Index newindex;
	for (int i = 0; i < indexlen; i++) {
		if (mindex[i].indexName == indexName) {
			return mindex[i];
		}
	}
	newindex.indexName = "";
	return newindex;
}

Table CatalogManager::getTablefromlog(const string tableName)
{
	Table newtable;
	string tableinfo = tableName + "_table.log";
	string primarykey, tmp;
	int attrnum;
	ifstream in(tableinfo);
	if (!in) {
		error("No such table file!\n");
		newtable.tableName = "";
		return newtable;
	}
	in >> newtable.tableName >> primarykey >> newtable.attributenum;
	for (int i = 0; i < newtable.attributenum; i++) {
		Attribute newattr;
		newattr.id = i;
		in >> newattr.name;
		if (newattr.name == primarykey) {
			newtable.PrimaryKeyID = i;
		}
		in >> tmp;
		if (tmp[0] == 'i') {
			newattr.type = INT_ATTR;
			newattr.width = 4;
		}
		else if (tmp[0] == 'f') {
			newattr.type = FLOAT_ATTR;
			newattr.width = 4;
		}
		else {
			string tmpwidth = "";
			char tmpchar;
			tmpchar = tmp[5];
			for (int i = 6; tmpchar != ')'; i++) {
				tmpwidth += to_string(tmpchar-'0');
				tmpchar = tmp[i];
			}
			newattr.type = atoi(tmpwidth.c_str());
			newattr.width = atoi(tmpwidth.c_str());
		}
		in >> tmp;
		if (tmp[0] == 'N') newattr.key_type = NULL_KEY;
		else if (tmp[0] == 'P') {
			newattr.key_type = PRIMARY_KEY;
			//newtable.PrimaryKeyID = i;
		}
		else if (tmp[0] == 'U') newattr.key_type = UNIQUE_KEY;
		else newattr.key_type = -1;
		newtable.attributes[i] = newattr;
	}
	//cout << newtable.attributes[0].name << endl;
	in >> tmp;
	while (in.peek()!=EOF) {
		Index newindex = getIndexfromlog(tmp);
		if (newindex.indexName == "") {
			error("Error with reading the index information!\n");
			newtable.tableName = "";
			return newtable;
		}
		newtable.Index_on_table.push_back(newindex);
		in >> tmp;
	}
	in.close();
	return newtable;
}

Index CatalogManager::getIndexfromlog(const string indexName)
{
	Index newindex;
	string tmpindex, tmptable, tmpattr;
	int spacelen;
	ifstream in(IndexCatalog);
	if (!in) {
		error("Error with opening the catalog file!\n");
		newindex.indexName = "";
		return newindex;
	}
	in >> tmpindex;
	while (in.peek()!=EOF)
	{
		in >> tmptable >> tmpattr >> spacelen;
		if (tmpindex == indexName) {
			newindex.indexName = tmpindex;
			newindex.tableName = tmptable;
			newindex.attributeName = tmpattr;
			newindex.spacelen = spacelen;
			in.close();
			return newindex;
		}
		in >> tmpindex;
	}
	error("No such index!\n");
	newindex.indexName = "";
	in.close();
	return newindex;
}
