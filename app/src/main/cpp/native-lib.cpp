#include <jni.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <linux/uinput.h>
#include <cerrno>
#include <fcntl.h>
#include <poll.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <termios.h>
#include <unistd.h>
#include <linux/uhid.h>
#include <linux/input.h>
#include <jni.h>
#include <android/log.h>
#include <unordered_map>
#include <string>
#include <array>

static int uhid_fd;
static struct uhid_event uhidEvent;

// Keyboard description
static unsigned char description[] = {
        0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
        0x09, 0x06,        // Usage (Keyboard)
        0xA1, 0x01,        // Collection (Application)
        0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
        0x19, 0xE0,        //   Usage Minimum (0xE0)
        0x29, 0xE7,        //   Usage Maximum (0xE7)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x01,        //   Logical Maximum (1)
        0x75, 0x01,        //   Report Size (1)
        0x95, 0x08,        //   Report Count (8)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x01,        //   Report Count (1)
        0x75, 0x08,        //   Report Size (8)
        0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x05,        //   Report Count (5)
        0x75, 0x01,        //   Report Size (1)
        0x05, 0x08,        //   Usage Page (LEDs)
        0x19, 0x01,        //   Usage Minimum (Num Lock)
        0x29, 0x05,        //   Usage Maximum (Kana)
        0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x95, 0x01,        //   Report Count (1)
        0x75, 0x03,        //   Report Size (3)
        0x91, 0x03,        //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x95, 0x06,        //   Report Count (6)
        0x75, 0x08,        //   Report Size (8)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x65,        //   Logical Maximum (101)
        0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
        0x19, 0x00,        //   Usage Minimum (0x00)
        0x29, 0x65,        //   Usage Maximum (0x65)
        0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,              // End Collection

};

const int NUM_KEYS = 61;

std::array<std::string, NUM_KEYS> pianoQwertyKeys = {"1", "!", "2", "@", "3", "4", "$", "5", "%",
                                      "6", "^", "7", "8", "*", "9", "(", "0", "q", "Q", "w", "W",
                                      "e", "E", "r", "t", "T", "y", "Y", "u", "i", "I", "o", "O",
                                      "p", "P", "a", "s", "S", "d", "D", "f", "g", "G", "h", "H",
                                      "j", "J", "k", "l", "L", "z", "Z", "x", "c", "C", "v", "V",
                                      "b", "B", "n", "m"};

std::unordered_map<std::string, int> qwerty_to_hex_map = {
        {"a", 0x04},
        {"b", 0x05},
        {"c", 0x06},
        {"d", 0x07},
        {"e", 0x08},
        {"f", 0x09},
        {"g", 0x0A},
        {"h", 0x0B},
        {"i", 0x0C},
        {"j", 0x0D},
        {"k", 0x0E},
        {"l", 0x0F},
        {"m", 0x10},
        {"n", 0x11},
        {"o", 0x12},
        {"p", 0x13},
        {"q", 0x14},
        {"r", 0x15},
        {"s", 0x16},
        {"t", 0x17},
        {"u", 0x18},
        {"v", 0x19},
        {"w", 0x1A},
        {"x", 0x1B},
        {"y", 0x1C},
        {"z", 0x1D},
        {"0", 0x27},
        {"1", 0x1E},
        {"2", 0x1F},
        {"3", 0x20},
        {"4", 0x21},
        {"5", 0x22},
        {"6", 0x23},
        {"7", 0x24},
        {"8", 0x25},
        {"9", 0x26},

        // Capitals
        {"A", 0x04},
        {"B", 0x05},
        {"C", 0x06},
        {"D", 0x07},
        {"E", 0x08},
        {"F", 0x09},
        {"G", 0x0A},
        {"H", 0x0B},
        {"I", 0x0C},
        {"J", 0x0D},
        {"K", 0x0E},
        {"L", 0x0F},
        {"M", 0x10},
        {"N", 0x11},
        {"O", 0x12},
        {"P", 0x13},
        {"Q", 0x14},
        {"R", 0x15},
        {"S", 0x16},
        {"T", 0x17},
        {"U", 0x18},
        {"V", 0x19},
        {"W", 0x1A},
        {"X", 0x1B},
        {"Y", 0x1C},
        {"Z", 0x1D},
        {")", 0x27},
        {"!", 0x1E},
        {"@", 0x1F},
        {"#", 0x20},
        {"$", 0x21},
        {"%", 0x22},
        {"^", 0x23},
        {"&", 0x24},
        {"*", 0x25},
        {"(", 0x26},
};

std::unordered_map<std::string, int> meta_key_map = {
        {"a", 0x00},
        {"b", 0x00},
        {"c", 0x00},
        {"d", 0x00},
        {"e", 0x00},
        {"f", 0x00},
        {"g", 0x00},
        {"h", 0x00},
        {"i", 0x00},
        {"j", 0x00},
        {"k", 0x00},
        {"l", 0x00},
        {"m", 0x00},
        {"n", 0x00},
        {"o", 0x00},
        {"p", 0x00},
        {"q", 0x00},
        {"r", 0x00},
        {"s", 0x00},
        {"t", 0x00},
        {"u", 0x00},
        {"v", 0x00},
        {"w", 0x00},
        {"x", 0x00},
        {"y", 0x00},
        {"z", 0x00},
        {"0", 0x00},
        {"1", 0x00},
        {"2", 0x00},
        {"3", 0x00},
        {"4", 0x00},
        {"5", 0x00},
        {"6", 0x00},
        {"7", 0x00},
        {"8", 0x00},
        {"9", 0x00},

        // Capitals
        {"A", 0x02},
        {"B", 0x02},
        {"C", 0x02},
        {"D", 0x02},
        {"E", 0x02},
        {"F", 0x02},
        {"G", 0x02},
        {"H", 0x02},
        {"I", 0x02},
        {"J", 0x02},
        {"K", 0x02},
        {"L", 0x02},
        {"M", 0x02},
        {"N", 0x02},
        {"O", 0x02},
        {"P", 0x02},
        {"Q", 0x02},
        {"R", 0x02},
        {"S", 0x02},
        {"T", 0x02},
        {"U", 0x02},
        {"V", 0x02},
        {"W", 0x02},
        {"X", 0x02},
        {"Y", 0x02},
        {"Z", 0x02},
        {")", 0x02},
        {"!", 0x02},
        {"@", 0x02},
        {"#", 0x02},
        {"$", 0x02},
        {"%", 0x02},
        {"^", 0x02},
        {"&", 0x02},
        {"*", 0x02},
        {"(", 0x02}
};

std::unordered_map<std::string, int> numpad_to_hex_map = {
        {"*", 0x55},
        {"-", 0x56},
        {"+", 0x57},


        {"1", 0x59},
        {"2", 0x5A},
        {"3", 0x5B},
        {"4", 0x5C},
        {"5", 0x5D},
        {"6", 0x5E},
        {"7", 0x5F},
        {"8", 0x60},
        {"9", 0x61},
        {"0", 0x62},
};

// Function to write 8 integers to the file descriptor
void writeKeyboard(int metakey0, int reserved1, int data2) {

    // https://d1.amobbs.com/bbs_upload782111/files_47/ourdev_692986N5FAHU.pdf
    // https://www.usbzh.com/article/detail-326.html

    uhidEvent.u.input.data[0] = metakey0;
    uhidEvent.u.input.data[1] = reserved1; // Reserved

    // Keyboard keys in HEX
    uhidEvent.u.input.data[2] = data2;
    write(uhid_fd, &uhidEvent, sizeof(uhidEvent));

//    return 0;
}
void tapKeyboard(int metakey0, int reserved1, int data2) {
    writeKeyboard(metakey0, reserved1, data2);
    writeKeyboard(0x00, 0x00, 0x00);
}
//void writeKeyboard(int metakey0, int reserved1, int data2, int data3, int data4, int data5,
//                  int data6, int data7) {
//
//    // https://d1.amobbs.com/bbs_upload782111/files_47/ourdev_692986N5FAHU.pdf
//    // https://www.usbzh.com/article/detail-326.html
//
//    uhidEvent.u.input.data[0] = metakey0;
//    uhidEvent.u.input.data[1] = reserved1; // Reserved
//
//    // Keyboard keys in HEX
//    uhidEvent.u.input.data[2] = data2;
//    uhidEvent.u.input.data[3] = data3;
//    uhidEvent.u.input.data[4] = data4;
//    uhidEvent.u.input.data[5] = data5;
//    uhidEvent.u.input.data[6] = data6;
//    uhidEvent.u.input.data[7] = data7;
//    write(uhid_fd, &uhidEvent, sizeof(uhidEvent));
//
////    return 0;
//}

// Function to write 8 integers to the file descriptor
void SendEncodedKey(int a, int b, int c, int d) {
    tapKeyboard(0x00, 0x00, numpad_to_hex_map["*"]);


    tapKeyboard(0x00, 0x00, numpad_to_hex_map[std::to_string(a)]);

    tapKeyboard(0x00, 0x00, numpad_to_hex_map[std::to_string(b)]);

    tapKeyboard(0x00, 0x00, numpad_to_hex_map[std::to_string(c)]);

    tapKeyboard(0x00, 0x00, numpad_to_hex_map[std::to_string(d)]);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_tile_tuoluoyi_GamePadNative_nativeCreateUHid(JNIEnv *env, jclass clazz) {
    __android_log_print(ANDROID_LOG_WARN, "MyTag", "HELLO WORLD THIS IS FROM JNI");
    if ((uhid_fd = open("/dev/uhid", O_RDWR | O_NDELAY)) < 0) {
        return false;//error process.
    }
    struct uhid_event ev;
    memset(&ev, 0, sizeof(uhid_event));
    ev.type = UHID_CREATE;
    strcpy((char *) ev.u.create.name, "uHidKeyboard");
    ev.u.create.rd_data = description;
    ev.u.create.rd_size = sizeof(description);
    ev.u.create.bus = 0x03;
    ev.u.create.vendor = 0x1;
    ev.u.create.product = 0x1;
    ev.u.create.version = 0x1;
    ev.u.create.country = 0;
    if (write(uhid_fd, &ev, sizeof(ev)) != sizeof(uhid_event)) {
        return false;
    }

    memset(&uhidEvent, 0, sizeof(uhid_event));
    uhidEvent.type = UHID_INPUT;
    uhidEvent.u.input.size = 8;
    uhidEvent.u.input.data[0] = 0x00;

    return true;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_tile_tuoluoyi_GamePadNative_nativeCloseUHid(JNIEnv *env, jclass clazz) {

    struct uhid_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = UHID_DESTROY;
    return write(uhid_fd, &ev, sizeof(uhid_event)) > 0;
}


// qwerty mode function [works on most roblox piano games]
// RobloxMidiConnect(Piano Rooms) mode function [made specifically for Piano Rooms' MidiConnect functionality]

extern "C"
JNIEXPORT void JNICALL
Java_com_tile_tuoluoyi_GamePadNative_nativeQwertyKey(JNIEnv *env,
                                                    jclass thiz,
                                                    jint noteNumber,
                                                    jboolean isDown) {
        if (!isDown) {
//            return 0; // We won't bother with keys being held down(in qwerty mode) as most roblox piano games don't support it anyway
            return;

            // If you however wanted to support it(feel free to send a pr), we'd need to essentially do
            // what gaming keyboards do and that is create multiple instances of a virtual keyboard
            // to effectively increase the number of keys that we could press simultaneously
        }
        //# C2 is noteNumber: 36 is 0
        //# C7 is noteNumber: 96 is m
        if (!(noteNumber >= 36 && noteNumber <= 96)) {
            __android_log_print(ANDROID_LOG_ERROR, "MyTag", "noteNumber outside valid range!");
//            return 500;
        }


        // Converts the noteNumber to the corresponding qwerty key [36-36 = index 0 = "1", 96-36 = index 60 = "m"]
        std::string qwerty_key = pianoQwertyKeys[noteNumber - 36];

        // Converts the qwerty key into a hex value ["1" = 0x31, "m" = 0x10]
        int key_hex_value = qwerty_to_hex_map[qwerty_key];

        // Checks whether if the associated qwerty key has to be held with a meta key(in our case left shift to indicate it's capital)
        int meta_key_value = meta_key_map[qwerty_key];

        // quick tap
        tapKeyboard(meta_key_value,0x00,key_hex_value);

        // For Optimization's sake, we log directly instead of returning(which I think will improve performance)

        __android_log_print(ANDROID_LOG_DEBUG, "MyTag", "%s", ("Acknowledged : " + std::to_string(noteNumber) + " " + std::string(isDown ? "true" : "false")).c_str());
//        return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_tile_tuoluoyi_GamePadNative_nativePianoRoomsKey(JNIEnv *env,
                                                     jclass thiz,
                                                     jboolean isDown,
                                                     jint noteNumber,
                                                     jint velocity) {

//    # We are dividing by 12 because we will be encoding it with 12 keys only(The keys are "0123456789-+")
//    # Additionally, it adds up nicely because there are 12 semitones in one octave
//
//    # C0 aka Note C at Octave 0
//    # Note number: 12
//    # O = msg.note / 12 = 12 / 12 = 1
//    # Octave = 1 - O = 1 - 1 = 0
//
//    # C4 aka Note C at Octave 4
//    # Note number: 60
//    # O = msg.note / 12 = 60 /12 = 5
//    # Octave = 1 - O = 5 - 1 = 4
//
//    # To decode
//    # (DIV_VAL * 12) + MODULOS_VAL

    // NOTE: Because we explicitly declared the arguments as an int, C++ automatically discards
    // the decimal bit for us allowing us to skip having to floor(this wouldn't be possible if the values given were negative)

    int EncodedOctaveNo = noteNumber / 12;
    int EncodedNoteNo = noteNumber % 12;

    // Velocity defaults to 0 which tells the game that the notes are no longer being held
    int EncodedVelocityA = 0;
    int EncodedVelocityB = 0;

    if (isDown == true) {
        EncodedVelocityA = velocity / 12;
        EncodedVelocityB = velocity % 12;
    }

    SendEncodedKey(EncodedOctaveNo, EncodedNoteNo, EncodedVelocityA, EncodedVelocityB);

    // For Optimization's sake, we log directly instead of returning(which I think will improve performance)

    __android_log_print(ANDROID_LOG_DEBUG, "MyTag", "%s", ("Acknowledged PR : " + std::to_string(noteNumber) + " " + std::string(isDown ? "true" : "false")).c_str());
//        return 0;
}