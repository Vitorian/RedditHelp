#pragma once

#include <iomanip>
#include <iostream>
#include "vstring.h"

struct data
{
    vstring vendorName;
    vstring phoneNumber;
    vstring productType;
    vstring eventNotes;

    friend inline ostream& operator<< (ostream& out, const data& dataIn )
    {
        out << dataIn.vendorName  << '\t'
            << dataIn.phoneNumber << '\t'
            << dataIn.productType << '\t'
            << dataIn.eventNotes  << '\t' << endl;
        return out;
    }

    void clear()
    {
        vendorName.clear();
        phoneNumber.clear();
        productType.clear();
        eventNotest.clear();
    }

};
