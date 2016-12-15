#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include <windows.h>

using namespace std;

bool Interpolation_Search(int numbers[], int amount, int value, int &indexr) {
bool found = false;
int low = 0, high = amount-1;
bool run_once = false;
while (numbers[high] >= value && numbers[low] < value) {
    double rise = high - low;
    double run = numbers[high] - numbers[low];
    double x = value - numbers[low];
    int index = (((rise/run) * x) + low);
    if (value < numbers[index]) {
        high = index - 1;
    } else if (value > numbers[index]) {
        low = index + 1;
    } else {
        low = index;
    }
}
if (numbers[low] == value)
    indexr = low;
    found = true;

return found;
}

int arr[100];


int amount;
int value;
int index;

int main( )
{
cout << "Interpolation search Algorithm" << endl << endl;
cout << "How many numbers? ";
cin >> amount;
cout << "Number to seach for? ";
cin >> value;
cout << "Enter numbers:" << endl;
for (int i = 0; i < amount; i++) {
    cin >> arr[i];
}
system("cls");
Insertion_Sort(arr, amount);
cout << "Interpolation search result" << endl << endl;
if (Interpolation_Search(arr, amount, value, index)) {
    cout << value << " found in array at index " << (index + 1) << endl;
} else {
    cout << value << " not found in array" << endl;
}
return 0;
}
