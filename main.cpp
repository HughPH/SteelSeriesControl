#include <iostream>
#include <libusb-1.0/libusb.h>
#include <map>
#include <vector>
#include <dirent.h>
#include <fstream>
#include <iterator>
#include <zconf.h>
#include <algorithm>
#include <thread>
#include <cstring>

#define Byte unsigned char
// will need this for ramps
#define SByte char
#define UInt16 uint16_t

using namespace std;

void InvokeHapticFeedback(char **argv);
void ConfigureLED(char **args);
void ConfigureOLD(char **args);
void delay (unsigned int msecs);

libusb_device_handle* SetupDevice(libusb_context *ctx);

void PlayAnimation();

void ShutdownDevice(libusb_context *ctx);

struct Colour {
    Byte R;
    Byte G;
    Byte B;

    Colour(Byte R, Byte B, Byte G) {
        this->R = R;
        this->G = G;
        this->B = B;
    }
};

Colour ParseColour(char *arg);

libusb_device_handle* deviceHandle;

map<string, Byte> buzzes = {
        {"Strong"          , 0b000001},
        {"Soft"            , 0b000010},
        {"Sharp"           , 0b000100},
        {"Ping"            , 0b001000},
        {"Bump"            , 0b000111},
        {"Double"          , 0b001010},
        {"QuickDouble"     , 0b011011},
        {"QuickDoubleSoft" , 0b100000},
        {"QuickTriple"     , 0b001100},
        {"Buzz"            , 0b101111},
        {"LongBuzz"        , 0b001111},
        {"Ring"            , 0b010000},
        {"LongButLight"    , 0b111111},
        {"LightBuzz"       , 0b110011},
        {"Tick"            , 0b011000},
        {"Pulse"           , 0b110101},
        {"StrongPulse"     , 0b110100},
};

map<string, Byte> leds = {
        { "Logo", 0 },
        { "Wheel", 1 },
};

map<string, Byte> ledModes // numbers directly match the "mode" passed in the message content
        {
                { "Trigger", 8 },
                { "Steady", 1 },
                { "Shift", 0 }
        };

map<string, Colour> colours = {
        {"White"     , Colour(0xff,0xff,0xff)},
        {"Red"       , Colour(0xff,0x00,0x00)},
        {"Green"     , Colour(0x00,0xff,0x00)},
        {"LightGreen", Colour(0x80,0xff,0x80)},
        {"Lime"      , Colour(0x00,0xff,0x00)},
        {"Blue"      , Colour(0x00,0x00,0xff)},
        {"LightBlue" , Colour(0x80,0x80,0xff)},
        {"Yellow"    , Colour(0xff,0xff,0x00)},
        {"Aqua"      , Colour(0x00,0xff,0xff)},
        {"Teal"      , Colour(0x00,0xff,0xff)},
        {"Turquoise" , Colour(0x00,0xff,0xff)},
        {"Fuchsia"   , Colour(0xff,0x00,0xff)},
        {"Pink"      , Colour(0xff,0x00,0xff)},
        {"Purple"    , Colour(0x80,0x00,0x80)},
};

int main(int argc, char *argv[]) {

    libusb_context *ctx;
    deviceHandle = SetupDevice(ctx);

    if (argc < 2){
        //print_usage();
        return 0;
    }

    string command = string(argv[1]);

    if (command == "Tactile" ||
        command == "Haptic" ||
        command == "Buzz" ) {
        InvokeHapticFeedback(argv);
    }

    if (command == "Light" ||
        command == "Lamp" ||
        command == "Colour" ||
        command == "Color" ||
        command == "LED"){
        ConfigureLED(argv);
    }

    if (command == "Image" ||
        command == "Picture" ||
        command == "OLED"){
        ConfigureOLD(argv);
    }

    if (command == "Animation"){
        PlayAnimation();
    }

    ShutdownDevice(ctx);
    return 0;
}

void ShutdownDevice(libusb_context *ctx) {
    if(deviceHandle) {
        int attachResult = libusb_attach_kernel_driver(deviceHandle, 0); // if there is one I guess?
        if (attachResult!=0)
            cout<<"Could not attach kernel driver on interface 0" << endl;
        libusb_close(deviceHandle);
    }
    libusb_exit(ctx);
}

void PlayAnimation() {
    vector<string> files{};
    DIR *dir;
    struct dirent *entry;
    if ((dir = opendir(".")) != nullptr) {
        while ((entry = readdir(dir)) != nullptr) {
            string filename = string(entry->d_name);
            if (filename.length() > 5 && filename.substr(filename.length() - 5) == ".bits")
                files.insert(files.end(), filename);
        }
        closedir(dir);
    }
    sort(files.begin(), files.end());

    Byte command = 0x50;

    for (const auto &i : files) {
        ifstream file;
        cout<<"Uploading " + i<<endl;
        delay(70);
        file.open(i, ios::binary);
        file >> std::noskipws;
        vector<Byte> data  {command,0x00};
        data.insert(data.end(),istream_iterator<Byte>(file),istream_iterator<Byte>());
        file.close();
        libusb_control_transfer(deviceHandle, 0x21 , 9, 0x0300, 0, data.data(), static_cast<uint16_t>(data.size()), 60);
    }
}

void delay (unsigned int msecs) {
    clock_t clock1 = clock();
    clock_t goal = msecs * CLOCKS_PER_SEC / 1000 + clock1;  //convert msecs to clock count

    while ( goal > clock() ){
    };
}

void ConfigureOLD(char **args) {
    string i = string(args[2]);
    ifstream file;
    cout<<"Uploading " + i<<endl;
    delay(70);
    file.open(i, ios::binary);
    file >> std::noskipws;
    vector<Byte> data {0x51, 0x00};
    data.insert(data.end(),istream_iterator<Byte>(file),istream_iterator<Byte>());
    file.close();

    uint16_t wValue = 0x0300;
    libusb_control_transfer(deviceHandle, 0x21 , 9, wValue, 0, data.data(), data.size(), 60);
}

void ConfigureLED(char **args) {

    // args[2] = LED
    Byte LED = leds[args[2]];

    // args[3] = Mode
    Byte Mode = ledModes[args[3]];

    Colour Colour1 = ParseColour(args[4]);
    Colour Colour2 = Colour(0,0,0);
    UInt16 Time = 0;

    // if Mode == Trigger
    if (Mode == 8) {
        //   args[4] = Colour 1 (done)
        //   args[5] = Colour 2
        Colour2 = ParseColour(args[5]);
        //   args[6] = Duration in ms. (We'll round to the nearest 10)
        Time = static_cast<UInt16>(strtol(args[6], nullptr, 10) / 10);
    }

    if (Mode == 1) {
        // if Mode == Steady
        //   args[4] = Colour - which is already done
    }

    // not even going to try Shift yet


    // I'd like to be able to momentarily change the colour, then change back to what it was.
    // To support:
    //      Fading to and from a different colour - or not
    //      Flashing two colours, e.g. red/blue, with fading into and/or out of the flash and/or the transition between colours
    // need to see if we can query current colour
    // need to store local data

    Byte timeLowByte = static_cast<Byte>(Time & 0xff);
    Byte timeHighByte = static_cast<Byte>((Time >> 8) & 0xff);
    vector<Byte> data = {05, 00, LED, Colour1.R, Colour1.G, Colour1.B, Colour2.R, Colour2.G, Colour2.B, timeLowByte, timeHighByte};

    uint16_t wValue = 0x0200;
    libusb_control_transfer(deviceHandle, 0x21 , 9, wValue, 0, data.data(), static_cast<UInt16>(data.size()), 60);
}

Colour ParseColour(char *arg) {
    Colour result = colours[arg];
    if (result.R == 0 && result.G == 0 && result.B == 0){
        int adj = 0;
        if (arg[0] == '#')
            adj = 1;
        if (strlen(arg) - adj == 6){
            char buf[2];
            memcpy(buf, arg, 0 + adj);
            result.R = static_cast<Byte>(std::strtol(buf, nullptr, 16));
            memcpy(buf, arg, 2 + adj);
            result.G = static_cast<Byte>(std::strtol(buf, nullptr, 16));
            memcpy(buf, arg, 4 + adj);
            result.B = static_cast<Byte>(std::strtol(buf, nullptr, 16));

        } else if(strlen(arg) - adj == 3){
            char buf[1];
            memcpy(buf, arg, 0 + adj);
            result.R = static_cast<Byte>(std::strtol(buf, nullptr, 16) * 0x11);
            memcpy(buf, arg, 1 + adj);
            result.G = static_cast<Byte>(std::strtol(buf, nullptr, 16) * 0x11);
            memcpy(buf, arg, 2 + adj);
            result.B = static_cast<Byte>(std::strtol(buf, nullptr, 16) * 0x11);
        }
    }

    return result;
}

void InvokeHapticFeedback(char **argv) {
    string buzzName = string(argv[2]);
    Byte buzzType;

    if (buzzes[buzzName])
        buzzType = buzzes[buzzName];
    else
        buzzType = static_cast<Byte>(stoi(buzzName));

    int wValue = 0x0200;

    std::vector<Byte> data = {0x59,0x01, 0x00, buzzType};

    libusb_control_transfer(deviceHandle, 0x21 , 9, wValue, 0, data.data(), data.size(), 60);
}

libusb_device_handle* SetupDevice(libusb_context *ctx) {
    
    ctx = nullptr;

    int r = libusb_init(&ctx);
    libusb_device_handle *deviceHandle;

    if (r != 0) {
        cout << "Init error " << r << endl;
        return deviceHandle;
    }
    libusb_set_debug(ctx, 3);

    int detachResult;
    int claimResult;

    deviceHandle = libusb_open_device_with_vid_pid(ctx, 0x1038, 0x1700);
    if (!deviceHandle) {
        cout<<"Could not open device" << endl;
        return deviceHandle;
    }

    detachResult = libusb_detach_kernel_driver(deviceHandle, 0);
    if (detachResult!=0)
        cout<<"Could not detach kernel driver on interface 0" << endl;

    claimResult = libusb_claim_interface(deviceHandle, 0);
    if (claimResult!=0)
        cout<<"Could not claim interface 0" << endl;

    return deviceHandle;
}



