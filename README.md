# Note
This app is still in development due to the fact that I wasn't able to come up with a solution to work with the keystroke input delay when attempting to use Piano Rooms mode. 
Additionally, I hadn't implemented a solution to hold down multiple keys when using qwerty mode since ONE emulated physical keyboard has a limit of 6 regular qwerty keys that could be held down simultaneously.(meta keys are handled slightly differently but that's irrelevant to the problem). I had thought of some solutions like emulating more than one physical keyboard at once and various others but couldn't really find the time nor motivation to do it.
At the app's current state, it works best in qwerty mode in a roblox game that allows you to turn on sustain.

If you have experience with android development then help would be greatly appreciated.

TL;DR: Piano rooms mode has delay problems, Qwerty mode can't hold keys, There are solutions but no motivation
# RobloxAndroidMidi

This is an Android app that creates a virtual physical Keyboard via the linux UHID API to be able to convert MIDI signals into keyboard key strokes.

Roblox DOES NOT accept virtual keystrokes sent to it via any means without registering a corresponding physical keyboard and therefore, a computer keyboard must be emulated.

The source of the MIDI signals could either be a real electric piano you have plugged in via a MIDI cable(aka printer cable) or a MIDI file(for autoplaying).
In simpler terms, you can use your piano or a music file to play music in Roblox Games that offer a piano.
Eg: Piano Rooms, Piano Keyboard v1.1, Digital Piano, Piano Visualizations 2, etc.


There are three ways for you to open this app for convenience\'s sake.

1. You have the normal app button that shows up in your launcher like any other app.
2. In your notification bar, there is a tile which you could add to the notification tile tab which\'d allow you to launch the application quickly
3. Finally, a floating icon on your screen which appears whenever you start the MIDI Background Service

You have two primary modes, Qwerty mode and Piano Rooms.

Qwerty Mode is how the majority of piano games on Roblox function via keyboard. Qwerty mode is limited as it doesn\'t carry over information like note velocity(softness/loudness), currently held notes information(can\'t hold notes, which is a keyboard limitation[workaround available but then most roblox piano games don\'t even support it and it\'s too much work(feel free to send a PR)], sustain pedal, limited octave ranges(how high and low you can go), etc.

\Piano Rooms mode, named after the Roblox game Piano Rooms(https://www.roblox.com/games/10888259502/Piano-Rooms), on the other hand was made to specifically work with Piano Room\'s(https://www.roblox.com/games/10888259502/Piano-Rooms) Midi Connect functionality.
Unlike Qwerty mode, Piano Rooms DOES support note velocity(softness/loudness), held key information(CAN hold multiple notes properly), sustain pedal, bigger octave ranges(how high and low you can go), etc.
The only caveat with it is the fact that Piano Rooms is the only game on Roblox that supports that mode.

Huge thanks to WuDi-ZhanShen, yume-chan and the scrcpy project as this wouldn\'t have been possible without them.
