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

static int uhid_fd;
static struct uhid_event uhidEventXY;

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

    memset(&uhidEventXY, 0, sizeof(uhid_event));
    uhidEventXY.type = UHID_INPUT;
    uhidEventXY.u.input.size = 8;
    uhidEventXY.u.input.data[0] = 0x00;

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

extern "C"
JNIEXPORT jint JNICALL
Java_com_tile_tuoluoyi_GamePadNative_nativePianoKey(JNIEnv *env,
                                                    jclass thiz,
                                                    jint keyNumber,
                                                    jboolean isDown) {
//    pressed ? (uhidEventXY.u.input.data[2] |= 1 << 5) : (uhidEventXY.u.input.data[2] &= ~(1 << 5));
//    write(uhid_fd, &uhidEventXY, sizeof(uhid_event));
//    __android_log_print(ANDROID_LOG_INFO, "MyTag", "This is a log message from JNI%d",
//                        uhidEventXY.u.input.data[2]);

        uhidEventXY.u.input.data[0] = 0x00;
        uhidEventXY.u.input.data[1] = 0x00;
        uhidEventXY.u.input.data[2] = isDown ? 0x1D : 0x00;
        uhidEventXY.u.input.data[3] = 0x00;
        uhidEventXY.u.input.data[4] = 0x00;
        uhidEventXY.u.input.data[5] = 0x00;
        uhidEventXY.u.input.data[6] = 0x00;
        uhidEventXY.u.input.data[7] = 0x00;
        write(uhid_fd, &uhidEventXY, sizeof(uhidEventXY));

//    __android_log_print(ANDROID_LOG_INFO, "MyTag", "HELLO WORLD THIS IS FROM JNI");
        return 0;
}