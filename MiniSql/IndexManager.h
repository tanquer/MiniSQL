#pragma once
#include "BufferManager.h"
#include "sql.h"
#include "Structures.h"
#include <string>
#include <vector>

extern BufferManager bm;
class Data;
class IndexBranchNode;
class IndexLeafNode{
public:
	string key;
	int blockoffset, fileoffset;
	IndexLeafNode():
		key(""), blockoffset(0), fileoffset(0){}
	IndexLeafNode(string key, int blockoffset, int fileoffset):
		key(key), blockoffset(blockoffset), fileoffset(fileoffset){}
};

class IndexBranchNode{
public:
	string key;
	int ptrtoChild;
	IndexBranchNode():
		key(""), ptrtoChild(0){}
	IndexBranchNode(string key, int ptrC):
		key(key), ptrtoChild(ptrC){}
};

class BPlusTreeNode{
public:
	bool isRoot;
	int ptrtoFather, recordnum, width;
	BlockNode* ptrtoblock;
	BPlusTreeNode() {};
	~BPlusTreeNode() {};
	BPlusTreeNode(BlockNode* newblock) : ptrtoblock(newblock), recordnum(0) {};
	int getRecordcount();
	int getPtr(int pos);
};



class BPlusTreeBranchNode : public BPlusTreeNode
{
public:
	vector<IndexBranchNode> nodelist;
	BPlusTreeBranchNode() {}
	BPlusTreeBranchNode(BlockNode* newblock) : BPlusTreeNode(newblock) {};
	~BPlusTreeBranchNode();
	BPlusTreeBranchNode(BlockNode* bufferblock, Index indexinfo)
	{
		ptrtoblock = bufferblock;
		isRoot = (*ptrtoblock).BlockData[0] == 'R';
		int recordcount = getRecordcount();
		recordnum = 0;
		width = indexinfo.spacelen;
		ptrtoFather = getPtr(6);
		int position = 6 + INDEX_POINTER_SIZE;

		for (int i = 0; i < recordcount; ++i)
		{
			int ptrtochild = getPtr(position);
			position += INDEX_POINTER_SIZE;
			string key = "";
			for (int j = position; j < position + width; ++j)
			{
				if ((*ptrtoblock).BlockData[j] == EMPTY) break;
				else key += (*ptrtoblock).BlockData[j];
			}
			position += width;

			IndexBranchNode node(key, ptrtochild);
			insert(node);
		}
	}

	void insert(IndexBranchNode node)
	{
		recordnum++;
		vector<IndexBranchNode>::iterator iter = nodelist.begin();
		if (nodelist.size() == 0)
		{
			nodelist.insert(iter, node);
			return;
		}
		else
		{
			for (iter = nodelist.begin(); iter != nodelist.end(); iter ++ )
				if ((*iter).key > node.key) break;
		}
		nodelist.insert(iter, node);
	}
	IndexBranchNode pop(){
		recordnum -- ;
		IndexBranchNode temp = nodelist.back();
		nodelist.pop_back();
		return temp;
	}
	IndexBranchNode getfront(){
		return nodelist.front();
	}
};

class BPlusTreeLeafNode : public BPlusTreeNode
{
public:
	int nextSibling;
	vector<IndexLeafNode> nodelist;
	BPlusTreeLeafNode(BlockNode* newblock) : BPlusTreeNode(newblock) {};
	~BPlusTreeLeafNode();
	BPlusTreeLeafNode(BlockNode* bufferblock, Index indexinfo)
	{
		ptrtoblock = bufferblock;
		isRoot = (*ptrtoblock).BlockData[0] == 'R';
		int recordcount = getRecordcount();
		recordnum = 0;
		width = indexinfo.spacelen;
		ptrtoFather = getPtr(6);
		nextSibling = getPtr(6 + INDEX_POINTER_SIZE);
		int position = 6 + INDEX_POINTER_SIZE * 2;
		for (int i = 0; i < recordcount; ++i)
		{
			string key = "";
			int offsetinfile = getPtr(position);
			position += INDEX_POINTER_SIZE;
			int offsetinblock = getPtr(position);
			position += INDEX_POINTER_SIZE;
			for (int j = position; j < position + width; ++j)
			{
				if ((*ptrtoblock).BlockData[j] == EMPTY) break;
				else key += (*ptrtoblock).BlockData[j];
			}
			position += width;

			IndexLeafNode node(key, offsetinblock, offsetinfile);
			insert(node);
		}
	}
	void insert(IndexLeafNode node)
	{
		recordnum++;
		vector<IndexLeafNode>::iterator iter = nodelist.begin();
		if (nodelist.size() == 0)
		{
			nodelist.insert(iter, node);
			return;
		}
		else
		{
			for (iter = nodelist.begin(); iter != nodelist.end(); iter++)
				if ((*iter).key > node.key) break;
		}
		nodelist.insert(iter, node);
	}
	IndexLeafNode pop() {
		recordnum--;
		IndexLeafNode temp = nodelist.back();
		nodelist.pop_back();
		return temp;
	}
	IndexLeafNode getfront() {
		return nodelist.front();
	}
};




class IndexManager {
public:
  void createIndex(const Table tableInfo, Index indexInfo);
  void dropIndex(Index indexInfo);
  IndexBranchNode insertKey(Index indexInfo, IndexLeafNode node, int blockOffset = 0);
  outputdata searchEqual(const Table& tableInfo, const Index& indexInfo, string key, int blockOffset = 0);
private:
//	IndexBranchNode splitNode()
	outputrow splitRow(Table tableinfo, char* Row);
	
};

