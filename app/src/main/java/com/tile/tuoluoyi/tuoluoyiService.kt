package com.tile.tuoluoyi

import android.accessibilityservice.AccessibilityService
import android.annotation.SuppressLint
import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.SharedPreferences
import android.content.SharedPreferences.OnSharedPreferenceChangeListener
import android.content.res.Configuration
import android.graphics.Color
import android.graphics.drawable.Icon
import android.hardware.SensorManager
import android.media.midi.MidiDeviceInfo
import android.media.midi.MidiManager
import android.media.midi.MidiOutputPort
import android.media.midi.MidiReceiver
import android.os.Build
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.os.RemoteException
import android.util.DisplayMetrics
import android.util.Log
import android.util.TypedValue
import android.view.KeyEvent
import android.view.MotionEvent
import android.view.View
import android.view.View.OnTouchListener
import android.view.WindowManager
import android.view.accessibility.AccessibilityEvent
import android.widget.ImageView
import android.widget.Toast
import java.io.IOException
import java.util.concurrent.ConcurrentHashMap


class ConsoleList(private val Context: Context) {
    private val messages: MutableList<String>
    private val currentContext: Context

    init {
        messages = ArrayList()
        currentContext=Context
    }

    fun add(message: String) {
//        messages.add(message)
//        Display the message as a Toast
        Handler(Looper.getMainLooper()).post {
            Toast.makeText(
                currentContext,
                message,
                Toast.LENGTH_SHORT
            ).show()
        }
        Log.d("MyTag", message)
    }
}

class tuoluoyiService : AccessibilityService() {
    var iGamePad: IGamePad? = null
    var binder: IBinder? = null
    var isBroadcastRegistered = false
    var isGamePadCreated = false
    var isGyroEnabled = false
    var invertX = false
    var invertY = false
    var isFloatWindowExist = false
    var canFloatWindowMove = true
    var isSharedPreferenceRegistered = false
    var isThumbLPressed = false
    var sp: SharedPreferences? = null
    var sensityX = 0
    var sensityY = 0
    var windowManager: WindowManager? = null
    var params: WindowManager.LayoutParams? = null
    var floatWindowSize = 0
    var SCREEN_WIDTH = 0
    var SCREEN_HEIGHT = 0
    var view: ImageView? = null
    var mSensorMgr: SensorManager? = null // 声明一个传感管理器对象

    // MIDI RELATED OPTIONS
    var MIDIOutputPort: MidiOutputPort? = null
//    var binding: ActivityMainBinding? = null
    val consoleList = ConsoleList(this@tuoluoyiService)

    val mBroadcastReceiver: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            when (intent.action) {
                "intent.tuoluoyi.exit" -> disableSelf()
                "android.intent.action.CONFIGURATION_CHANGED" -> if (isFloatWindowExist) {
                    GetWidthHeight()
                    val rotation = windowManager!!.defaultDisplay.rotation
                    view!!.visibility = View.VISIBLE
                    view!!.setImageResource(R.drawable.icon)
                    windowManager!!.updateViewLayout(view, params)
                }

                "intent.tuoluoyi.sendBinder" -> {
                    val binderContainer = intent.getParcelableExtra<BinderContainer>("binder")
                    val binder = binderContainer!!.binder

                    //如果binder已经失去活性了，则不再继续解析
                    if (!binder.pingBinder()) return
                    this@tuoluoyiService.binder = binder
                    //将binder转换为接口
                    iGamePad = IGamePad.Stub.asInterface(binder)


                    try {
                        iGamePad?.changeMode(sp!!.getInt("currentMode", 0))
                        Log.d("MyTag", "ATTEMPTING TO CREATE GAMEPAD IN HERE! TUOSERVICE")
                        isGamePadCreated = iGamePad?.create() == true // Checks whether if the gamepad was created or not
                    } catch (e: RemoteException) {
                        e.printStackTrace()
                    }


                    if (isGamePadCreated) {
                        Toast.makeText(context, R.string.connect_success, Toast.LENGTH_SHORT).show()

                        Log.d("MyTag", "Attempting to access MIDI")
                        // MIDI HANDLING LOGIC
                        consoleList.add("Attempting to initialize MIDI service")
                        val midiManager = getSystemService(MIDI_SERVICE) as MidiManager
                        val devices: Array<MidiDeviceInfo> = midiManager.devices
                        var deviceInfo: MidiDeviceInfo? = null

                        for (device in devices) {
                            if (device.outputPortCount > 0) {
                                deviceInfo = device
                                break
                            }
                        }

                        if (deviceInfo == null) {
                            consoleList.add("FAILED TO FIND MIDI DEVICE!")
                            sendBroadcast(Intent("intent.tuoluoyi.exit"))
                            return
                        }


//                        refreshUI()

                        val TouchIDPool = object  {
                            private val unusedPool = ArrayList<Int>()
                            private var nextId = 0

                            init {
                                // Initialize the pool with some initial touch IDs
                                for (i in 0..9) {
                                    unusedPool.add(nextId)
                                    nextId++
                                }
                            }

                            fun getTouchID(): Int? {
                                if (unusedPool.isEmpty()) {
                                    // Pool is exhausted, consider expanding or handling overflow
                                    return null
                                }
                                return unusedPool.removeFirst()
                            }

                            fun releaseTouchID(touchID: Int) {
                                unusedPool.add(touchID)
                            }
                        }
                        var activeTouches = ConcurrentHashMap<Int, Int>()
                        val robloxKeys = arrayOf<String>("1", "!", "2", "@", "3", "4", "$", "5", "%", "6", "^", "7", "8", "*", "9", "(", "0", "q", "Q", "w", "W", "e", "E", "r", "t", "T", "y", "Y", "u", "i", "I", "o", "O", "p", "P", "a", "s", "S", "d", "D", "f", "g", "G", "h", "H", "j", "J", "k", "l", "L", "z", "Z", "x", "c", "C", "v", "V", "b", "B", "n", "m")

                        midiManager.openDevice(
                            deviceInfo,
                            { device ->
                                if (device == null) {
                                    consoleList.add("Failed to open device " + deviceInfo);
                                    disableSelf()
                                } else {
                                    consoleList.add("Connected to $device")

                                    class MyReceiver : MidiReceiver() {
                                        private val NOTE_ON = 0x90
                                        private val NOTE_OFF = 0x80
                                        private val ALIVE: Byte = 0xFE.toByte()

                                        private fun logByteArray(prefix: String, data: ByteArray, offset: Int, count: Int) {
                                            val builder = StringBuilder(prefix)
                                            for (i in 0 until count) {
                                                builder.append(String.format("0x%02X", data[offset + i]))
                                                if (i != count - 1) {
                                                    builder.append(", ")
                                                }
                                            }
                                            consoleList.add(builder.toString())
                                        }

                                        @Throws(IOException::class)
                                        override fun onSend(
                                            data: ByteArray, offset: Int,
                                            count: Int, timestamp: Long
                                        ) {
                                            // Ignore the alive signal
                                            if (data[offset] == ALIVE) {
                                                return
                                            }

                                            for (i in offset until offset + count) {
                                                val byte = data[i].toInt() and 0xFF
                                                if (byte >= 0x80) { // Status byte
                                                    val messageType = byte and 0xF0
                                                    val channel = byte and 0x0F + 1
                                                    val noteNumber = data[i + 1].toInt()

                                                    var isDown: Boolean = false
                                                    if (messageType == NOTE_ON) {
                                                        isDown = true
                                                    } else if (messageType == NOTE_OFF) {
                                                        isDown = false
                                                    } else {
                                                        continue
                                                    }

                                                    try {
                                                        if (noteNumber >= 0 && noteNumber <= 108) {
                                                            iGamePad?.pianoKey(noteNumber, isDown)
                                                                ?.let { consoleList.add(it) }
//                                                          consoleList.add("Key pressed: IsDown $isDown, Note $noteNumber")
                                                        }
                                                    } catch (exception: Exception) {
                                                        val errMsg = "ERROR ISDOWN $isDown: $exception"

                                                        consoleList.add(errMsg)
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    MIDIOutputPort?.close()
                                    MIDIOutputPort = device.openOutputPort(0)
                                    MIDIOutputPort?.connect(MyReceiver())

                                }
                            },
                            Handler(Looper.getMainLooper())
                        )
                        // END OF MIDI HANDLING LOGIC
                        isGyroEnabled = true

                        //如果用户开启了”悬浮球“，则展示一个悬浮球。
                        if (sp!!.getBoolean("floatWindow", true)) {
                            showFloatWindow()
                        }
                    } else {
                        Toast.makeText(context, "'iGamePad?.create() == true' check failed", Toast.LENGTH_SHORT).show()
                        Log.d("MyTag", "CONNECTON FAILED: $")
                    }
                }
            }
        }
    }

    //myListener用于实时更新设置项的值
    // Listens for when the shared preferences change
    val myListener = OnSharedPreferenceChangeListener { sharedPreferences, s ->
        if (s == "x" || s == "y") return@OnSharedPreferenceChangeListener
        if (isFloatWindowExist) {
            floatWindowSize = TypedValue.applyDimension(
                TypedValue.COMPLEX_UNIT_DIP,
                sharedPreferences.getInt("size", 50).toFloat(),
                resources.displayMetrics
            )
                .toInt()
            params!!.width = floatWindowSize
            params!!.height = floatWindowSize
            params!!.alpha = sharedPreferences.getInt("tran", 90) * 0.01f
            windowManager!!.updateViewLayout(view, params)
        }
        canFloatWindowMove = sharedPreferences.getBoolean("canmove", true)
        invertX = sharedPreferences.getBoolean("invertX", false)
        invertY = sharedPreferences.getBoolean("invertY", false)
        sensityX = sharedPreferences.getInt("sensityX", 100)
        sensityY = sharedPreferences.getInt("sensityY", 100)
    }

    override fun onServiceConnected() {
        super.onServiceConnected()
        windowManager = getSystemService(WINDOW_SERVICE) as WindowManager

        //读取用户的灵敏度设置项等等
        sp = getSharedPreferences("data", 0)
        invertX = sp!!.getBoolean("invertX", false)
        invertY = sp!!.getBoolean("invertY", false)
        sensityX = sp!!.getInt("sensityX", 100)
        sensityY = sp!!.getInt("sensityY", 100)


        //注册广播接收器，用来接收陀螺仪进程发来的广播
        registerReceiver(mBroadcastReceiver, IntentFilter("intent.tuoluoyi.exit"))
        registerReceiver(mBroadcastReceiver, IntentFilter("intent.tuoluoyi.sendBinder"))
        registerReceiver(
            mBroadcastReceiver,
            IntentFilter("android.intent.action.CONFIGURATION_CHANGED")
        )
        isBroadcastRegistered = true

        //如果用户开启了”使用前台通知“，则发送前台通知
        if (sp!!.getBoolean("foreground", true)) {
            sendNotification()
        }

        //注册偏好变动监视器，用来实时更新用户的灵敏度设置等等
        sp!!.registerOnSharedPreferenceChangeListener(myListener)
        isSharedPreferenceRegistered = true
    }

    @SuppressLint("ClickableViewAccessibility")
    private fun showFloatWindow() {
        GetWidthHeight()
        floatWindowSize = TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_DIP,
            sp!!.getInt("size", 50).toFloat(),
            resources.displayMetrics
        )
            .toInt()
        params = WindowManager.LayoutParams(
            floatWindowSize,
            floatWindowSize,
            WindowManager.LayoutParams.TYPE_ACCESSIBILITY_OVERLAY,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE or WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL,
            1
        )
        params!!.alpha = sp!!.getInt("tran", 90) * 0.01f
        params!!.x = sp!!.getInt("x", 0)
        params!!.y = sp!!.getInt("y", 0)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) params!!.layoutInDisplayCutoutMode =
            WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES
        canFloatWindowMove = sp!!.getBoolean("canmove", true)
        view = ImageView(this)
        val rotation = windowManager!!.defaultDisplay.rotation
        view!!.visibility = if (rotation == 0 || rotation == 2) View.GONE else View.VISIBLE
        view!!.setImageResource(R.drawable.icon) //设置悬浮球的View
        //设置悬浮球的触摸响应

        // Logic for handling our floating window
        view!!.setOnTouchListener(object : OnTouchListener {
            var lastX = 0f
            var lastY = 0f
            var downTime: Long = 0
            var moved = false
            override fun onTouch(view: View, motionEvent: MotionEvent): Boolean {
                when (motionEvent.action) {
                    MotionEvent.ACTION_DOWN -> {
                        downTime = System.currentTimeMillis()
                        moved = false
                        lastX = motionEvent.rawX
                        lastY = motionEvent.rawY
                    }

                    MotionEvent.ACTION_MOVE -> {
                        if (!canFloatWindowMove) return true
                        val rawX = motionEvent.rawX
                        val rawY = motionEvent.rawY
                        val dx = Math.round(rawX - lastX)
                        val dy = Math.round(rawY - lastY)
                        lastX += dx.toFloat()
                        lastY += dy.toFloat()
                        if (Math.abs(dx) > 4 || Math.abs(dy) > 4) moved = true
                        params!!.x += dx
                        params!!.y += dy
                        windowManager!!.updateViewLayout(view, params)
                    }

                    MotionEvent.ACTION_UP ->
                        //如果是单击，则暂停/恢复陀螺仪服务
                        if (!moved) {
                            if (System.currentTimeMillis() - downTime < 200) {
//                                if (isGyroEnabled) { // Make sure to change isGyroEnabled
//                                    // DISABLE HERE!
//                                } else {
//                                    // ENABLE HERE!
//                                }
                                Log.d("MyTag", "Starting app via float window click")
                                val launchIntent = Intent(this@tuoluoyiService, MainActivity::class.java)
                                launchIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                                this@tuoluoyiService.startActivity(launchIntent)
                            } else {
                                isThumbLPressed = !isThumbLPressed
                                // Implement logic here for when the floating window is held and then released
                                view.setBackgroundColor(if (isThumbLPressed) Color.DKGRAY else Color.TRANSPARENT)
                            }
                        }
                }
                //自动贴边
                params!!.x = Math.min(
                    Math.max(params!!.x, -(SCREEN_WIDTH - floatWindowSize) / 2),
                    (SCREEN_WIDTH - floatWindowSize) / 2
                )
                params!!.y = Math.min(
                    Math.max(params!!.y, -(SCREEN_HEIGHT - floatWindowSize) / 2),
                    (SCREEN_HEIGHT - floatWindowSize) / 2
                )
                windowManager!!.updateViewLayout(view, params)

                //存储悬浮球位置
                sp!!.edit().putInt("x", params!!.x).putInt("y", params!!.y).apply()
                return false
            }
        })
        windowManager!!.addView(view, params) //显示悬浮球
        isFloatWindowExist = true
    }

    private fun sendNotification() {
        val notification = Notification.Builder(this)
            .setContentText(getString(R.string.noti_text))
            .setContentTitle(getString(R.string.noti_title))
            .addAction(
                Notification.Action(
                    android.R.drawable.ic_delete,
                    getString(R.string.noti_action),
                    PendingIntent.getBroadcast(
                        this,
                        0,
                        Intent("intent.tuoluoyi.exit"),
                        PendingIntent.FLAG_IMMUTABLE
                    )
                )
            )
            .setSmallIcon(Icon.createWithResource(this, R.drawable.tile))
            .setColor(getColor(R.color.bg))
            .setContentIntent(
                PendingIntent.getActivity(
                    this,
                    0,
                    Intent(this, MainActivity::class.java),
                    PendingIntent.FLAG_IMMUTABLE
                )
            )
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val notificationChannel = NotificationChannel(
                "daemon",
                getString(R.string.noti_channel),
                NotificationManager.IMPORTANCE_DEFAULT
            )
            notificationChannel.enableLights(false)
            notificationChannel.setShowBadge(false)
            notificationChannel.lockscreenVisibility = Notification.VISIBILITY_SECRET
            val notificationManager = getSystemService(NOTIFICATION_SERVICE) as NotificationManager
            notificationManager.createNotificationChannel(notificationChannel)
            notification.setChannelId("daemon")
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            notification.setForegroundServiceBehavior(Notification.FOREGROUND_SERVICE_IMMEDIATE)
        }
        startForeground(1, notification.build())
    }

    //获取设备的真实宽高(会计算导航栏和刘海区域。并且横竖屏时得到的宽高是相反的)。
    fun GetWidthHeight() {
        val metrics = DisplayMetrics()
        windowManager!!.defaultDisplay.getRealMetrics(metrics)
        SCREEN_WIDTH = metrics.widthPixels
        SCREEN_HEIGHT = metrics.heightPixels
    }

    override fun onDestroy() {
        super.onDestroy()

        MIDIOutputPort?.close()
        iGamePad?.close()
        if (isFloatWindowExist) windowManager?.removeView(view)
        if (isBroadcastRegistered) unregisterReceiver(mBroadcastReceiver)
        if (isSharedPreferenceRegistered) sp?.unregisterOnSharedPreferenceChangeListener(myListener)
    }

    override fun onAccessibilityEvent(accessibilityEvent: AccessibilityEvent) {}
    override fun onInterrupt() {}
    override fun onKeyEvent(event: KeyEvent): Boolean {
        if (event.keyCode == KeyEvent.KEYCODE_VOLUME_UP) {
            iGamePad?.pianoKey(120, event.action == KeyEvent.ACTION_DOWN)
            return true // Consumes the key event
        }
        if (event.keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) {
            iGamePad?.pianoKey(120, event.action == KeyEvent.ACTION_DOWN)
            return true // Consumes the key event
        }
        return super.onKeyEvent(event)
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        if (isFloatWindowExist) {
            GetWidthHeight()
            view!!.visibility =
                if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) View.VISIBLE else View.GONE
            view!!.setImageResource(R.drawable.icon)
            windowManager!!.updateViewLayout(view, params)
        }
        super.onConfigurationChanged(newConfig)
    }
}
