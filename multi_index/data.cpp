/*
// Private Data Members:
	char * vendorName;
	int * phoneNumber;
	char * productType;
	char * eventNotes;
*/
#include <cstring>
#include "data.h"

using namespace std;

data::data()
{
	vendorName = {'\0'};
	phoneNumber = {'\0'};
	productType = {'\0'};
	eventNotes = {'\0'};
}

data::data(const data& dataIn)
{
	setName(dataIn.vendorName);
	setNumber(dataIn.phoneNumber);
	setProduct(dataIn.productType);
	setEvents(dataIn.eventNotes);
}

const data& data::operator=(const data& dataIn)
{
	if (this != &dataIn)
	{
		setName(dataIn.vendorName);
		setNumber(dataIn.phoneNumber);
		setProduct(dataIn.productType);
		setEvents(dataIn.eventNotes);
	}

	return *this;
}

const data& data::operator=(const data * dataIn)
{
	if (this != dataIn)
	{
		setName(dataIn->vendorName);
		setNumber(dataIn->phoneNumber);
		setProduct(dataIn->productType);
		setEvents(dataIn->eventNotes);
	}

	return *this;
}

ostream& operator<<(ostream& out, const data * dataIn)
{
	if (dataIn)
	{
		out << dataIn->vendorName << '\t' << dataIn->phoneNumber << '\t' << dataIn->productType << '\t' << dataIn->eventNotes << '\t' << endl;
	}

	return out;
}

ostream& operator<<(ostream& out, const data& dataIn)
{
	out << dataIn.vendorName << '\t' << dataIn.phoneNumber << '\t' << dataIn.productType << '\t' << dataIn.eventNotes << '\t' << endl;

	return out;
}
data::~data()
{

	if (vendorName)
		delete [] vendorName;
	if (phoneNumber)
		delete [] phoneNumber;
	if (productType)
		delete [] productType;
	if (eventNotes)
		delete [] eventNotes;

//	clear();
}

void data::clear()
{

	if (vendorName)
		delete [] vendorName;
	if (phoneNumber)
		delete [] phoneNumber;
	if (productType)
		delete [] productType;
	if (eventNotes)
		delete [] eventNotes;
}

void data::getName(char * nameIn)const
{
	strcpy(nameIn, this->vendorName);
}

void data::getNumber(char * numIn)const
{
	strcpy(numIn, this->phoneNumber);
}

void data::getProduct(char * productIn)const
{
	strcpy(productIn, this->productType);
}

void data::getEvents(char * eventIn)const
{
	strcpy(eventIn, this->eventNotes);
}

void data::setName(char * nameIn)
{
	if (this->vendorName)
	{
//		delete [] this->vendorName;
	}

	this->vendorName = new char[strlen(nameIn)+1];
	strcpy(this->vendorName, nameIn);
}

void data::setNumber(char * numIn)
{
	if (this->phoneNumber)
	{
//		delete [] this->phoneNumber; // Seg fault
	}

	this->phoneNumber = new char[strlen(numIn)+1];
	strcpy(this->phoneNumber, numIn);
}

void data::setProduct(char * productIn)
{
	if(this->productType)
	{
//		delete [] this->productType; // Seg faults
	}

	this->productType = new char[strlen(productIn)+1];
	strcpy(this->productType, productIn);
}

void data::setEvents(char * eventIn)
{
	if(this->eventNotes)
	{
//		delete [] this->eventNotes;
	}

	this->eventNotes = new char[strlen(eventIn)+1];
	strcpy(this->eventNotes, eventIn);
}
