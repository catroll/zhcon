<?xml version="1.0" encoding="UTF-8"?>
<leo_file>
<leo_header file_format="2" tnodes="0" max_tnode_index="0" clone_windows="0"/>
<globals body_outline_ratio="0.504843918192">
	<global_window_position top="10" left="10" height="919" width="1270"/>
	<global_log_window_position top="0" left="0" height="0" width="0"/>
</globals>
<preferences/>
<find_panel_settings/>
<vnodes>
<v t="zhangle.20060428160317" a="ETV"><vh>@rst ../README.html</vh>
<v t="zhangle.20060428160700"><vh>What is zhcon?</vh></v>
<v t="zhangle.20060428160700.1"><vh>License</vh></v>
<v t="zhangle.20060428161113"><vh>Download and Install</vh></v>
<v t="zhangle.20060428161611"><vh>Quick Framebuffer Setup</vh></v>
<v t="zhangle.20060428161611.1"><vh>Hotkeys</vh></v>
<v t="zhangle.20060428161611.2"><vh>Thanks</vh></v>
</v>
</vnodes>
<tnodes>
<t tx="zhangle.20060428160317">==================================================
 zhcon -- A Fast CJK Virtual Console Environment
==================================================

.. contents::</t>
<t tx="zhangle.20060428160700">Zhcon is a fast virtual console Chinese environment running on modern unix systems like Linux and FreeBSD. It can display Chinese, Japanese or Korean double byte characters via VGA, framebuffer or libGGI. Supported language encodings include: GB2312, GBK, BIG5, JIS, KSC and UTF-8. It can also use input methods(table based) from M$ pwin98 and UCDOS for M$-DOG. More information can be found on http://zhcon.sourceforge.net</t>
<t tx="zhangle.20060428160700.1">All the source codes are published under GPL!(see COPYING)
The input methods used in zhcon are originally written by
ejoy&lt;ejoy@users.sf.net&gt; and are published under GPL too!
However currently the words of the ime are converted from M$ windows
&amp; UCDOS. Future version of zhcon will use its own input table to
purify its *free* spirit.
The bpsf font format used in zhcon is proposed by one of its core
developers Hu Yong&lt;ccpaging@etang.com&gt; based on psf format.See doc/
bpsf.txt for more detail.

</t>
<t tx="zhangle.20060428161113">The release source tar ball can be found on zhcon's project page onsourceforget:http://sourceforge.net/projects/zhcon. Many distributions, such as Debian and FreeBSD, also provide pre-compiled binary package for quick install.

The latest source code can be obtained from Subversion Repository:
     svn co https://svn.sourceforge.net/svnroot/zhcon zhcon
     
See file INSTALL for install information.

Full user manual (slightly out-dated) is located in doc/</t>
<t tx="zhangle.20060428161611">Set console as fb device
Add line to /etc/lilo.conf as below:

vga=0x303
image=/boot/vmlinuz
label=FrameBuf

#lilo -v
#reboot

The  following table shows the mode numbers you can input at
the VGA prompt  or  for use with the LILO program. (actually
these  numbers  are plus 0x200 to make it easier to refer to
the table)

========== ======= ======= ======= ======== ========= ========= 
Resolotion 640x400 640x480 800x600 1024x768 1280x1024 1600x1200
========== ======= ======= ======= ======== ========= ========= 
4 bits      ?          ?     0x302      ?        ?         ?
8 bits      0x300   0x301    0x303    0x305    0x307     0x31C
15 bits     ?       0x310    0x313    0x316    0x319     0x31D
16 bits     ?       0x311    0x314    0x317    0x31A     0x31E
24 bits     ?       0x312    0x315    0x318    0x31B     0x31F
32 bits     ?         ?       ?        ?        ?         ?
========== ======= ======= ======= ======== ========= ========= 






Key:  8 bits = 256 colours,  15 bits = 32, 768 colours,
16 bits = 65, 536 colours,   24  bits  = 16.8 million colours,
32 bits - same as 24 bits,  but  the extra 8 bits can be used
for  other  things,  and fits perfectly with a 32 bit PCI/VLB
/EISA bus.


Additional  modes are at the discretion of the manufacturer,
as the VESA 2.0 document only defines modes up to 0x31F. You
may  need  to  do  some  fiddling around to find these extra
modes.</t>
<t tx="zhangle.20060428161611.1">**Normal Mode**

===============   ============================================
CTRL-ALT_H        pop up online help
CTRL-SPACE        open/close input method
ALT_SPACE         show/hide input bar(open/close CJK mode)
CTRL-comma        toggle Full/Half char mode
CTRL-period       toggle Chinese symbol
CTRL-F1           set encode to GB2312
CTRL-F2           set encode to GBK
CTRL-F3           set encode to BIG5
CTRL-F4           set encode to JIS
CTRL-F5           set encode to KSC
CTRL-F7           toggle input style
CTRL-F9           toggle GB2312/BIG5 auto-detect mode
CTRL-F10          menu mode
CTRL-ALT-1-9      switch to input method 1 to 9
CTRL-ALT-0        English mode
===============   ============================================


**History Mode**

===============   ============================================
SHIFT_PAGEUP       scroll up half screen in history
SHIFT_PAGEDOWN     scroll down half screen in history
SHIFT_ARROWUP      scroll up one line in history
SHIFT_ARROWDOWN    scroll down one in history
===============   ============================================
</t>
<t tx="zhangle.20060428161611.2">When I wrote the original version of zhcon the following sources help me
a lot.
    
=========================================     ===========================================
Source                                        Help
=========================================     ===========================================
chdrv, jmce                                   they show me how a Chinese console works.
unicon                                        language encoding and font support,  great!
``/usr/src/linux/video/fbcon-*.c``            low level framebuffer routine.
``/usr/src/linux/drivers/char/console.c``     VT102 implementation.
MINIX source code                             VT102 implementation.
autoconvert                                   GB2312/BIG5 auto-detect&amp;convert
=========================================     ===========================================
</t>
</tnodes>
</leo_file>
