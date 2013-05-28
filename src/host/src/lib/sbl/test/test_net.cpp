#include <string>
#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sbl_net.h>
#include <sbl_test.h>

using namespace std;
using namespace SBL;

string ifconfig(const char* what) {
    FILE* fp = popen("/sbin/ifconfig", "r");
    char line[200];
    while (fgets(line, sizeof line, fp)) {
        char* p = strstr(line, what);
        if (p) {
            p += strlen(what);
            p[strcspn(p, " \n")] = '\0';
            fclose(fp);
            return p;
        }
    } 
    fclose (fp);
    return "";
}

void test_mac() {
    FILE *fp = fopen("/sys/class/net/eth0/address", "r");
    char sys[100];
    fgets(sys, sizeof sys, fp);
    sys[strlen(sys) - 1] = '\0';
    char our[Net::MAC_ADDR_BUFF_SIZE];
    Net::mac_address(our);
    SBL_TEST_EQ_STR(our, sys);
}

void test_ip() {
    char addr[Net::IP_ADDR_BUFF_SIZE];
    Net::ip_address(addr);
    string golden(ifconfig("inet addr:"));
    SBL_TEST_EQ_STR(addr, golden.c_str());
}

string gateway() {
    FILE* fp = popen("netstat -r -n | awk '{print $1, $2}'", "r");
    char line[200];
    const char deflt[] = "0.0.0.0 ";
    while (fgets(line, sizeof line, fp)) {
        if (strncmp(line, deflt, strlen(deflt)) == 0) {
            fclose(fp);
            char* nl = strchr(line, '\n');
            if (nl)
                *nl = '\0';
            return line + strlen(deflt);
        }
    }
    fclose(fp);
    return "";
}

void test_gateway() {
    char gtw[Net::IP_ADDR_BUFF_SIZE];
    Net::gateway_address(gtw);
    string golden(gateway());
    SBL_TEST_EQ_STR(gtw, golden.c_str());
}

void test_subnet() {
    char subn[Net::IP_ADDR_BUFF_SIZE];
    Net::subnet(subn);
    string golden(ifconfig("Mask:"));
    SBL_TEST_EQ_STR(subn, golden.c_str());
}

int main(int argc, char* argv[]) {
    test_mac();
    test_ip();
    test_gateway();
    test_subnet();
    cout << argv[0] << " passed.\n";
    return 0;
}
