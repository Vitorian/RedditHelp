
# Build

```c++
sudo apt install libbenchmark-dev
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=release -DCMAKE_CXX_FLAGS_RELEASE_INIT='-march=native -mtune=native' ..
make -j
./bm_fnpointers
```
