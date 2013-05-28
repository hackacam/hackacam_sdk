#include <cassert>
#include <map>
#include <iostream>
#include "sbl_map.h"

using namespace std;
using namespace SBL;

map<const char*, int, StrCompare> my_map = CreateMap<const char*, int, StrCompare>
    ("Zero",       0)
    ("One",        1)
    ("Two",        2)
    ("Three",      3)
;

int main(int argc, char* argv[]) {
    assert(my_map["Zero"]   == 0);
    assert(my_map["One"]    == 1);
    assert(my_map["Two"]    == 2);
    assert(my_map["Three"]  == 3);
    cout << argv[0] << " passed.\n";
    return 0;
}
