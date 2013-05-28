#include <string>
#include <sstream>
#include <iostream>
#include <sbl_param_set.h>
#include <sbl_test.h>

using namespace std;
using namespace SBL;

struct Stream : public ParamSet {
    Param<int>      id;
    Param<int>      width;
    Param<int>      height;
    Param<string>   encoder;
    Stream() : ParamSet("stream"),
               id(this, "id", 0),
               width(this,  "width", 5, VerifyRange(0, 100)),
               height(this, "height", 7, VerifyRange(0, 100)),
               encoder(this, "encoder", "h264", VerifyEnum("h264", "mjpeg"))
               {}
};

struct Image : public ParamSet {
    Param<bool>     osd;
    Param<int>      hue;
    Image(ParamSet* parent, const char* name) :   ParamSet(parent, name),
                osd(this, "osd", false),
                hue(this, "hue", 0, VerifyRange(0,50))
                {}
};

struct Image2 : public ParamSet {
    Param<bool>     osd;
    Param<int>      hue;
    Param<int>      sensor_rate;
    Image2() :   ParamSet((ParamSet*) 0, ""),
                osd(this, "osd", false),
                hue(this, "hue", 0, VerifyRange(0,50)),
                sensor_rate(this, "sensor_rate", 30, VerifyEnum(25, 30))
                {}
};

struct Top : public ParamSet {
    static const int STREAM_COUNT = 2;
    Stream  stream[STREAM_COUNT];
    Image   image1;
    Image   image2;
    Top() : ParamSet("top"),
            image1(this, "image1"),
            image2(this, "image2")
    { stream[0].init("id=0&width=10&height=20&encoder=h264", this, 0);
      stream[1].init("id=1&width=20&height=30&encoder=mjpeg", this, 1);
    }
};

struct TestInt : public ParamSet {
    Param<int> frames;
    TestInt() : 
        frames(this, "frames", -1, VerifyEnum(-1, 0, 1))
        {}
};

void test_int() {
    TestInt test_int;
    SBL_TEST_EQ(test_int.frames, -1);
    test_int.frames = 0;
    SBL_TEST_EQ(test_int.frames, 0);
}


void init_stream() {
    Stream  stream;
    SBL_TEST_EQ(stream.width, 5);
    SBL_TEST_EQ(stream.height, 7);
    SBL_TEST_EQ(stream.encoder.get(), string("h264"));
}

void init_image() {
    Image image(0, "image");
    SBL_TEST_EQ(image.osd, false);
    SBL_TEST_EQ(image.hue, 0);
}

void init_top() {
    Top top;
    char name[100];
    SBL_TEST_EQ(top.stream[0].width, 10);
    SBL_TEST_EQ_STR(top.stream[0].name(), "stream[0]");
    SBL_TEST_EQ(top.stream[1].width, 20);
    SBL_TEST_EQ_STR(top.stream[1].name(), "stream[1]");
}

void revert_top() {
    Top top;
    top.image1.hue = 25;
    SBL_TEST_EQ(top.image1.hue, 25);
    top.stream[0].encoder = "mjpeg";
    SBL_TEST_EQ(top.stream[0].encoder.get(), string("mjpeg"));
    top.revert();
    SBL_TEST_EQ(top.image1.hue, 0);
    SBL_TEST_EQ_STR(top.stream[0].encoder.get().c_str(), "h264");
}

void chkpt_top() {
    Top top;
    top.image1.hue = 25;
    SBL_TEST_EQ(top.image1.hue, 25);
    top.stream[0].encoder = "mjpeg";
    SBL_TEST_EQ_STR(top.stream[0].encoder.get().c_str(), "mjpeg");
    top.checkpt();
    top.image1.hue = 35;
    SBL_TEST_EQ(top.image1.hue, 35);
    top.revert();
    SBL_TEST_EQ(top.image1.hue, 25);
    SBL_TEST_EQ_STR(top.stream[0].encoder.get().c_str(), "mjpeg");
}

void write_top() {
    Top top;
    top.set_eol("|");
    stringstream str;
    str << top;
    SBL_TEST_EQ_STR(str.str().c_str(), "image1=hue=0|osd=0||image2=hue=0|osd=0||stream[0]=encoder=h264|height=20|id=0|width=10||stream[1]=encoder=mjpeg|height=30|id=1|width=20||");
}

void test_ignore_bad_args() {
    Stream stream;
    ParamSet::ArgMap map;
    map["encoder"] = "invalid";
    map["width"] = "100";
    map["height"] = "50";
    map["bad_arg"] = "100";
    stream.set(map, false); // this should throw on bad arg
    SBL_TEST_EQ(stream.width, 100);
    SBL_TEST_EQ(stream.height, 50);
    SBL_TEST_EQ_STR(stream.encoder.get().c_str(), "h264");
}

void changed() {
    Top top;
    top.image1.osd = true;
    SBL_TEST_EQ(top.image1.osd.changed(), true);
    SBL_TEST_EQ(top.image1.changed(), true);
    SBL_TEST_EQ(top.changed(), true);
    top.checkpt();
    SBL_TEST_EQ(top.changed(), false);
    top.image1.osd = false;
    SBL_TEST_EQ(top.image1.osd.changed(), true);
    SBL_TEST_EQ(top.image1.changed(), true);
    SBL_TEST_EQ(top.changed(), true);
}

void verify_dots() {
    VerifyDots dots1;
    SBL_TEST_TRUE(dots1("192.168.1.40"));
    SBL_TEST_FALSE(dots1("192.168.1."));
    SBL_TEST_FALSE(dots1("192.168.1:"));
    SBL_TEST_FALSE(dots1("19a.168.1.40"));
    SBL_TEST_FALSE(dots1("192.168.1.40.20"));
    VerifyDots dots2(2, true);
    SBL_TEST_TRUE(dots2("0.0.1:1245"));
    SBL_TEST_FALSE(dots2("0.0.1:"));
    SBL_TEST_FALSE(dots2("0.0.1.5:1245"));
    SBL_TEST_FALSE(dots2("0.0.1::"));
}

void verify_copy_changed() {
    Stream stream1;
    Stream stream2;
    stream2.height = 20;
    stream1.width = 30;
    stream2.copy_changed(stream1);
    SBL_TEST_EQ(stream2.height, 20);
    SBL_TEST_EQ(stream2.width, 30);
    SBL_TEST_EQ(stream2.id, 0);
}

void verify_int_enum() {
    Image2 image;
    image.sensor_rate = 25;
    SBL_TEST_EQ(image.sensor_rate, 25);
    try {
        image.sensor_rate = 20;
        cout << "verify enum failed\n";
        exit(1);
    } catch (Exception& ex) {
        SBL_TEST_EQ(image.sensor_rate, 25);
    }
    image.sensor_rate = 30;
    SBL_TEST_EQ(image.sensor_rate, 30);
}

int main(int argc, char* argv[]) {
    Exception::enable_log(false);
    init_stream();
    init_image();
    init_top();
    revert_top();
    chkpt_top();
    write_top();
    changed();
    verify_dots();
    verify_copy_changed();
    verify_int_enum();
    test_ignore_bad_args();
    test_int();
    cout << argv[0] << " passed.\n";
    return 0;
}
