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

static struct uinput_user_dev uinput_dev;
static int uinput_fd;
static struct input_event inputEventX, inputEventY, inputEventSYN, inputEventTL, inputEventTR, inputEventThumbL;

extern "C"
JNIEXPORT void JNICALL
Java_com_tile_tuoluoyi_GamePadNative_nativeUInputEvent(JNIEnv *env, jclass clazz, jint x_value,
                                                       jint y_value) {

    inputEventX.value = x_value;
    write(uinput_fd, &inputEventX, sizeof(struct input_event));

    inputEventY.value = y_value;
    write(uinput_fd, &inputEventY, sizeof(struct input_event));

    write(uinput_fd, &inputEventSYN, sizeof(struct input_event));
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_tile_tuoluoyi_GamePadNative_nativeCloseUInput(JNIEnv *env, jclass clazz) {
    ioctl(uinput_fd, UI_DEV_DESTROY);
    if (close(uinput_fd) < 0) return false;
    return true;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_tile_tuoluoyi_GamePadNative_nativeCreateUInput(JNIEnv *env, jclass clazz) {

    if ((uinput_fd = open("/dev/uinput", O_RDWR | O_NDELAY)) < 0) {
        return false;//error process.
    }

    //注册虚拟手柄设备
    memset(&uinput_dev, 0, sizeof(struct uinput_user_dev));
    strcpy(uinput_dev.name, "Xbox Wireless Controller");
    uinput_dev.id.version = 1;
    uinput_dev.id.bustype = BUS_VIRTUAL;
    uinput_dev.id.vendor = 0x1;
    uinput_dev.id.product = 0x1;
    //UInput机制中,轴事件的最小值是-32768，最大值是32767。但是我为了正负对称，就没有把最小值设为-32768。
    //设定好了最小值和最大值的作用是，后续我们发送轴事件时安卓系统会自动将轴事件归一化至-1到1之间。比如后续发一个-666，安卓系统就将这个数据归一化为-666/65534
    uinput_dev.absmin[ABS_X] = -1;
    uinput_dev.absmax[ABS_X] = 1;
    uinput_dev.absmin[ABS_Y] = -1;
    uinput_dev.absmax[ABS_Y] = 1;
    uinput_dev.absmin[ABS_Z] = -32767;
    uinput_dev.absmax[ABS_Z] = 32767;
    uinput_dev.absmin[ABS_RZ] = -32767;
    uinput_dev.absmax[ABS_RZ] = 32767;
    uinput_dev.absmin[ABS_THROTTLE] = -1;
    uinput_dev.absmax[ABS_THROTTLE] = 1;
    uinput_dev.absmin[ABS_BRAKE] = -1;
    uinput_dev.absmax[ABS_BRAKE] = 1;
    uinput_dev.absmin[ABS_HAT0X] = -1;
    uinput_dev.absmax[ABS_HAT0X] = 1;
    uinput_dev.absmin[ABS_HAT0Y] = -1;
    uinput_dev.absmax[ABS_HAT0Y] = 1;

    //ioctl函数用来声明设备所具有的一些按键和轴
    ioctl(uinput_fd, UI_SET_EVBIT, EV_SYN);
    ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(uinput_fd, UI_SET_EVBIT, EV_MSC);
    ioctl(uinput_fd, UI_SET_EVBIT, EV_ABS);

    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_THUMBL);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_THUMBR);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_TL);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_TR);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_X);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_A);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_Y);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_B);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_SELECT);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_START);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_DPAD_LEFT);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_DPAD_RIGHT);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_DPAD_UP);
    ioctl(uinput_fd, UI_SET_KEYBIT, BTN_DPAD_DOWN);
    ioctl(uinput_fd, UI_SET_MSCBIT, MSC_SCAN);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_HAT0X);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_HAT0Y);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_X);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_Y);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_Z);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_RZ);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_THROTTLE);
    ioctl(uinput_fd, UI_SET_ABSBIT, ABS_BRAKE);

    if (write(uinput_fd, &uinput_dev, sizeof(struct uinput_user_dev)) < 0) {
        return false;//error process.
    }

    if (ioctl(uinput_fd, UI_DEV_CREATE) < 0) {
        close(uinput_fd);
        return false;//error process.
    }
    memset(&inputEventX, 0, sizeof(struct input_event));
    inputEventX.type = EV_ABS;
    inputEventX.code = ABS_RZ;
    memset(&inputEventY, 0, sizeof(struct input_event));
    inputEventY.type = EV_ABS;
    inputEventY.code = ABS_Z;
    memset(&inputEventSYN, 0, sizeof(struct input_event));
    inputEventSYN.type = EV_SYN;
    inputEventSYN.code = SYN_REPORT;
    inputEventSYN.value = 0;
    memset(&inputEventTL, 0, sizeof(struct input_event));
    inputEventTL.type = EV_KEY;
    inputEventTL.code = BTN_TL;
    memset(&inputEventTR, 0, sizeof(struct input_event));
    inputEventTR.type = EV_KEY;
    inputEventTR.code = BTN_TR;
    memset(&inputEventThumbL, 0, sizeof(struct input_event));
    inputEventThumbL.type = EV_KEY;
    inputEventThumbL.code = BTN_THUMBL;

    return true;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_tile_tuoluoyi_GamePadNative_nativeUInputPressTL(JNIEnv *env, jclass clazz,
                                                         jboolean pressed) {
    inputEventTL.value = pressed ? 1 : 0;
    write(uinput_fd, &inputEventTL, sizeof(struct input_event));
    write(uinput_fd, &inputEventSYN, sizeof(struct input_event));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_tile_tuoluoyi_GamePadNative_nativeUInputPressTR(JNIEnv *env, jclass clazz,
                                                         jboolean pressed) {
    inputEventTR.value = pressed ? 1 : 0;
    write(uinput_fd, &inputEventTR, sizeof(struct input_event));
    write(uinput_fd, &inputEventSYN, sizeof(struct input_event));
}
extern "C"
JNIEXPORT void JNICALL
Java_com_tile_tuoluoyi_GamePadNative_nativeUInputPressThumbL(JNIEnv *env, jclass clazz,
                                                             jboolean pressed) {
    inputEventThumbL.value = pressed ? 1 : 0;
    write(uinput_fd, &inputEventThumbL, sizeof(struct input_event));
    write(uinput_fd, &inputEventSYN, sizeof(struct input_event));
}

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
JNIEXPORT void JNICALL
Java_com_tile_tuoluoyi_GamePadNative_nativeUHidEvent(JNIEnv *env, jclass clazz, jint x_value,
                                                     jint y_value) {
//    uhidEventXY.u.input.data[0] = 0x00;
//    uhidEventXY.u.input.data[1] = 0x00;
//    uhidEventXY.u.input.data[2] = 0x1D;
//    uhidEventXY.u.input.data[3] = 0x00;
//    uhidEventXY.u.input.data[4] = 0x00;
//    uhidEventXY.u.input.data[5] = 0x00;
//    uhidEventXY.u.input.data[6] = 0x00;
//    uhidEventXY.u.input.data[7] = 0x00;
//    write(uhid_fd, &uhidEventXY, sizeof(uhidEventXY));
}


extern "C"
JNIEXPORT void JNICALL
Java_com_tile_tuoluoyi_GamePadNative_nativeUHidPressTL(JNIEnv *env, jclass clazz,
                                                       jboolean pressed) {

    //pressed ? (uhidEventXY.u.input.data[1] |= 1 << 6) : (uhidEventXY.u.input.data[1] &= ~(1 << 6));
    uhidEventXY.u.input.data[0] = 0x00;
    uhidEventXY.u.input.data[1] = 0x00;
    uhidEventXY.u.input.data[2] = pressed ? 0x1D : 0x00;
    uhidEventXY.u.input.data[3] = 0x00;
    uhidEventXY.u.input.data[4] = 0x00;
    uhidEventXY.u.input.data[5] = 0x00;
    uhidEventXY.u.input.data[6] = 0x00;
    uhidEventXY.u.input.data[7] = 0x00;
    write(uhid_fd, &uhidEventXY, sizeof(uhidEventXY));

    __android_log_print(ANDROID_LOG_INFO, "MyTag", "HELLO WORLD THIS IS FROM JNI");
}
extern "C"
JNIEXPORT void JNICALL
Java_com_tile_tuoluoyi_GamePadNative_nativeUHidPressTR(JNIEnv *env, jclass clazz,
                                                       jboolean pressed) {
    uhidEventXY.u.input.data[0] = 0x00;
    uhidEventXY.u.input.data[1] = 0x00;
    uhidEventXY.u.input.data[2] = pressed ? 0x1D : 0x00;
    uhidEventXY.u.input.data[3] = 0x00;
    uhidEventXY.u.input.data[4] = 0x00;
    uhidEventXY.u.input.data[5] = 0x00;
    uhidEventXY.u.input.data[6] = 0x00;
    uhidEventXY.u.input.data[7] = 0x00;
    write(uhid_fd, &uhidEventXY, sizeof(uhidEventXY));
    __android_log_print(ANDROID_LOG_INFO, "MyTag", "HELLO WORLD THIS IS FROM JNI");
}
extern "C"
JNIEXPORT void JNICALL
Java_com_tile_tuoluoyi_GamePadNative_nativeUHidPressThumbL(JNIEnv *env, jclass clazz,
                                                           jboolean pressed) {
//    pressed ? (uhidEventXY.u.input.data[2] |= 1 << 5) : (uhidEventXY.u.input.data[2] &= ~(1 << 5));
//    write(uhid_fd, &uhidEventXY, sizeof(uhid_event));
//    __android_log_print(ANDROID_LOG_INFO, "MyTag", "This is a log message from JNI%d",
//                        uhidEventXY.u.input.data[2]);

    uhidEventXY.u.input.data[0] = 0x00;
    uhidEventXY.u.input.data[1] = 0x00;
    uhidEventXY.u.input.data[2] = pressed ? 0x1D : 0x00;
    uhidEventXY.u.input.data[3] = 0x00;
    uhidEventXY.u.input.data[4] = 0x00;
    uhidEventXY.u.input.data[5] = 0x00;
    uhidEventXY.u.input.data[6] = 0x00;
    uhidEventXY.u.input.data[7] = 0x00;
    write(uhid_fd, &uhidEventXY, sizeof(uhidEventXY));

//    __android_log_print(ANDROID_LOG_INFO, "MyTag", "HELLO WORLD THIS IS FROM JNI");
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