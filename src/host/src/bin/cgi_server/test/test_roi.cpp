#include <iostream>
#include <sbl/sbl_param_set.h>
#include "roi.h"

using namespace std;
using namespace SBL;
using namespace CGI;

int main() {
    Roi roi;
    roi.set("0.12,0.2,0.28,0.3");
    cout << roi;
    roi.add("0.33,0.87,0.21,0.6");
    cout << roi;
    roi.del("0.15,0.25,0.15,0.25");
    cout << roi;
    roi.clear();
    cout << roi;
}
