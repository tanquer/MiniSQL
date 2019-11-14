#include "stdafx.h"
#include "IndexManager.h"

string getKeyvalue(Table tableinfo, Index indexInfo, string row)
{
	string keyvalue;
	int sp = 0, fp = 0;
	for (int i = 0; i < tableinfo.attributenum; i++)
	{
		sp = fp;
		fp += tableinfo.attributes[i].typeSize();
		if (indexInfo.attributeName == tableinfo.attributes[i].name)
			break;
	}
	for (int i = sp; i < fp && row[i] != EMPTY; ++i) keyvalue += row[i];
	return keyvalue;
}

void IndexManager::createIndex(const Table tableInfo, Index indexInfo)
{
	string IndexFileName = indexInfo.indexName + ".index";
	fstream out(IndexFileName.c_str(), ios::out);
	out.close();
	int first = bm.newBlock(IndexFileName);
	BlockNode* genesisblock = bm.fetchBlock(IndexFileName, first);
	(*genesisblock).BlockData[0] = 'R';
	(*genesisblock).BlockData[1] = 'L';
	(*genesisblock).offset = 0;
	(*genesisblock).fileName = IndexFileName;
	memset((*genesisblock).BlockData + 2, '0', 4);
	for (int i = 0; i < 2; ++i)
		memset((*genesisblock).BlockData + 6 + INDEX_POINTER_SIZE * i, '0', INDEX_POINTER_SIZE);
	//increase index's block
	int length = tableInfo.spacelen; //
	string TableFileName = "TABLE_FILE_"+tableInfo.tableName;
	int recordnum = BLOCK_SIZE / length;

	for (int blockoffset = 0; blockoffset < tableInfo.blocknumber; blockoffset++)
	{
		BlockNode* tableblock = bm.fetchBlock(TableFileName, blockoffset);
		for (int offset = 0; offset < recordnum; offset++)
		{
			int pos = offset * length;
			string row;
			for (int i = pos; i < pos + length; ++i)
				row += (*tableblock).BlockData[i];
//			if (row[0] == EMPTY) continue;
//			row.erase(row.begin());
			string  key = getKeyvalue(tableInfo, indexInfo, row);
			if (key == "") continue;
			IndexLeafNode node(key, blockoffset, offset);
			insertKey(indexInfo, node);
		}
	}
	return;
}

IndexBranchNode IndexManager::insertKey(Index indexInfo, IndexLeafNode node, int blockOffset)
{
	string IndexFileName = indexInfo.indexName + ".index";
	BlockNode* theNode = bm.fetchBlock(IndexFileName, blockOffset);
	bool isleaf = (*theNode).BlockData[1] == 'L';
	if (isleaf)
	{
		BPlusTreeLeafNode leaf(theNode, indexInfo);
		leaf.insert(node);
		int RecordLength = indexInfo.spacelen + INDEX_POINTER_SIZE * 2;
		int Maxrecordnum = (BLOCK_SIZE - 6 - INDEX_POINTER_SIZE * 2) / RecordLength;
		if (leaf.recordnum > Maxrecordnum) // split;
		{
			if (leaf.isRoot)
			{
				BlockNode* newrootblock = leaf.ptrtoblock;
				int newleafoffset = bm.newBlock(IndexFileName);
				int siblingoffset = bm.newBlock(IndexFileName);
				BlockNode* newleafnode = bm.fetchBlock(IndexFileName, newleafoffset);
				BlockNode* siblingnode = bm.fetchBlock(IndexFileName, siblingoffset);
				// int siblingbufferNum = get a buffer's number
				// leaf.bufferNum = get a buffer's number
				BPlusTreeBranchNode newroot(newrootblock);
				leaf.ptrtoblock = newleafnode;
				BPlusTreeLeafNode newsibling(siblingnode);

				newroot.isRoot = true;
				leaf.isRoot = 0;
				newsibling.isRoot = 0;

				newroot.ptrtoFather = 0;
				leaf.ptrtoFather = 0;
				newsibling.ptrtoFather = 0;

				newroot.width = leaf.width;
				newsibling.width = leaf.width;

				while (newsibling.nodelist.size() < leaf.nodelist.size()) {
					IndexLeafNode tnode = leaf.pop();
					newsibling.insert(tnode);
				}

				IndexBranchNode tnode;
				tnode.key = newsibling.getfront().key;
				tnode.ptrtoChild = (*(newsibling.ptrtoblock)).offset;// block offset
				newroot.insert(tnode);
				tnode.key = leaf.getfront().key;
				tnode.ptrtoChild = (*(leaf.ptrtoblock)).offset;
				newroot.insert(tnode);
				/* insert the key to the node(non-leaf) */
			}
			else	// leaf needs split
			{
				//bufferNum
				int siblingoffset = bm.newBlock(IndexFileName);
				BlockNode* siblingnode = bm.fetchBlock(IndexFileName, siblingoffset);
				BPlusTreeLeafNode newsibling(siblingnode);
				newsibling.isRoot = 0;
				newsibling.ptrtoFather = leaf.ptrtoFather;
				newsibling.width = leaf.width;
				// adjust sibling
				newsibling.nextSibling = leaf.nextSibling;
				leaf.nextSibling = (*(newsibling.ptrtoblock)).offset;// newsibling ' s block
				while (newsibling.nodelist.size() < leaf.nodelist.size()) {
					IndexLeafNode tnode = leaf.pop();
					newsibling.insert(tnode);
				}
				IndexBranchNode Nodetoreturn;
				Nodetoreturn.key = newsibling.getfront().key;
				Nodetoreturn.ptrtoChild = leaf.nextSibling; // new node 's block number
				return Nodetoreturn;
			}
		}
		else
			return IndexBranchNode();
	}
	else
	{
		BPlusTreeBranchNode branch(theNode, indexInfo);
		vector<IndexBranchNode>::iterator i = branch.nodelist.begin();
		if ((*i).key > node.key) {
			(*i).key = node.key;
		}
		else
		{
			while (i != branch.nodelist.end() &&(*i).key < node.key ) i++;
			i--;
		}
		IndexBranchNode tnode = insertKey(indexInfo, node, (*i).ptrtoChild);
		if (tnode.key == "") return IndexBranchNode();
		else	// split happened
		{
			branch.insert(tnode);
			int RecordLength = indexInfo.spacelen + INDEX_POINTER_SIZE;	// ptr to son
			int Maxrecordnum = (BLOCK_SIZE - 6 - INDEX_POINTER_SIZE) / RecordLength;
			/* Branch's split */
			return IndexBranchNode();
		}
	}
	return IndexBranchNode();
}

outputrow IndexManager::splitRow(Table tableinfo, char* Row)
{
	int sp = 0, fp = 0;
	outputrow splitrow;
	
	for (int i = 0; i < tableinfo.attributenum; i++)
	{
		sp = fp;
		fp += tableinfo.attributes[i].typeSize();
		char* columndata = (char*)malloc(sizeof(char) * tableinfo.attributes[i].typeSize());
	/*	for (int j = sp; j < fp; ++j)
			if (Row[j] == EMPTY) break;
			else columndata += Row[j];*/
		memcpy(columndata, Row + sp, tableinfo.attributes[i].typeSize());
			splitrow.attribute.push_back(columndata);
	}
	return splitrow;
}

outputdata IndexManager::searchEqual(const Table& tableInfo, const Index& indexInfo, string key, int blockOffset)
{
	outputdata datas;
	string IndexFileName = indexInfo.indexName + ".index";
	BlockNode* theNode = bm.fetchBlock(IndexFileName, blockOffset);// get the buffer's number
	bool isleaf = (*theNode).BlockData[1] == 'L'; // judge if it is leaf
	if (isleaf)
	{
		BPlusTreeLeafNode leaf(theNode,indexInfo);
		vector<IndexLeafNode>::iterator iter = leaf.nodelist.begin();
		for (iter = leaf.nodelist.begin(); iter != leaf.nodelist.end(); ++iter)
			if ((*iter).key == key) {
				//µΩtable÷–»°
				string tablename = "TABLE_FILE_" +indexInfo.tableName;
				//cout << (*iter).blockoffset << endl;
				BlockNode* tableblock = bm.fetchBlock(tablename, (*iter).blockoffset);
				int position = (tableInfo.spacelen) * ((*iter).fileoffset);
				char* rowdata = (char*)malloc(MAX_TABLE_LENGTH);
				/*for (int i = position; i < position + tableInfo.spacelen; ++i)
					rowdata += (*tableblock).BlockData[i];*/
				memcpy(rowdata, (*tableblock).BlockData + position, tableInfo.spacelen);
//				if (rowdata.c_str()[0] != EMPTY)
//				{
//					rowdata.erase(rowdata.begin());
					outputrow thisdata = splitRow(tableInfo, rowdata);
					datas.Row.push_back(thisdata);
					free(rowdata);
					return datas;
//				}
			}
	}
	else
	{
		//BPlusTreeBranchNode branch
		BPlusTreeBranchNode branch(theNode, indexInfo);
		vector<IndexBranchNode>::iterator i = branch.nodelist.begin();
		for (i = branch.nodelist.begin(); i != branch.nodelist.end(); i++)
		{
			if ((*i).key > key)
			{
				i--;
				break;
			}
		}
		if (i == branch.nodelist.end()) i--;
		datas = searchEqual(tableInfo, indexInfo, key, (*i).ptrtoChild);
	}
	return datas;
}

void IndexManager::dropIndex(Index indexInfo)
{
	string filename = indexInfo.indexName + ".index";
	bm.deleteFile(filename);
	return;
}

int BPlusTreeNode::getRecordcount()
{
	int record = 0;
	for (int i = 2; i < 6; ++i)
	{
		if ((*ptrtoblock).BlockData[i] == EMPTY) break;
		record = 10 * record + (*ptrtoblock).BlockData[i] - '0';
	}
	return record;
}

int BPlusTreeNode::getPtr(int pos) {
	int pointer = 0;
	for (int i = pos; i < pos + INDEX_POINTER_SIZE; ++i)
		pointer = pointer * 10 + (*ptrtoblock).BlockData[i] - '0';
	return pointer;
}


BPlusTreeLeafNode::~BPlusTreeLeafNode()
{
	if (isRoot)
		(*ptrtoblock).BlockData[0] = 'R';
	else
		(*ptrtoblock).BlockData[0] = 'X';
	(*ptrtoblock).BlockData[1] = 'L';
	string record = to_string(recordnum);
	while (record.length() < 4)
		record = "0" + record;
	memcpy((*ptrtoblock).BlockData + 2, record.c_str(), 4 * sizeof(BYTE));

	int position = 6;
	string ptrfather = to_string(ptrtoFather);
	while (ptrfather.length() < INDEX_POINTER_SIZE)
		ptrfather = "0" + ptrfather;
	memcpy((*ptrtoblock).BlockData + position, ptrfather.c_str(), INDEX_POINTER_SIZE * sizeof(BYTE));
	position += INDEX_POINTER_SIZE;

	string nextsib = to_string(nextSibling);
	while (nextsib.length() < INDEX_POINTER_SIZE)
		nextsib = "0" + nextsib;
	memcpy((*ptrtoblock).BlockData + position, nextsib.c_str(), INDEX_POINTER_SIZE * sizeof(BYTE));
	position += INDEX_POINTER_SIZE;

	for (int i = 0; i < recordnum; ++i)
	{
		int ptr = nodelist[i].fileoffset;
		string pointer = to_string(ptr);
		while (pointer.length() < INDEX_POINTER_SIZE)
			pointer = "0" + pointer;
		memcpy((*ptrtoblock).BlockData + position, pointer.c_str(), INDEX_POINTER_SIZE * sizeof(BYTE));
		position += INDEX_POINTER_SIZE;

		ptr = nodelist[i].blockoffset;
		pointer = to_string(ptr);
		while (pointer.length() < INDEX_POINTER_SIZE)
			pointer = "0" + pointer;
		memcpy((*ptrtoblock).BlockData + position, pointer.c_str(), INDEX_POINTER_SIZE * sizeof(BYTE));
		position += INDEX_POINTER_SIZE;

		memcpy((*ptrtoblock).BlockData + position, this->nodelist[i].key.c_str(), this->nodelist[i].key.length() * sizeof(BYTE));
		for (int j = nodelist[i].key.length(); j < width; ++j)
			(*ptrtoblock).BlockData[position + j] = 0;
		position += width;
	}

}

BPlusTreeBranchNode::~BPlusTreeBranchNode()
{
	if (isRoot)
		(*ptrtoblock).BlockData[0] = 'R';
	else (*ptrtoblock).BlockData[0] = 'X';
	(*ptrtoblock).BlockData[1] = 'X';
	string record = to_string(recordnum);
	while (record.length() < 4)
		record = "0" + record;
	memcpy((*ptrtoblock).BlockData + 2, record.c_str(), 4 * sizeof(BYTE));
	int position = 6;
	string ptrfather = to_string(ptrtoFather);
	while (ptrfather.length() < INDEX_POINTER_SIZE)
		ptrfather = "0" + ptrfather;
	memcpy((*ptrtoblock).BlockData + position, ptrfather.c_str(), INDEX_POINTER_SIZE * sizeof(BYTE));
	position += INDEX_POINTER_SIZE;
	for (int i = 0; i < recordnum; ++i)
	{
		int ptr = nodelist[i].ptrtoChild;
		string pointer = to_string(ptr);
		while (pointer.length() < INDEX_POINTER_SIZE)
			pointer = "0" + pointer;
		memcpy((*ptrtoblock).BlockData + position, pointer.c_str(), INDEX_POINTER_SIZE * sizeof(BYTE));
		position += INDEX_POINTER_SIZE;
		memcpy((*ptrtoblock).BlockData + position, this->nodelist[i].key.c_str(), this->nodelist[i].key.length() * sizeof(BYTE));
		for (int j = nodelist[i].key.length(); j < width; ++j)
			(*ptrtoblock).BlockData[position + j] = 0;
		position += width;
	}

}
