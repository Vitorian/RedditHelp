#include <string.h>
#include <iostream>
#include <limits> // for NaN()

// Compile with
//  g++ ArrayReversalAndAverage.cpp -o ArrayReversalAndAverage

using namespace std;

float avg( int x[], int size )
{
    if ( size>0 ) {
        float total = 0;
        for (int i = 0; i < size; i++){
            total += x[i];
        }
        return total/size;
    }
    return std::numeric_limits<float>::quiet_NaN();
}



int main () {

    const unsigned N = 100;
    int list[N];

    memset( list, 0, sizeof(list) );

    int a = 0;
    int entries = 0;

    while ( (a!=-1) and (entries<N) ){
        cout << "Please enter an integer: ";
        cin >> a;
        if (a != -1){
            list[entries]=a;
            entries++;
        }
    }

    // Reverse in place
    for ( unsigned j=0, i=entries-1; i>j; ++j, --i ) {
        int tmp = list[j];
        list[j] = list[i];
        list[i] = tmp;
    }

    cout << "The reversed list is ";
    for (int i = 0; i <= entries-1; i++) {
        cout << list[i] << " ";
    }

    cout << "The average of the numbers is "<< avg(list,entries) << "." << endl;
    return 0;
}
