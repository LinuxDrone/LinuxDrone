What is the LinuxDrone project?
-------------------------------

  It is an attempt to create a really open source and open hardware
autopilot/UAV system based on Linux with real-time Xenomai co-kernel.
The goal is to use the same processor for all tasks including hard
real-time ones, not like a Linux as a governor for another flight
board.

Some of plans are:
 - Web-based LinuxDrone EmbeddedGCS™ (eGCS™), which will run directly
   onboard. For users it means that you can connect to the board via
   WiFi from any device: PC with any operating system, laptop, iPhone,
   iPad, Android phone/tablet, etc. And configure or tune board without
   any software or special hardware.
 - javascript scripting. For instance, you may write a script to take
   a 360 pano photo from a predefined POI and return home with a click
   of a switch. We are making new system completely scriptable. The
   base scripting language will be javascript, running onboard under
   node.js, popular platform for high-performance web applications.
   Its Google V8 engine provides all necessary power for that.
 - Linux userland: any software, applications, scripting languages can
   run directly onboard.
 - C++ for firmware development. Yes, it should be used carefully for
   embedded applications, but in the end it can be as good for that as
   plain old C, but with few advantages.
 - Open CC-BY-SA source code license. No hidden tags attached. Use it
   for any kind of applications, even commercial ones. Just mention us
   and contribute changed code back to the project.
 - Open and available hardware. Unlike some other projects, we decided
   to use commodity hardware boards like BeagleBone Black and Raspberry
   Pi. There are a lot of owners of such boards, and everyone can add
   few components to it and start flying his LinuxDrone. Moreover, the
   goal is to not use any specific hardware features of particular
   board. Yes, we can or even will use some (say, use both 200MHz PRU
   units on the TI chip of BBB to run PWM acquisition/generation), but
   as an alternative it could run i2c or CAN bus devices, main
   board-independent. So any Linux board which runs Xenomai-enabled
   kernel should be OK. But we will provide firmware images for
   BeagelBone Black and Raspberry Pi only yet.

The project should be exciting for both us and users.

LinuxDrone project sites
------------------------

The project provides feature-rich development and collaboration
environment using advanced tools such as GCC compilers, git, Atlassian
JIRA, Confluence, FishEye, Crucible, Bamboo, github mirror and forums.

Main project front-end:   http://www.linuxdrone.org/        (redirect to wiki)
Project forums:           http://forum.linuxdrone.org/  (multirotorforums.com)
Wiki, docs and manuals:   http://wiki.linuxdrone.org/   (Atlassian Confluence)
Bug and issue tracker:    http://jira.linuxdrone.org/         (Atlassian JIRA)
Project build server:     http://bamboo.linuxdrone.org/     (Atlassian Bamboo)
Repository browser:       http://code.linuxdrone.org/      (FishEye, Crucible)
Read/write source access: ssh://git@git.linuxdrone.org    (gitolite with keys)
Github repository mirror: https://github.com/osnwt/LinuxDrone      (read-only)
Toolchain downloads:      http://downloads.linuxdrone.org    (used by scripts)
