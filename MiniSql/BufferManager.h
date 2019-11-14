#pragma once
#include<iostream>
#include<cstdio>
#include<fstream>
#include<vector>
#include"sql.h"
using namespace std;

class BufferManager {
public:
	BufferManager();
	~BufferManager();
	BlockNode* fetchBlock(string fileName, int offset);//in which file, offset num(how many blocks)
	int newBlock(string fileName);//add a block to the end of the file.
	bool newFile(string fileName);//create a file
	bool deleteFile(string fileName);//delete a file
	int bgetNewloc();// LRU (clock). return the location to replace
	bool setpin(BlockNode& block);//check whether all nodes are pinned, and set pin the block
	bool removepin(BlockNode& block);//
	int fileSize(string fileName);//return how many blocks are there in ths file
	bool writeAll();
	bool writeDirty(BlockNode block);//write this block back to the file
	void clearBuffer();
private:
	BlockNode blockList[MAX_BLOCK_NUM];
	int tail, referenceloc, pinnum;//pinnum是被pin的节点的数量（为了防止所有都被pin）
	bool initblock(BlockNode& bnode,string fileName,int offset);
	inline int nexti(int curi){if(curi==MAX_BLOCK_NUM-1) return 0; else return curi+1;}
};



