#ifndef DATA_H
#define DATA_H
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

class data
{
	char * vendorName;
	char * phoneNumber;
	char * productType;
	char * eventNotes;
public:
	data();
	data(const data& dataIn);
 	const data& operator= (const data * dataIn);
 	const data& operator= (const data& dataIn);
 	friend ostream& operator<< (ostream& out, const data * dataIn);
 	friend ostream& operator<< (ostream& out, const data& dataIn);
	~data();
	void clear();
	void getName(char * nameIn)const;
	void getNumber(char * numIn)const;
	void getProduct(char * typeIn)const;
	void getEvents(char * eventIn)const;
	void setName(char * nameIn);
	void setNumber(char * numIn);
	void setProduct(char * typeIn);
	void setEvents(char * eventIn);
};

#endif
