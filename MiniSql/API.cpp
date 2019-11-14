#include "stdafx.h"
#include "API.h"

IndexManager index;
CatalogManager cat;
Interpreter inter;
BufferManager bm;
RecordManager rec;

API::API()
{	
	puts("| Our Project for Database System 2017-2018 Summer |");
}

API::~API()
{
}

void API::getInstruction()
{
	inter.CleanInstruction();
	string command = inter.GetInstruction();
	validinstruction = -1;
//	cout << command << endl;
	validinstruction = inter.HandleInstruction(command);
	if (validinstruction != SUCCESS)
		HandleException(validinstruction);
	else
		tree = inter.GetParseTree();
	return;
}

void API::getInstruction(fstream & iff)
{
	inter.CleanInstruction();
	string command = inter.GetInstruction(iff);
	validinstruction = -1;
//	cout << command << endl;
	validinstruction = inter.HandleInstruction(command);
	if (validinstruction != SUCCESS)
		HandleException(validinstruction);
	else
		tree = inter.GetParseTree();
	return;
}

void API::HandleException(int valid)
{
	std::cout << "Syntax Error, please try again" << endl;
}

void API::InstructionExcute()
{
	Data datas;
	RC code;
	if (tree.opcode == QUIT)
	{
		cout << "Bye" << endl;
		exit(0);
	}
	else if (tree.opcode == CRETABLE)
	{
		tree.tableinfo.attributenum = tree.attributes.size();
		code = cat.createTable(tree.table_name, tree.attributes, tree.primarykey, tree.attributes.size());
		if (code == -1)
		{
			validinstruction = CATALOG_TABLE_NAMECONFLICT;
			return;
		}
		else if (code == 0)
		{
			validinstruction = CATALOG_TABLE_NOSPACE;
			return;
		}
		else if (code == SUCCESS)
			validinstruction = SUCCESS;
		code = rec.tableCreate(tree.table_name);
	/*	if (code == 0)
		{
			validinstruction = CATALOG_TABLE_CANTOPEN;
			//continue;
		}
		else validinstruction = SUCCESS; */
	}
	else if (tree.opcode == CREINDEX)
	{
		code = cat.createIndex(tree.index_name, tree.table_name, tree.attr_name);
		if (code == -1)
		{
			validinstruction = CATALOG_INDEX_NAMECONFLICT;
			return;
		}
		else if (code == 0)
		{
			validinstruction = CATALOG_INDEX_CANTOPEN;
			return;
		}
		else if (code == 2)
		{
			validinstruction = CATALOG_INDEX_NO_SUCH_TABLE;
			return;
		}
		else if (code == 3)
		{
			validinstruction = CATALOG_INDEX_NO_SUCH_ATTR;
			return;
		}
		rec.indexCreate(tree.index_name);

		tree.tableinfo.tableName = tree.table_name; // preparing the indexinfo and the tableinfo
		tree.tableinfo = cat.getTable(tree.table_name);
		string tableFileName = rec.tableFileNameGet(tree.table_name);
		FILE* fp = fopen(tableFileName.c_str(), "rb+");
		fseek(fp, 0, SEEK_END);
		int offset = ftell(fp) / BLOCK_SIZE;
		fclose(fp);
		for (int i = 0; i < tree.tableinfo.attributenum; i++)
		{
			tree.tableinfo.spacelen += tree.tableinfo.attributes[i].typeSize();
			if (tree.tableinfo.attributes[i].name == tree.attr_name)
				tree.indexinfo.spacelen = tree.tableinfo.attributes[i].typeSize();
		}
		tree.tableinfo.blocknumber = offset;
		//cout << tree.tableinfo.blocknumber;
		tree.indexinfo.attributeName = tree.attr_name;
		tree.indexinfo.indexName = tree.index_name;
		tree.indexinfo.tableName = tree.table_name;
		index.createIndex(tree.tableinfo, tree.indexinfo);	// do some convert
	}

	else if (tree.opcode == INSERT)
	{
		// get table
		// prepare a record
		// rec.insert
		tree.tableinfo.tableName = tree.table_name; // preparing the indexinfo and the tableinfo
		tree.tableinfo = cat.getTable(tree.table_name);
		string tableFileName = rec.tableFileNameGet(tree.table_name);
		vector<Condition> checkprimary;
		int primary = 0;
/*		FILE* fp = fopen(tableFileName.c_str(), "r");
		if (fp != NULL)
		{
			fseek(fp, 0, SEEK_END);
			int offset = ftell(fp) / BLOCK_SIZE;
			fclose(fp);
			for (int i = 0; i < tree.tableinfo.attributenum; i++)
			{
				tree.tableinfo.spacelen += tree.tableinfo.attributes[i].typeSize();
				if (tree.tableinfo.attributes[i].name == tree.attr_name)
					tree.indexinfo.spacelen = tree.tableinfo.attributes[i].typeSize();
				if (tree.tableinfo.attributes[i].key_type == TYPEPRIMARY)
				{
					Condition p(tree.tableinfo.attributes[i].name, tree.rowdata.attribute[i], 0);
					checkprimary.push_back(p);
				}

			}
			tree.tableinfo.blocknumber = offset;
			primary = rec.recordAllFind(tree.table_name, tree.tableinfo.attributes, &checkprimary);
		}
*/		
//		cout << tree.tableinfo.attributes[0].key_type << endl;
		Table tmptable = cat.getTable(tree.table_name);
		int indexid = -1;
		if (tmptable.Index_on_table.size() > 0)
		{
			Index ind = cat.getIndex(tmptable.Index_on_table[0].indexName);
			for (int i = 0; i < tmptable.attributenum; ++i)
				if (tmptable.attributes[i].name == tmptable.Index_on_table[0].attributeName)
					indexid = i;
			outputdata data = index.searchEqual(tmptable, ind, tree.rowdata.attribute[indexid]);
			if (data.Row.size() > 0) primary = 1;
		}
		if (primary == 1)
		{
			validinstruction = RECORD_PRIMARY_CONFLICT;
		}
		else {
			int check = -1;
			/*for (size_t i = 0; i < tree.tableinfo.attributenum; i++) {

				int tmp = tree.tableinfo.attributes[i].key_type;
				if (tmp == TYPEPRIMARY || tmp == TYPEUNIQUE) {
					Condition con(tree.tableinfo.attributes[i].name, tree.rowdata.attribute[i], 0);
					checkprimary.push_back(con);
					check = rec.recordAllShow(tree.table_name, tmptable.attributes, &checkprimary);
					if (check > 0) {
						validinstruction = RECORD_PRIMARY_CONFLICT;
						break;
					}
					checkprimary.clear();
				}
			}*/
			if (check <= 0) {
				const int len = tree.tableinfo.spacelen;
				BYTE tobeinsert[MAX_TABLE_LENGTH];
				int pos = 0;
				for (int i = 0; i < tree.rowdata.attribute.size(); ++i) {
					Table thistable = cat.getTable(tree.table_name);
					if (thistable.attributes[i].type == 0)	// int
					{
						int a;
						a = atoi(tree.rowdata.attribute[i].c_str());
						memcpy(tobeinsert + pos, &a, 4);
						pos += 4;
					}
					else if (thistable.attributes[i].type == -1) {
						float f;
						f = atof(tree.rowdata.attribute[i].c_str());
						memcpy(tobeinsert + pos, &f, 4);
						pos += 4;
					}
					else {
						memcpy(tobeinsert + pos, tree.rowdata.attribute[i].c_str(), thistable.attributes[i].typeSize());
						pos += thistable.attributes[i].typeSize();
					}
				}
				rec.recordInsert(tree.table_name, tobeinsert, pos);
			}
		}
	}

	else if (tree.opcode == DELETE)
	{
		vector<Condition> conditionvector;
		for (int i = 0; i < tree.conditions.size(); i++)
		{
			Condition con(tree.conditions[i].Attribute_value, tree.conditions[i].Constant_value, tree.conditions[i].comparator);
			//			con.value = tree.conditions[i].Constant_value;
			//			con.attributeName = tree.conditions[i].Attribute_value;
			//			con.operate = tree.conditions[i].comparator;
			conditionvector.push_back(con);
		}
		Table tmptable = cat.getTable(tree.table_name);
		rec.recordAllDelete(tree.table_name, tmptable.attributes, &conditionvector);
	}
	else if (tree.opcode == SELECT)
	{
		vector<Condition> conditionvector;
		for (int i = 0; i < tree.conditions.size(); i++)
		{
			Condition con(tree.conditions[i].Attribute_value, tree.conditions[i].Constant_value, tree.conditions[i].comparator);
//			cout << tree.conditions[i].Attribute_value << tree.conditions[i].Constant_value << tree.conditions[i].comparator << endl;
			//			con.value = tree.conditions[i].Constant_value;
			//			con.attributeName = tree.conditions[i].Attribute_value;
			//			con.operate = tree.conditions[i].comparator;
			conditionvector.push_back(con);
		}
		Table tmptable = cat.getTable(tree.table_name);
		if (tmptable.tableName == "")
		{
			validinstruction = CATALOG_INDEX_NO_SUCH_TABLE;
			return;
		}
		for (int i = 0; i < tmptable.attributenum; i++)
			tmptable.spacelen += tmptable.attributes[i].typeSize();
		//cout << tmptable.spacelen << endl;
		//cout << tmptable.Index_on_table.size() << endl;
		if (tmptable.Index_on_table.size() == 1 && conditionvector.size() == 1 && \
			tmptable.Index_on_table[0].attributeName == conditionvector[0].attributeName) // we can use index
		{

			Index ind = cat.getIndex(tmptable.Index_on_table[0].indexName);
			outputdata data = index.searchEqual(tmptable, ind, conditionvector[0].value);
			if (data.Row.size() > 0)
				Displaydata(data,tmptable);
			else validinstruction = RECORD_NO_SUCH_RECORD;
		}
		else rec.recordAllShow(tree.table_name, tmptable.attributes, &conditionvector);
	}
	else if (tree.opcode == DROPINDEX)
	{
		cat.dropIndex(tree.index_name);
		rec.indexDrop(tree.index_name);
		index.dropIndex(cat.getIndex(tree.index_name));
	}
	else if (tree.opcode == DROPTABLE)
	{
		cat.dropTable(tree.table_name);
		rec.tableDrop(tree.table_name);
	}
	else if (tree.opcode == EXECFILE)
	{
		fstream iff(tree.filename);
		while (!iff.eof())
		{
			getInstruction(iff);
			InstructionExcute();
		}
		iff.close();
	}
	else if (tree.opcode == SHOWTABLE)
	{
		cat.printALLTableInfo();
	}
	else if (tree.opcode == CLEARBUFFER)
	{
		bm.clearBuffer();
	}
	return;
}

void API::ShowResult()
{
	switch (validinstruction)
	{
	case SUCCESS:
		cout << "Instruction executed." << endl;
		break;
	case RECORD_PRIMARY_CONFLICT:
		cout << "No duplicated unique key allowed." << endl;
		break;
	case RECORD_MAX_SIZE:
		cout << "Record max size exceeded." << endl;
		break;
	case CATALOG_INDEX_CANTOPEN:
		cout << "Can't open index." << endl;
		break;
	case CATALOG_INDEX_NAMECONFLICT:
		cout << "Name conflict in index." << endl;
		break;
	case CATALOG_INDEX_NO_SUCH_ATTR:
		cout << "No such attr in index creation." << endl;
		break;
	case RECORD_NO_SUCH_RECORD:
		cout << "There is no such recore." << endl;
		break;
	case CATALOG_INDEX_NO_SUCH_TABLE:
		cout << "There is no such table." << endl;
		break;
	default:
		cout << "Other ERROR." << endl;
		break;
	}

	return;
}

void API::Displaydata(outputdata data, Table tableinfo)
{
	for (int i = 0; i < data.Row.size(); ++i)
	{
		for (int j = 0; j < data.Row[i].attribute.size(); ++j)
		{
			//			cout << data.Row[i].attribute[j];
			if (tableinfo.attributes[j].type == 0)
			{
				//if the content is a int
				int tmp = *((int *)data.Row[i].attribute[j]);   //get content value by point
				printf("%d ", tmp);
			}
			else if (tableinfo.attributes[j].type == -1)
			{
				//if the content is a float
				float tmp = *((float *)data.Row[i].attribute[j]);   //get content value by point
				printf("%.2f ", tmp);
			}
			else
			{
				//if the content is a string
				string tmp = data.Row[i].attribute[j];
				printf("%s ", tmp.c_str());
			}
			free(data.Row[i].attribute[j]);
		}
		printf("\n");
	}
	return;
}