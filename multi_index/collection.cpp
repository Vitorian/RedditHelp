/*******************
Privte data members:
treeNode * root;
hashNode ** table;
int treeSize;
int tableSize;

******************/

#include <iostream>
#include <cstring>
#include <fstream>
#include "collection.h"
#include "data.h"

using namespace std;


collection::collection() : capacity(DEFAULT_CAPACITY), root(NULL), treeSize(0), tableSize(0)
{
	initializeTable();
}

collection::collection(char * fileName) : capacity(DEFAULT_CAPACITY), root(NULL), treeSize(0), tableSize(0)
{
	data currData;

	initializeTable();

	char name[100] = {'\0'};
	char events[100] = {'\0'};
	char number[100] = {'\0'};
	char products[100] = {'\0'};
	char tempString[100] = {'\0'};

	ifstream inFile;
	inFile.open(fileName);
	inFile.peek();

	while (!inFile.eof() && !inFile.fail())
	{
		inFile.getline(name, 100, ';');
		currData.setName(name);
		currData.getName(tempString);

		inFile.getline(number, 100, ';');
		currData.setNumber(number);
		currData.getNumber(tempString);

		inFile.getline(products, 100, ';');
		currData.setProduct(products);
		currData.getProduct(tempString);

		inFile.getline(events, 100, ';');
		currData.setEvents(events);
		currData.getEvents(tempString);

		inFile.ignore(100, '\n');

		insert(currData);

		inFile.peek();
	}

	inFile.close();
}

collection::collection(const collection& collectionIn) :root(collectionIn.root), table(collectionIn.table), capacity(collectionIn.capacity), treeSize(collectionIn.treeSize), tableSize(collectionIn.tableSize)
{
	copyTable(table, collectionIn.table);
	copyTree(root, collectionIn.root);

	*this = collectionIn;
}

collection::~collection()
{
	destroyTree(root);
	destroyTable();
}

const collection& collection::operator= (const collection& collectionIn)
{
	if (this != &collectionIn)
	{
		destroyTable();
		copyTable(table, collectionIn.table);

		destroyTree(root);
		copyTree(root, collectionIn.root);
	}

	return *this;
}


void collection::addTable(data * dataIn)
{
	char key[100] = {'\0'};
	char compKey[100] = {'\0'};
	dataIn->getProduct(key); // Seg faults
	int index = calculateHash(key);

	hashNode * currNode = NULL;
	hashNode * newNode = new hashNode(dataIn);
	hashNode * prevNode = NULL;

	if (table[index] == NULL)
	{
		table[index] = newNode;
		newNode->next = NULL;
	}
	else
	{
	       	currNode = table[index];

		while (currNode != NULL)
		{
			currNode->vendorData->getProduct(compKey);

			if (strcmp(compKey,key) > 0 )
			{

				newNode->next = currNode;

				if (prevNode != NULL)
				{
					prevNode->next = newNode;
					break;
				}
				else
				{
					table[index] = newNode;
					break;
				}
			}
			else if (!currNode->next)
			{
				currNode->next = newNode;

				newNode->next = NULL;

				break;
			}
			else
			{
				prevNode = currNode;
				currNode = currNode->next;
			}
		}
	}

	tableSize++;

	return;
}

void collection::addTree(data * dataIn)
{
	if (dataIn)
	{
		root = addTreeHelper(dataIn,root);
	}
	else
		return;
}

treeNode* collection::addTreeHelper(data * dataIn, treeNode*& treeIn)
{
	treeNode * currTree = NULL;

	char compStringA[100] = {'\0'};
	char compStringB[100] = {'\0'};

	if (!root)
	{
		root = new treeNode(dataIn);
		root->left = NULL;
		root->right = NULL;
		treeSize++;
		return root;
	}
	else
	{
		currTree = treeIn;

		if (!currTree)
		{
			currTree = new treeNode(dataIn);
			currTree->left = NULL;
			currTree->right = NULL;
			treeSize++;
			return currTree;
		}
		else
		{

			data * currData = currTree->vendorData;

			dataIn->getName(compStringA);
			currData->getName(compStringB);

			if (strcmp(compStringA,compStringB) < 0)
			{
				currTree->left = addTreeHelper(dataIn,currTree->left);
			}
			else
			{
				currTree->right = addTreeHelper(dataIn,currTree->right);
			}

		}
	}
}

int collection::calculateHash(char * key) const
{
	char * temp = key;
	int total = 0;

	while (*temp)
	{
		total = total + *temp;
		temp++;
	}

	return total%capacity;
}

void collection::copyTable(hashNode ** newTable, hashNode ** tableIn)
{
	newTable = new hashNode*[capacity];

	for (int i = 0; i < capacity; i++)
	{
		if (tableIn[i] == NULL)
		{
			newTable[i] = NULL;
		}
		else
		{
			newTable[i] = new hashNode(tableIn[i]->vendorData);
			hashNode * srcNode = tableIn[i]->next;
			hashNode * destNode = newTable[i];

			while (srcNode)
			{
				destNode->next = new hashNode(srcNode->vendorData);
				destNode = destNode->next;
				srcNode = srcNode->next;
			}

			destNode->next = NULL;
		}
	}
}

void collection::copyTree(treeNode *& newRoot, treeNode * treeIn)
{
	if (treeIn)
	{
		newRoot = new treeNode(treeIn->vendorData);
		copyTree(newRoot->left, treeIn->left);
		copyTree(newRoot->right, treeIn->right);
	}
	else
	{
		newRoot = NULL;
	}
}

treeNode* collection::deleteNode(treeNode *& treeIn)
{
	treeNode * currNode = treeIn;
	treeNode * prevNode = NULL;

	data * currData;

	if (!treeIn)
	{
		return NULL;
	}
	else if (!treeIn->right && !treeIn->left)
	{

//		if (treeIn != root)
//			delete treeIn->vendorData; //Errors
		delete treeIn;
		treeIn = NULL;
		return treeIn;
	}
	else if (!treeIn->right)
	{

		currNode = treeIn->left;
		delete treeIn->vendorData;
		delete treeIn;
		treeIn = NULL;
		return currNode;
	}
	else if (!treeIn->left)
	{
		currNode = treeIn->right;
		delete treeIn->vendorData;
		delete treeIn;
		treeIn = NULL;
		return currNode;
	}
	else
	{

		currNode = treeIn->right;
		prevNode = NULL;

		while (currNode->left) // Find in-order successor
		{
			prevNode = currNode;
			currNode = currNode->left;
		}

		treeIn->vendorData = currNode->vendorData;

		if (!prevNode)
		{
			treeIn->right = currNode->right;
                }
                else
                {
                        prevNode->left = currNode->right;
                }

		currNode->right = NULL;

	    	delete currNode->vendorData;
	    	delete currNode;
		return treeIn;
	}

	treeSize--;
//	return root;
}

void collection::destroyTable()
{
	for (int i = 0; i < capacity; i++)
	{
		hashNode * head = table[i];
		hashNode * currNode = NULL;

		while(head)
		{
			currNode = head->next;
			head->next = NULL;
//			delete head->vendorData; // Error
			delete head;
			head = currNode;
		}
	}

	delete [] table;
}

void collection::destroyTree(treeNode *& root)
{
	if (root)
	{
		destroyTree(root->left);
		destroyTree(root->right);
		delete root->vendorData;
		delete root;
		root = NULL;
	}
}

void collection::displayName()
{
//	cout << "Vendor data by name: " << endl;

	displayNameHelper(root);
}

void collection::displayNameHelper(treeNode * treeIn)
{

	if (treeIn)
	{
		displayNameHelper(treeIn->left);
		cout << treeIn->vendorData << endl;
		displayNameHelper(treeIn->right);
	}
}

void collection::displayProduct(void) const
{
	hashNode * currNode = NULL;

//	cout << "Vendor data by product: " << endl;

	for(int i = 0; i < capacity; i++)
	{
		for(currNode = table[i]; currNode; currNode = currNode->next) // Seg faults
		{
			cout << currNode->vendorData << endl;
		}
	}
}

int collection::getTreeSize()
{
	return treeSize;
}

int collection::getTableSize()
{
	return tableSize;
}

void collection::initializeTable()
{
	table = new hashNode*[capacity];

	for (int i = 0; i < capacity; i++)
	{
		table[i] = NULL;
	}
}

void collection::insert(const data& dataIn)
{
	char tempChar[100] = {'\0'};

	data * newData = new data(dataIn);
	*newData = dataIn;

	addTree(newData);
	addTable(newData);
}

bool collection::remove(char * nameIn)
{
	bool delStatus = false;

	root = removeTree(root,nameIn,delStatus);

	if (delStatus)
	{
		return true;
	}
	else
		return false;
}

bool collection::removeTable(char * productIn)
{
	int index = calculateHash(productIn);

	hashNode * currNode = table[index];
	hashNode * prevNode = NULL;
	char product[100] = {'\0'};

	while(currNode)
	{
		currNode->vendorData->getProduct(product);

		if (strcmp(productIn, product) == 0)
		{
			if (!prevNode)
			{
				table[index] = currNode->next;
			}
			else
			{
				prevNode->next = currNode->next;
			}

			currNode->next = NULL;
			delete currNode->vendorData;
			delete currNode;

			tableSize--;

			return true;
		}
		else
		{
			prevNode = currNode;
			currNode = currNode->next;
		}
	}

	return false;
}

treeNode* collection::removeTree(treeNode*& treeIn, char * nameIn, bool& delStatusIn)
{
	char name[100] = {'\0'};
	char product[100] = {'\0'};

	if(!treeIn)
	{
		delStatusIn = false;
		return NULL;
	}
	else
	{
		treeIn->vendorData->getName(name);

		int temp = strcmp(name, nameIn);

		if(temp == 0)
		{
			treeIn->vendorData->getProduct(product);
			removeTable(product);

			if (treeIn == root)
			{
				root = deleteNode(treeIn);
			}
			else
				treeIn = deleteNode(treeIn);

			delStatusIn = true;

			return treeIn;
		}
		else if(temp < 0)
		{
			treeIn->right = removeTree(treeIn->right, nameIn, delStatusIn);

			return treeIn;
		}
		else
		{
			treeIn->left = removeTree(root->left, nameIn, delStatusIn);

			return treeIn;
		}
	}
}

int collection::retrieveProduct(data dataRtn[], char * productIn, int dataItemsRtn) const
{
	int dataItems = 0;

	int strCmp = 0;
	char compString[100] = {'0'};

	int index = calculateHash(productIn);
	int i = 0;

	hashNode * currNode = table[index];

	if (currNode)
	{
		while (currNode)
		{
			dataRtn[dataItems] = currNode->vendorData;
			dataItems++;
			currNode = currNode->next;
		}

		dataItemsRtn = dataItems;

		return dataItems;
	}
	else
		return 0;
}

bool collection::retrieveName(data& dataRtn, char * nameIn) const
{
	return retrieveNameHelper(root, dataRtn, nameIn);
}

bool collection::retrieveNameHelper(treeNode * treeIn, data& dataRtn, char * nameIn) const
{
	char name[100] = {'\0'};

	if(!treeIn)
	{
		return false;
	}
	else
	{
		treeIn->vendorData->getName(name);

		int temp = strcmp(name, nameIn);

		if(temp == 0)
		{
			dataRtn = treeIn->vendorData;
			return true;
		}
		else if(temp < 0)
		{
			return retrieveNameHelper(treeIn->right, dataRtn, nameIn);
		}
		else
		{
			return retrieveNameHelper(treeIn->left, dataRtn, nameIn);
		}
	}

}

void collection::writeOut (char * fileName)
{
	char events[100] = {'\0'};
	char name[100] = {'\0'};
	char number[100] = {'\0'};
	char products[100] = {'\0'};

	hashNode * currNode;

	ofstream outFile;
	outFile.open(fileName);

	for(int i = 0; i < capacity; i++)
	{
		for(currNode = table[i]; currNode; currNode = currNode->next)
		{
			currNode->vendorData->getName(name);
			currNode->vendorData->getNumber(number);
			currNode->vendorData->getProduct(products);
			currNode->vendorData->getEvents(events);

			outFile << name << ";";
			outFile << number << ";";
			outFile << products << ";";
			outFile << events << ";";
			outFile << '\n';
		}
	}

	outFile.close();
}

void collection::setRightChild(treeNode * treeIn, treeNode * childIn)
{
	treeIn->right = childIn;
}

void collection::setLeftChild(treeNode * treeIn, treeNode * childIn)
{
	treeIn->left = childIn;
}
