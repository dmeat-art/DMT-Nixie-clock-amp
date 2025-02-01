<img src="https://github.com/dmeat-art/DMT-Nixie-clock-amp/blob/main/Images/Thumbs/Banner.JPG">

# Table of Contents

1. [Project timeline / story](#timeline)
2. [Concept Art](#concept)
3. [WARNING](#warning)
4. [Parts](#parts)
5. [Code](#code)
6. [Custom PCB](#pcb)
7. [Laser cut acrylic plates](#plates)
8. [Wood Templates](#wood)

# Project timeline / story <a name="timeline"></a>

I think I discovered the existence of nixie tubes around 15 years ago ? That's the moment I also noticed the timers in Final Fantasy 7 were recolored nixie tubes. From that time, I would search for Nixie Clocks on Youtube from time to time.

<img src="https://github.com/dmeat-art/DMT-Nixie-clock-amp/blob/main/Images/ff7_escape_timer_zoom.png">

Time passed, and later in life, in 2017, I got a small but stable income for the first time, and looked around for info and tutorials.

The decision to make my own nixie clock was taken. I am trained in art, enjoying traditional, digital, and 3D creation, but I also like making stuff in general. I did build some kits when I was a young teenager, but this was ages ago. But I also was really excited to make something cool, and I finally stepped in.

And I also got another kit for a nixie tube VU meter (using IN-13 tubes), with the goal of maybe combining the clock and an audio amplifier in one package.

The clock project back then was such :
- Display time and date
- Adjust the time and date
- Remember the time and date
- An alarm bell (an extremely loud shop door bell running in 220V, fun times !)
- Alarm time set up
- Alarm on/off switch

I'm absolutely not trained in electronic circuits design, so I was happy to find the [Arduinix](http://www.arduinix.com/)  board, it was a good base to get started !

By the first half of 2018, I had all my functions cabled up, with a complete menu system (long/short button presses to access and cycle through the options, and a rotary encoder to adjust the values). I wanted to use a realtime clock module to remember the time, but I could never make it work reliably.

Then, in 2018, bigger projects got in the way, and the project was disassembled and put in a box.

In 2023, I decided to finally assemble a [Lily58 split keyboard](https://github.com/kata0510/Lily58?tab=readme-ov-file) I had also bought years ago. Galvanized by this successful endeavour, I took out the nixie clock out and started working on it.

After bringing it back to life, I decided to simplify the project : out goes the RTC module, I decided to have a system that could synchronize itself from the internet.

I experimented a little bit with an ESP32, but there weren't enough available pins to interface with the arduinix shield. I then discovered that there now was a new arduino board with wifi included, and the same (or same enough) header pin layout, and decided to go with that.

I took the base I had already coded, cut out all the setup system, and included the wifi stuff (see ['code'](#code) section below).

After that, time passed again, I decided on the final design, and tried hard to get this project finished in summer 2024, before september and the end of the summer break. This didn't work out, but I didn't put the project away and used my (now very reduced) free time to push hard towards the conclusion of this project.

I reached the finish line on february, saturday 18, 2025.

# Concept Art <a name="concept"></a>

I wanted to share the research sketches I made over the years.

2017

2023

I made a 3D model of the clock in Blender to better vizualise it.
Link to the Artstation page to be added here later.

<img src="https://github.com/dmeat-art/DMT-Nixie-clock-amp/blob/main/Images/Thumbs/3D.png">

# WARNING <a name="warning"></a>

**I'm absolutely not trained in electronic circuits design !**

**The code and PCB design I made for this project do work for me, but should not be taken as-is without any review and modification for your own project !**

**Nixie Tubes use high voltage, be careful !**

**This is absolutely not a tutorial or a kit ! This page is more of a post-mortem, and the files are available for anyone to take a look. There are ALOT of improvisation and adaptations I made along the way when making the actual boxes and mounting the components inside.**

# Parts <a name="parts"></a>

- **Nixie Clock :**

The clock uses an arduino uno R4 Wifi, the Arduinix sheld, 4 WF Z5600M Nixie Tubes, 4 6mm Neon Lamp. There are some resistors on the mounting PCB to adjust the voltage to the neon lamps (10K ? I can't remember). Also a cool chrome button, a nice lever switch, and C14 power input, with switch and fuse.

- **Nixie VU meter Amplifier**

The VU meter kit is the SGVU151 (found on ebay) and it uses IN-13 tubes. I would advice to relocate the variable resistors on the board to an easier accessible spot for adjusting after the final assembly of the project.

The amp uses a board I found on Amazon that matched the power of the speakers I had on hand and wanted to use : ELA21306. I wanted both minijack input and bluetooth support, and this board works. I sodered wires on the input jack to offset it to a rotary selector, including the plug detection pin. I used a 4 way 3 pin commutator, got some nice aluminium knobs, input jacks, lever switches, and C14 power input, with switch and fuse.

With hindsight, I'm not satisfied with it and I might rework it some day, either with an amplifier only board, or with a raspberry pi 3 + Hifi shield. But not today.

# Code <a name="code"></a>

**CompleteNixie.ino**

I didn't write everything from scratch, the wifi stuff is pretty much copied from available examples, with credit to the author. The nixie multiplexing code was re-written from how I understood it from the given examples on the Arduinix Website.

** *Sorry, the code comments are a mish-mash of english and french.* **

Here's a quick run-down of the main parts of the code :
1. **Wifi Access point Mode :**
If the button is pressed down when the clock is powered on, a wifi access point is created. You can connect to it without any password. You then go to the IP adress specified in the code, enter your local wifi SSID and password, click on submit. These infos are saved in the arduino's EEPROM. The clock will "reboot" and get into "get time" mode.
2. **"Get Time" mode :**
The clock "boots" : it starts the wifi client, connects to your local wifi, then to the NTP Server, recovers the current time, and then disconnects and gets into Clock Mode.
3. **Clock Mode :**
The clocks displays the current time it got (or 1st january 1970 00:00 if it couldn't).
Every minute, the tubes display random numbers for a second to mitigate cathode poisoning. It will then display the year, then the month and day, and then the time.
Every friday at 9am, the clock will connect to your local wifi to re-synchronize.


# Custom PCB <a name="pcb"></a>

I don't know how to use PCB CAD software, but I know how to use vector drawing software, so I drew the PCB like that.

# Laser cut acrylic plates <a name="plates"></a>

The design uses laser cut acrylic plates. They are assembled with aluminum angle brackets and M3 screws and bolts, and I tapped most of the holes for direct mounting.
The diameter of the holes for the nixie tubes might actually be bad, I remember boring them with a step-bit to make them larger ^^'''

# Wood Templates <a name="wood"></a>

These files are a recap of the size and shape of all the wooden parts for the front of the clock and the amp. They are glued to the face plates using slow setting epoxy glue and creative claming.

The wood parts are made from cherry. I try to only work with hand tools. I prepared the 1x1cm stock and cut them to length and angle. The outer frame has a slight angle.
