
#include <iostream>

template<typename T>
std::ostream& operator <<(std::ostream &os, const std::vector<T> &v) {
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(os, "\n"));
    return os;
}

template<typename T>
std::ostream& operator >> (std::istream &os, const std::vector<T> &v) {

   std::copy(v.begin(), v.end(), ostream_iterator<T>(os, "\n"));
   return os;
}

void write_file( const std::string& filename,
                 const std::vector<int>& vec ) {
    std::fstream fout( filename );
    fout << vec << "\n";
}

void read_file( const std::string& filename,
                std::vector<int>& vec ) {
    std::ifstream fin( filename );
    fin >> vec;
}

int main( int argc, char* argv[] ) {
    std::vector<int> vec{ 0, 1, 2, 3, 4 };
    write_file( "f1.txt", vec );
    read_file( "f1.txt", vec );

}
