#include "stdafx.h"
#include "RecordManager.h"

extern BufferManager bm;
bool Condition::ifRight(int content)
{
    stringstream ss;
    ss << value;
    int myContent;
    ss >> myContent;
    
    switch (operate)
    {
        case Condition::OPERATOR_EQUAL:
            return content == myContent;
            break;
        case Condition::OPERATOR_NOT_EQUAL:
            return content != myContent;
            break;
        case Condition::OPERATOR_LESS:
            return content < myContent;
            break;
        case Condition::OPERATOR_MORE:
            return content > myContent;
            break;
        case Condition::OPERATOR_LESS_EQUAL:
            return content <= myContent;
            break;
        case Condition::OPERATOR_MORE_EQUAL:
            return content >= myContent;
            break;
        default:
            return true;
            break;
    }
}

bool Condition::ifRight(float content)
{
    stringstream ss;
    ss << value;
    float myContent;
    ss >> myContent;
    
    switch (operate)
    {
        case Condition::OPERATOR_EQUAL:
            return content == myContent;
            break;
        case Condition::OPERATOR_NOT_EQUAL:
            return content != myContent;
            break;
        case Condition::OPERATOR_LESS:
            return content < myContent;
            break;
        case Condition::OPERATOR_MORE:
            return content > myContent;
            break;
        case Condition::OPERATOR_LESS_EQUAL:
            return content <= myContent;
            break;
        case Condition::OPERATOR_MORE_EQUAL:
            return content >= myContent;
            break;
        default:
            return true;
            break;
    }
}

bool Condition::ifRight(string content)
{
    string myContent = value;
    switch (operate)
    {
        case Condition::OPERATOR_EQUAL:
            return content == myContent;
            break;
        case Condition::OPERATOR_NOT_EQUAL:
            return content != myContent;
            break;
        case Condition::OPERATOR_LESS:
            return content < myContent;
            break;
        case Condition::OPERATOR_MORE:
            return content > myContent;
            break;
        case Condition::OPERATOR_LESS_EQUAL:
            return content <= myContent;
            break;
        case Condition::OPERATOR_MORE_EQUAL:
            return content >= myContent;
            break;
        default:
            return true;
            break;
    }
}

Condition::Condition(string attribute,string value,int operate) {
    attributeName = attribute;
    this->value = value;
    this->operate = operate;
}

//create a table
int RecordManager::tableCreate(string tableName)
{
    string tableFileName = tableFileNameGet(tableName);
    FILE *fp;
    fp = fopen(tableFileName.c_str(), "rb+");
    if (fp == NULL)
    {
        return 0;
    }
    //create new blockNode
    BlockNode newb(tableFileName,0, false);
		fwrite(newb.BlockData, sizeof(BYTE), BLOCK_SIZE, fp);
		fclose(fp);
    return 1;
}

//drop a table
int RecordManager::tableDrop(string tableName)
{
    string tableFileName = tableFileNameGet(tableName);
    if (remove(tableFileName.c_str()))
    {
        return 0;
    }
    return 1;
}

//create a index
int RecordManager::indexCreate(string indexName)
{
    string indexFileName = indexFileNameGet(indexName);
    
    FILE *fp;
    fp = fopen(indexFileName.c_str(), "rb+");
    if (fp == NULL)
    {
        return 0;
    }
    //create new blockNode
    BlockNode newb(indexFileName,0, false);
		fwrite(newb.BlockData, sizeof(BYTE), BLOCK_SIZE, fp);
    fclose(fp);
    return 1;
}

//drop a index
int RecordManager::indexDrop(string indexName)
{
    string indexFileName = indexFileNameGet(indexName);
    if (remove(indexFileName.c_str()))
    {
        return 0;
    }
    return 1;
}

//insert a record to table
//@return int: the offset of the BlockNode(-1 represent error)
int RecordManager::recordInsert(string tableName,BYTE* record, int recordSize)
{
    string tableFileName = tableFileNameGet(tableName);
    BlockNode *btmp;
    FILE* fp = fopen(tableFileName.c_str(), "ab+");
	  if (fp == (FILE*)NULL) {
		  printf("Error with opening the file!\n");
		  return -1;
	  }
	  else {
		  fseek(fp, 0, SEEK_END);
		  int offset = (ftell(fp) / BLOCK_SIZE) - 1 < 0 ? 0 : (ftell(fp) / BLOCK_SIZE) - 1;
		  btmp=bm.fetchBlock(tableFileName,offset);
		  while (true)
      {
        if (btmp == NULL)
        {
            return -1;
        }
        if (btmp->usingSize <= (BLOCK_SIZE - recordSize))
        {
            //if the rest space is enough
            BYTE *addressBegin;
            addressBegin=&(btmp->BlockData[btmp->usingSize]);
            memcpy(addressBegin, record, recordSize);
//			cout << "record size = " << recordSize << endl;
            btmp->usingSize += recordSize;
			btmp->dirty = true;
//            bm.writeDirty(*btmp);
			fclose(fp);
            return btmp->offset;
        }
        else//need to create new BlockNode
        {
            offset = bm.newBlock(tableFileName);
//			cout << offset << endl;
			BlockNode * tmpblock = bm.fetchBlock(tableFileName, offset);
			btmp->nextBlock = tmpblock;
			tmpblock->preBlock = btmp;
 //           btmp=bm.fetchBlock(tableFileName,offset);
			btmp = tmpblock;
		//	cout << 
        }
      }
		  
		  fclose(fp);
	  }
    
    return -1;
}

//print all record of a table meet requirement
//@return int: the number of the record meet requirements(-1 represent error)
int RecordManager::recordAllShow(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM], vector<Condition>* conditionVector)
{
    string tableFileName = tableFileNameGet(tableName);
    BlockNode *btmp;
    btmp=bm.fetchBlock(tableFileName,0);
	int filelength = bm.fileSize(tableFileName) - 1;
    int count = 0;
	int currentloc = 1;
    while (true)
    {
        if (btmp == NULL)
        {
            return -1;
        }
		if (currentloc >= filelength && btmp->nextBlock == NULL)
        {
//			cout << (btmp->offset)<<endl;
            int recordBlockNum = recordBlockShow(tableName,attributes, conditionVector, btmp);
            count += recordBlockNum;
            return count;
        }
        else
        {
            int recordBlockNum = recordBlockShow(tableName, attributes, conditionVector, btmp);
            count += recordBlockNum;
            btmp = bm.fetchBlock(tableFileName,(btmp->offset)+1);
        }
		currentloc++;
    }
    
    return -1;
}

//print record of a table in a block
//@return int: the number of the record meet requirements in the block(-1 represent error)
int RecordManager::recordBlockShow(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM], vector<Condition>* conditionVector, int blockOffset)
{
    string tableFileName = tableFileNameGet(tableName);
    BlockNode* block = bm.fetchBlock(tableFileName,blockOffset);
    if (block == NULL)
    {
        return -1;
    }
    else
    {
        return  recordBlockShow(tableName, attributes, conditionVector, block);
    }
}

//print record of a table in a block
//@return int: the number of the record meet requirements in the block(-1 represent error)
int RecordManager::recordBlockShow(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM], vector<Condition>* conditionVector, BlockNode* block)
{
    //if block is null, return -1
    if (block == NULL)
    {
        return -1;
    }
    int recordNum,i=0;
    int count = 0;
    string tableFileName = tableFileNameGet(tableName);
    BYTE* recordBegin = block->BlockData;
    int recordsize = recordSize(attributes);
    recordNum = BLOCK_SIZE / recordsize;
//	cout << "block using size = " << block->usingSize << endl;
//	cout << "record size = " << recordsize << endl;
    	
    while (i < recordNum)
    {
        if(recordConditionFit(recordBegin, recordsize, attributes, conditionVector))
        {
            count ++;
            recordPrint(recordBegin, recordsize, attributes, attributes);
            printf("\n");
        }
        
        recordBegin += recordsize;
        i++;
    }
    
    return count;
}

//find the number of all record of a table meet requirement
//@return int: the number of the record meet requirements(-1 represent error)
int RecordManager::recordAllFind(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM], vector<Condition>* conditionVector)
{
    string tableFileName = tableFileNameGet(tableName);
    BlockNode *btmp=bm.fetchBlock(tableFileName,0);
	int filelength = bm.fileSize(tableFileName) - 1;
	int count = 0;
	int currentloc = 1;
    while (true)
    {
        if (btmp == NULL)
        {
            return -1;
        }
        if (currentloc >= filelength && btmp->nextBlock==NULL)
        {
            int recordBlockNum = recordBlockFind(tableName, attributes, conditionVector, btmp);
            count += recordBlockNum;
            return count;
        }
        else
        {
            int recordBlockNum = recordBlockFind(tableName, attributes, conditionVector, btmp);
            count += recordBlockNum;
            btmp = bm.fetchBlock(tableFileName,(btmp->offset)+1);
        }
		currentloc++;
    }
    
    return -1;
}

//find the number of record of a table in a block
//@return int: the number of the record meet requirements in the block(-1 represent error)
int RecordManager::recordBlockFind(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM], vector<Condition>* conditionVector, BlockNode* block)
{
    //if block is null, return -1
    if (block == NULL)
    {
        return -1;
    }
    int recordNum,i=0;
    int count = 0;
    string tableFileName = tableFileNameGet(tableName);
    BYTE* recordBegin = block->BlockData;
    int recordsize = recordSize(attributes);
	recordNum = BLOCK_SIZE / recordsize;
    
    while (i < recordNum)
    {
        //if the recordBegin point to a record
        
        if(recordConditionFit(recordBegin, recordsize, attributes, conditionVector))
        {
            count++;
        }
        
        recordBegin += recordsize;
        i++;
    }
    
    return count;
}

//delete all record of a table meet requirement
//@return int: the number of the record meet requirements(-1 represent error)
int RecordManager::recordAllDelete(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM], vector<Condition>* conditionVector)
{
    string tableFileName = tableFileNameGet(tableName);
    BlockNode *btmp=bm.fetchBlock(tableFileName,0);
	int filelength = bm.fileSize(tableFileName) - 1;
	int count = 0;
	int currentloc = 1;
    while (true)
    {
        if (btmp == NULL)
        {
            return -1;
        }
        if (currentloc >= filelength && btmp->nextBlock==NULL)
        {
			btmp->dirty = true;
            int recordBlockNum = recordBlockDelete(tableName, attributes, conditionVector, btmp);
            count += recordBlockNum;
            return count;
        }
        else
        {
			btmp->dirty = true;
            int recordBlockNum = recordBlockDelete(tableName, attributes, conditionVector, btmp);
            count += recordBlockNum;
            btmp = bm.fetchBlock(tableFileName,(btmp->offset)+1);
        }
		currentloc++;
    }
    
    return -1;
}

//delete record of a table in a block
//@return int: the number of the record meet requirements in the block(-1 represent error)
int RecordManager::recordBlockDelete(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM],  vector<Condition>* conditionVector, int blockOffset)
{
    string tableFileName = tableFileNameGet(tableName);
    BlockNode *btmp=bm.fetchBlock(tableFileName,blockOffset);
    if (btmp == NULL)
    {
        return -1;
    }
    else
    {
        return  recordBlockDelete(tableName, attributes, conditionVector, btmp);
    }
}

//delete record of a table in a block
//@return int: the number of the record meet requirements in the block(-1 represent error)
int RecordManager::recordBlockDelete(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM],  vector<Condition>* conditionVector, BlockNode* block)
{
    //if block is null, return -1
    if (block == NULL)
    {
        return -1;
    }
    int recordNum,i=0;
    int count = 0;
    string tableFileName = tableFileNameGet(tableName);
    BYTE* recordBegin = block->BlockData;
    int recordsize = recordSize(attributes);
//    recordNum = block->usingSize / recordsize;
	recordNum = BLOCK_SIZE / recordsize;
    while (i < recordNum)
    {
        //if the recordBegin point to a record
        
        if(recordConditionFit(recordBegin, recordsize, attributes, conditionVector))
        {
            count ++;
            
            int temp = 0;
            for (temp = 0; temp < (recordNum-i-1) * recordsize ; temp++)
            {
                recordBegin[temp] = recordBegin[temp + recordsize];
            }
            memset(recordBegin + temp, 0, recordsize);
            block->usingSize -= recordsize;
        }
        else
        {
            recordBegin += recordsize;
        }
        i++;
    }
  /*  if (block->usingSize==0){// if all the records of this block are deleted
    	 block->preBlock->nextBlock = block->nextBlock;
    	 if (block->nextBlock != NULL)
    	 	 block->nextBlock->preBlock = block->preBlock;
    	 BlockNode* btmp=block;
    	 while(btmp->nextBlock != NULL){
    	 	 btmp=btmp->nextBlock;
    	 	 btmp->offset -= 1;
    	 
    	 }
    }
    else*/
    	 bm.writeDirty(*block);
    return count;
}

//print all records of certain attributes of a table
//@return int: the number of the record meet requirements(-1 represent error)
int RecordManager::recordChosePrint(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM],Attribute attributesPrint[MAX_ATTRIBUTE_NUM])
{
    string tableFileName = tableFileNameGet(tableName);
    BlockNode *btmp;
    btmp=bm.fetchBlock(tableFileName,0);
        
    int count = 0;
    while (true)
    {
        if (btmp == NULL)
        {
            return -1;
        }
        if (btmp->nextBlock==NULL)
        {
            int recordBlockNum = recordBlockChosePrint(tableName,attributes, attributesPrint, btmp);
            count += recordBlockNum;
            return count;
        }
        else
        {
            int recordBlockNum = recordBlockChosePrint(tableName, attributes, attributesPrint, btmp);
            count += recordBlockNum;
            btmp = bm.fetchBlock(tableFileName,(btmp->offset)+1);
        }
    }
    
    return -1;
}

//print record of certain attributes of a table in a block
//@return int: the number of the record meet requirements in the block(-1 represent error)
int RecordManager::recordBlockChosePrint(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM], Attribute attributesPrint[MAX_ATTRIBUTE_NUM], int blockOffset)
{
    string tableFileName = tableFileNameGet(tableName);
    BlockNode* block = bm.fetchBlock(tableFileName,blockOffset);
    if (block == NULL)
    {
        return -1;
    }
    else
    {
        return  recordBlockChosePrint(tableName, attributes, attributesPrint, block);
    }
}

//print record of certain attributes of a table in a block
//@return int: the number of the record meet requirements in the block(-1 represent error)
int RecordManager::recordBlockChosePrint(string tableName, Attribute attributes[MAX_ATTRIBUTE_NUM],Attribute attributesPrint[MAX_ATTRIBUTE_NUM], BlockNode* block)
{
    //if block is null, return -1
    if (block == NULL)
    {
        return -1;
    }
    int recordNum,i=0;
    int count = 0;
    string tableFileName = tableFileNameGet(tableName);
    BYTE* recordBegin = block->BlockData;
    int recordsize = recordSize(attributes);
    recordNum = block->usingSize / recordsize;
    
    	
    while (i < recordNum)
    {
        recordPrint(recordBegin, recordsize, attributes, attributesPrint);
        printf("\n");
        recordBegin += recordsize;
        i++;
    }
    
    return recordNum;
}



//compute the recordSize
int RecordManager::recordSize(Attribute attributes[MAX_ATTRIBUTE_NUM])
{
	  int i,recordSize=0;
	  for (i=0 ; i < MAX_ATTRIBUTE_NUM ; i++)
	  {
	      if (attributes[i].name.length() == 0)
	      	  return recordSize;
	      switch (attributes[i].type)
	      {
	          case 0:
	          case -1:
	          	recordSize += 4;
	          	break;
	          default:
	            recordSize += attributes[i].type;
	            break;
	      }
	  }
	  return recordSize;
}

//judge if the record meet the requirement
//@return bool: if the record fit the condition
bool RecordManager::recordConditionFit(BYTE* recordBegin,int recordSize, Attribute attributes[MAX_ATTRIBUTE_NUM],vector<Condition>* conditionVector)
{
    if (conditionVector == NULL) {
        return true;
    }
    int alreadySize=0,i;
    char content[RECORD_MAX_SIZE];
    char contentTemp[COTENT_MAX_SIZE];
    BYTE *contentBegin = recordBegin;
	bool emptyflag = true;
    for (i=0; i < recordSize ; i++)
    {
        content[i] = contentBegin[i];
		if (content[i] != 0) emptyflag = false;
    }
	if (emptyflag) return false;
    
    for (i=0 ; attributes[i].name.length() != 0 && i < MAX_ATTRIBUTE_NUM ; i++)
    {
    	  for (int j=0 ; j<(*conditionVector).size() ; j++)
    	  {
    	  	  if (attributes[i].name==(*conditionVector)[j].attributeName){
    	  	      //if this attribute need to deal about the condition
    	  	      memset(contentTemp, 0, COTENT_MAX_SIZE);
                memcpy(contentTemp, content+alreadySize, attributes[i].typeSize());
                if(!contentConditionFit(contentTemp,attributes[i].type, &(*conditionVector)[j]))
                {
                    //if this record is not fit the conditon
                    return false;
                }
            }
    	  }	
    	  //alreadySize means the size of attributes that have been handled 
    	  alreadySize += attributes[i].typeSize();
    }
    
    return true;
}

//print value of record
//@param attributesPrint : the attributes wanted to print
void RecordManager::recordPrint(BYTE* recordBegin, int recordSize,Attribute attributes[MAX_ATTRIBUTE_NUM],Attribute attributesPrint[MAX_ATTRIBUTE_NUM])
{
    int alreadySize=0,i;
    char content[RECORD_MAX_SIZE];
    char contentTemp[COTENT_MAX_SIZE];
    BYTE *contentBegin = recordBegin;
	memset(contentTemp, 0, COTENT_MAX_SIZE);
    for (i=0; i < recordSize ; i++)
    {
		content[i] = contentBegin[i];
    }
    for (i=0 ; attributes[i].name.length() != 0 && i < MAX_ATTRIBUTE_NUM ; i++)
    {
    	  for (int j=0 ; attributesPrint[j].name.length() != 0 && j < MAX_ATTRIBUTE_NUM ; j++)
    	  {
    	  	  if (attributes[i].name == attributesPrint[j].name){
    	  	      //if this attribute need to print
    	  	      memset(contentTemp, 0, COTENT_MAX_SIZE);
                memcpy(contentTemp, content+alreadySize, attributes[i].typeSize());
				/* DEBUG */
//				cout << attributes[i].typeSize() << " " << attributes[i].name << endl;
					contentPrint(contentTemp, attributes[i].type);
            }
    	  }	
    	  //alreadySize means the size of attributes that have been handled 
    	  alreadySize += attributes[i].typeSize();
    }
    
   
}

//print value of content
void RecordManager::contentPrint(char* content, int type)
{
    if (type == 0)
    {
        //if the content is a int
        int tmp = *((int *) content);   //get content value by point
        printf("%d ", tmp);
    }
    else if (type == -1)
    {
        //if the content is a float
        float tmp = *((float *) content);   //get content value by point
        printf("%.2f ", tmp);
    }
    else
    {
        //if the content is a string
		string tmp = content;
        printf("%s ", tmp.c_str());
    }

}

//judge if the content meet the requirement
//@return bool: the content if meet
bool RecordManager::contentConditionFit(char* content,int type,Condition* condition)
{
    if (type == 0)
    {
        //if the content is a int
        int tmp = *((int *) content);   //get content value by point
        return condition->ifRight(tmp);
    }
    else if (type == -1)
    {
        //if the content is a float
        float tmp = *((float *) content);   //get content value by point
        return condition->ifRight(tmp);
    }
    else
    {
        //if the content is a string
        return condition->ifRight(content);
    }
    return true;
}

//get a table's file name
string RecordManager::tableFileNameGet(string tableName)
{
    string tmp = "";
    return tmp + "TABLE_FILE_" + tableName;
}

//get a index's file name
string RecordManager::indexFileNameGet(string indexName)
{
    string tmp = "";
    return indexName + ".index";
}

