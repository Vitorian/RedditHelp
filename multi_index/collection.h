#pragma once

#include <iostream>
#include "data.h"

using namespace std;

struct treeNode
{
	data * vendorData;
	treeNode * left;
	treeNode * right;

	treeNode(data * dataIn)
	{
		vendorData = dataIn;
		left = NULL;
		right = NULL;
	}
};

struct hashNode
{
	data * vendorData;
	hashNode * next;

	hashNode(data * dataIn)
	{
		vendorData = dataIn;
		next = NULL;
	}
};

class collection
{
	treeNode * root;
	hashNode ** table;
	int tableSize;
	int treeSize;
	int capacity;
	const static int DEFAULT_CAPACITY = 11;

	void addTable(data * dataIn);
	void addTree(data * dataIn);
	treeNode* addTreeHelper(data * dataIn, treeNode *& treeIn);
	int calculateHash(char * key) const;
	void copyTable(hashNode **& newTable, hashNode ** tableIn);
	void copyTree(treeNode *& newRoot, treeNode * treeIn);
	treeNode* deleteNode(treeNode *& root);
	void destroyTable();
	void destroyTree(treeNode *& treeIn);
	void displayNameHelper(treeNode * treeIn);
	void initializeTable();
	bool removeTable(char * nameIn);
	treeNode* removeTree(treeNode*& treeIn, char * nameIn, bool& delStatusIn);
	bool removeTree(treeNode * treeIn, char * nameIn);
	bool retrieveNameHelper(treeNode * treeIn, data& dataRtn, char * nameIn) const;
public:
	collection();
	collection(char * fileName);
	collection(const collection&);
	const collection& operator= (const collection&);
	~collection();
	void displayName();
	void displayProduct(void) const;
	int getTableSize();
	int getTreeSize();
	void insert(const data&);
	bool remove(char * nameIn);
	int retrieveProduct(data dataRtn[], char * productIn, int dataItemsRtn) const;
	bool retrieveName(data& dataIn, char * nameIn) const;
	void setLeftChild(treeNode * treeIn, treeNode * childIn);
	void setRightChild(treeNode * treeIn, treeNode  *childIn);
	void writeOut(char * fileName);
};
