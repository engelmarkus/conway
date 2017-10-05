# conway
C++ compile time version of Conway's Game of Life.

## Usage
Pass the width and height of the field, the initial configuration of the field, and the number of iterations you want to calculate to the compiler:
```
> g++ -std=c++1z -D'WIDTH=4' -D'HEIGHT=3' \
    -D'FIELD="{1000}{1000}{1000}"' -D'STEPS=1' \
    -o conway conway.cpp
```
Now don't run the resulting program (you may and it will print the correct field anyway) but instead look into the executable and you will find the precalculated result there:
```
> strings conway | grep "{"
{0000}
{1100}
{0000}
```
