#include "stdafx.h"
#include"BufferManager.h"

void error(string content)
{
	//cout << content << endl;
}

BufferManager::BufferManager()
{
	for (int i = 0; i < MAX_BLOCK_NUM; i++) {
		BlockNode newblock;
		blockList[i] = newblock;
	}
	tail=referenceloc=0;
}

BufferManager::~BufferManager()
{
	writeAll();
}

bool BufferManager::initblock(BlockNode& bnode,string fileName,int offset)
{
    bnode.dirty=bnode.pin=false;
    bnode.LRUtag=true;
    bnode.offset=offset;
    bnode.fileName=fileName;
	FILE* fp = fopen(fileName.c_str(), "rb+");
	if (fp == (FILE*)NULL) {
		//error("Error with opening the file!\n");
		return nullptr;
	}
	fseek(fp, offset*BLOCK_SIZE, SEEK_SET);
	fread(&bnode.BlockData, sizeof(BYTE), BLOCK_SIZE, fp);
	fclose(fp);
    return true;
}

//先在blockbuffer里面找，如果找到，直接返回
//如果没找到：
//blocklist未满——加在末尾
//blocklist已满——从上次reference指针的地方开始往后找替换的位置【clock的办法，pin=0且reference=1则替换，否则更改reference为0】
BlockNode* BufferManager::fetchBlock(string fileName, int offset)
{
    for(int i=0;i<tail;i++){
        if(blockList[i].fileName==fileName&&blockList[i].offset==offset){
            blockList[i].LRUtag=true;
            return &blockList[i];
        }
    }//block in the buffer, return it directly
    //block not in the buffer, fetch it from the file
    if(0 == tail){//empty
        initblock(blockList[tail],fileName,offset);
        FILE* fp=fopen(fileName.c_str(),"rb+");
        if(fp==(FILE*)NULL){
			//error("Error with opening the file!\n");
			return nullptr;
        }
        fseek(fp,offset*BLOCK_SIZE,SEEK_SET);
        fread(&blockList[tail].BlockData,sizeof(BYTE),BLOCK_SIZE,fp);
        int tmp=tail;
		tail++;
        fclose(fp);
        return &blockList[tmp];
    }
    else if(MAX_BLOCK_NUM == tail){//full. use the clock to replace the LRU block.
		int i = bgetNewloc();
		writeDirty(blockList[i]);
		initblock(blockList[i], fileName, offset);
		FILE* fp = fopen(fileName.c_str(), "rb+");
		if (fp == (FILE*)NULL) {
			//error("Error with opening the file!\n");
			return nullptr;
		}
		fseek(fp, offset*BLOCK_SIZE, SEEK_SET);
		fread(&blockList[i].BlockData, sizeof(BYTE), BLOCK_SIZE, fp);
		fclose(fp);
		blockList[i].dirty = true;
		return &blockList[i];
    }
    else{//
        initblock(blockList[tail],fileName,offset);
        FILE* fp=fopen(fileName.c_str(),"rb+");
        if(fp==(FILE*)NULL){
			//error("Error with opening the file!\n");
            return NULL;
        }
        fseek(fp,offset*BLOCK_SIZE,SEEK_SET);
        fread(&blockList[tail].BlockData,sizeof(BYTE),BLOCK_SIZE,fp);
        int tmp=tail;
		tail++;
		//cout << "Block " << tail << " used. " << endl;
        fclose(fp);
		blockList[tmp].dirty = true;
        return &blockList[tmp];
    }
}

bool BufferManager::writeDirty(BlockNode block)
{
    FILE* fp=fopen(block.fileName.c_str(),"rb+");
    if(fp==(FILE*)NULL){
        //error("Error with opening the file!\n");
        return false;
    }
	else {
		fseek(fp, block.offset*BLOCK_SIZE, SEEK_SET);
		fwrite(block.BlockData, sizeof(BYTE), BLOCK_SIZE, fp);
		fclose(fp);
		return true;
	}
}

bool BufferManager::writeAll()
{
	bool tmp = true;
	for (int i = 0; i < tail; i ++) {
		if (blockList[i].dirty == true) {
			tmp = writeDirty(blockList[i]);
			if (tmp == false) return false;
		}
		else {
			continue;
		}
	}
	return true;
}

int BufferManager::newBlock(string fileName)
{
	FILE* fp = fopen(fileName.c_str(), "rb+");
	if (fp == (FILE*)NULL) {
		//error("Error with opening the file!\n");
		return -1;
	}
	else {
		BlockNode newb;
		fseek(fp, 0, SEEK_END);
		int offset = (ftell(fp) / BLOCK_SIZE);////
		fwrite(newb.BlockData, sizeof(BYTE), BLOCK_SIZE, fp);
		fclose(fp);
		return offset;
	}
}

bool BufferManager::newFile(string fileName)
{
	FILE* fp = fopen(fileName.c_str(), "wb+");
	if (fp == (FILE*)NULL) {
		//error("Error with creating the file!\n");
		return false;
	}
	else {
		fclose(fp);
		return true;
	}
}

bool BufferManager::deleteFile(string fileName)
{
	remove(fileName.c_str());
	return true;
}

int BufferManager::fileSize(string fileName)
{
	FILE* fp = fopen(fileName.c_str(), "rb+");
	if (fp == (FILE*)NULL) {
		error("Error with open the file!\n");
		return -1;
	}
	else {
		fseek(fp, 0, SEEK_END);
		int offset = ftell(fp) / BLOCK_SIZE;
		fclose(fp);
		return offset+1;
	}
}

bool BufferManager::setpin(BlockNode& block)
{
	if (block.pin == false && tail == MAX_BLOCK_NUM+1 && pinnum == MAX_BLOCK_NUM - 1) {//检查是否所有的都被pin
		error("Illegal! All blocks are pinned!\n");
		return false;
	}
	else {
		if (block.pin == true) return true;
		else {
			block.pin = true;
			pinnum++;
		}
	}
	return true;
}

bool BufferManager::removepin(BlockNode& block)
{
	block.pin = false;
	return true;
}

int BufferManager::bgetNewloc()// LRU (clock)
{
	int tmp = referenceloc;
	referenceloc++;
	for (; referenceloc != tmp; referenceloc = nexti(referenceloc)) {
		if (blockList[referenceloc].pin == true) continue;
		if (blockList[referenceloc].LRUtag == true) {//遇到LRUtag是1的置为0
			blockList[referenceloc].LRUtag = false;
			continue;
		}
		else {											//遇到LRUtag是0的置为1并返回
			blockList[referenceloc].LRUtag = true;
			//cout << referenceloc << endl;
			return referenceloc;
		}
	}
	blockList[referenceloc].LRUtag = true;
	//cout << "block " << referenceloc << " substituted." << endl;
	return referenceloc;
}

void BufferManager::clearBuffer()
{
	writeAll();
	for (int i = 0; i < MAX_BLOCK_NUM; i++) {
		if (blockList[i].offset == -1) {//not used
			continue;
		}
		blockList[i].dirty = false;
		blockList[i].LRUtag = false;
		blockList[i].pin = false;
		blockList[i].offset = -1;
		blockList[i].fileName = "";
		memset(blockList[i].BlockData, 0, sizeof(BLOCK_SIZE));
	}
	tail = referenceloc = 0;
	return;
}

