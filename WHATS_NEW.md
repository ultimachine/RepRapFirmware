
Summary of important changes in recent versions
===============================================

Version 1.17b
=============

Printer status:
- New printer status "Tool change" implemented. This is recognised by DWC 1.14 and the forthcoming PanelDue firmware 1.16.
Bug fix:
- M226 (gcode-initiated pause) was hanging the printer (thanks chrishamm for the fix)
Z probe:
- For Z probe types 4 and higher, the MOD signal on the Z probe connector is driven high at the start of a probing move and low at the end. This is to help with certain types of Z probe, in particular accelerometer-based Z probes.
Other:
- Upgraded DWC files to 1.14 release (thanks chrishamm)

For upgrade notes, see version 1.17.

Version 1.17a
=============

Homing speeds:
- The speed factor (M220 command, or speed control in DWC and PanelDue) no longer affects the speed of homing moves or other special moves (i.e. G1 commands with S1 or S2 parameter)

Bug fixes:
- Fixed the M120 (Push) command
- Setting relative extruder movement (M83) or a feed rate (G1 Fxxx) in config.g now works again
- The F (feed rate) parameter is now processed correctly on a G1 command with the R parameter
- If you used M106 to change a fan PWM frequency, the change did not take place until you next changed the speed
- On boards with SAM3X processors, due to a chip bug the fan speed was sometimes incorrect if you changed the PWM frequency
- If an http client requested a non-existent file that was not an HTML file, the server incorrectly returned a 404 HTML page

For upgrade notes, see version 1.17.

Version 1.17
============

Implemented grid bed compensation:
- M557 defines the grid
- G29 probes the grid, and saves the height map to file and activates it
- G29 S1 loads and activates a height map
- G29 S2 clears the height map (so does M561)
- M374 also saves the height map, and M375 loads a height map
- M376 sets the compensation taper height
- Long moves are segmented when grid compensation is in use so as to follow the contours of the bed
- Duet Web Control 1.14 can display the height map graphically (thanks chrishamm)

Changes to information extraction from gcode files:
- Recognise generated-with comment written by newer versions of Cura
- Recognise filament usage info in kisslicer-generated gcode files (thanks chrishamm)

 M500, M501 and M502 now use config_override.g instead of flash memory. The parameters saved and restored are:
- M307 auto tune results
- PID parameters, if you used M301 to override the auto tune PID settings
- Delta printer M665 and M666 settings
- G31 trigger height, trigger value and X and Y offsets
- The M501 auto save option has been removed

 Duet 0.8.5/0.6 web server changes:
- Support gzipped files, and look for gzipped versions of files before looking for regular ones

 Changes to Z probe configuration and usage
- Z probe offsets are now applied during G30 probing with specified XY coordinates, including during delta auto calibration
- Z probe recovery time can be defined (R parameter in M558) and adds a delay between the travel move and the probing move
- Added T parameter to the G31 command to specify Z probe type. This allows you to view the parameters for the Z probe(s) and to set parameters for a particular Z probe type without selecting that type. G31 P or G31 P0 prints the parameters of the currently-selected Z probe.

 Changes to heater management:
- You can now specify a Steinhart-Hart C coefficient in the M305 command, for better thermistor accuracy over a wide temperature range. If you do use a non-zero C coefficient then you will need to change the B (beta) parameter as well. The B parameter should be the reciprocal of the Steinhart-Hart B coefficient.
- The thermistor disconnected detection now takes account of the thermistor parameters configured with M305. This should allow the Dyze thermistor to be used without getting so many 'thermistor disconnected' reports. You may need to use a small negative H parameter in your M305 command to make it reliable.
- M143 now takes an H parameter to specify the heater whose temperature limit you are setting. If it is not provided then heater 1 is assumed.
- M109 and M190 commands now support both R and S parameters in the same way as Marlin. If you specify temperature using the S parameter, the firmware will wait for the heater to heat up but not to cool down. If you specify temperature using the R parameter, the firmware will wait both when heating up and when cooling down.
- M104 and M109 default to tool 0 if no tool is selected and no T parameter provided
- M109 now selects the tool after setting the active temperature if it was not already selected
- M191 (set chamber temperature and wait) is now supported, with both R and S parameters as for M190. However you may wish instead to use M141 followed by M116 later to wait for all temperatures.
- Removed S and T parameters from M301 command. Use the M307 command instead.
- M301 with negative P parameter no longer sets bang-bang mode. Use M307 instead.
- Increased the default max temperature excursion to 15C
- Setting the temperature of a tool heater no longer sets the heater temperature unless the tool is currently selected or no tool is selected

Changes to M571 command:
- M571 now accepts a P parameter to select the output pin
- Added F parameter to M571 command to set PWM frequency

Bug fixes:
- Firmware retraction with Z hop now works when using 'retract on layer change' in slic3r
- Fixed bad JSON message during printing when there were no active extruders
- Software reset code storage/retrieval now works on Duet WiFi
- Fixed reset reason text because on the Duet WiFi a watchdog reset can look like an external reset
- Fix for adjusting the mix ratio during printing when using absolute extruder coordinates with mixing extruders, except for pause/resume
- Workaround for DWC 1.13 including a volume ID in the new file path when renaming files across directories
- Bug fix: M300 now causes a beep on PanelDue again (was broken in 1.16).
- Bug fix: when a move was aborted, the head position was incorrectly calculated if the move has a direction reversal scheduled later on. In practice this situation did not arise.

Miscellaneous changes:
- Multiple commands from different sources that do not interfere are executed concurrently. Previously, only a few status reporting commands could be executed concurrently with other commands. 
- If the M569 command is used with the 'R' parameter to set the enable polarity then the corresponding driver is disabled after the polarity is set.
- Experimental code has been added to log Z probe transitions during a move, for use when calibrating nozzle offsets.
- Removed the undocumented M201 max average printing acceleration parameter
- Added exception handlers and store a software reset code when an exception occurs
- Removed Duet WiFi prototype 1 build configuration and added RADDS build configuration
- Tool offset is no longer applied to G1 moves with S1 or S2 modifiers
- A 2nd controlled fan using an external mosfet driven by expansion connector pin 25 on a Duet 0.6 is no longer inverted

Upgrade notes for 1.17 - VERY IMPORTANT TO AVOID DAMAGE!!!
----------------------------------------------------------
- On the Duet 0.6 and 0.85 the default direction for the X motor is now forward (as it is for all other motors and on the Duet WiFi). If you do not have a M569 P0 command in your config.g file then you will need to add M569 P0 S0 in order to keep the previous behaviour.
- On the Duet 0.6 and 0.85 the default bed thermistor resistance at 25C is now 100K (as it is for the Duet WiFi). If you have an Ormerod, Huxley Duo or RepRapPro Mendel printer with a 10K bed thermistor, you will need to add parameter T10000 to the M305 P0 command in config.g if you don't have that already.
- On the Duet 0.6 and 0.85 the default hot end heater thermistor parameters are changed to match the Semitec thermistor used in the E3DV6 and other popular hot ends. If you have a printer built from a kit supplied by RepRapPro then you should use the following parameters in your M305 P1 command to restore the previous behaviour: B4138 C0
- The default bed temperature limit is 125C. Use M143 H0 S### if you need to increase it.
- If your printer has multiple hot end heaters and you use the M143 command to change the temperature limit, you will need to use one M143 command with an appropriate H parameter for each heater.
- If you limit the maximum heater PWM using the S parameter in a M301 command, you will now have to use M307 to do this instead. Similarly the M307 A parameter takes the place of the M301 T parameter (divide the T parameter by 255 and take the reciprocal to get the A parameter).
- On the Duet 0.8.5 any parameters you saved to flash memory will be lost when upgrading to RC1 or later.
- You will need to add M501 in your config.g file, at the end or just before the T0 command if you have one, if you want to load saved values automatically at startup.
- If you use auto delta calibration and you have Z probe X and/or Y offsets defined, you should adjust your probe points to allow for the fact that the firmware will now place the probe over the points you specify instead of the nozzle
- If your pause.g and resume.g contain any extruder movement commands to retract or prime filament, make sure these files have the M83 command at the start to select relative extruder coordinates
- The recommended web interface is Duet Web Control 1.14. To use DWC 1.14 on the Duet WiFi, you must be using version 1.02 or later of DuetWiFiServer. Version 1.03-ch is recommended.

Known issues for 1.17
---------------------
- The M120 (Push) command doesn't work in macro or gcode files, because it causes execution of the file to be terminated
- Setting relative extruder movement (M83) or a feed rate (G1 Fxxx) in config.g has no effect once config.g has completed
- The F (feed rate) parameter is not processed correctly on a G1 command with the R parameter. In the resume.g file, you can use a G1 Fxxx command to set the feed rate, then leave the F parameter off the G1 R1 command.
- If you enable tool mixing, you should use relative extrusion only. If you use absolute extrusion, then if you pause and resume the print, the extruder is likely to extrude the wrong amount of filament in the first move after resuming.
- Firmware retraction won't work properly if you have created additional axes, for example the U axis on an IDEX machine.

Version 1.16
============

- Support the DueX2 and DueX5 expansion boards for the Duet WiFi
- Add support for up to 3 additional axes U, V and W. The number of axes is reported to DWC and to PanelDue.
- Add support for X axis remapping in the M563 tool creation command
- Add support for default fan remapping in the M563 tool creation command
- Support dual material prints and dual simultaneous prints on IDEX (independent dual X carriage) printers. See [https://duet3d.com/wiki/Configuring_multiple_independent_X-carriages_on_a_Cartesian_printer].
- Support minimum fan speeds and fan PWM blipping when starting fans from standstill
- Files uploaded to SD card are now time-stamped (thanks chrishamm)
- Sending M307 A-1 C-1 D-1 disables the PID for a heater channel, allowing its pin and driver to be used for other purposes
- Sending M106 P# I-1 disables the specified fan so that its control pin can be used as for general purpose output (M42) or a servo (M280). Caution: the polarity may not be what you expect.
- Changed M42 pin numbering, see [https://duet3d.com/wiki/Using_servos_and_controlling_unused_I/O_pins]
- Added M280 servo support, see [https://duet3d.com/wiki/Using_servos_and_controlling_unused_I/O_pins]
- Allow separate firmware un-retract speed to be configured in M207
- Allow negative extra un-retraction in M207
- Support expansion connector pin PB6 on the Duet WiFi and use it by default for a cooling fan tacho input
- Added Z probe type 6 (switch on E1 endstop connector)
- Added optional I1 parameter to the M558 command to reverse the sense of the Z probe reading. This replaces M574 E0 S0 when using Z probe type 4 and also works with other types of Z probe.
- Support heater 6 on Duet 0.8.5 (untested and probably incomplete). You need to send a M307 H6 command with valid model parameters to enable heater 6, and Fan 1 will be disabled (they share the same control signal).
- On the Duet 0.8.5, Fan 1 no longer defaults to thermostatic but instead defaults to fully on. This is in case you have heater 6 connected.
- Sending a T command to select a tool no longer runs the tool change macros if the specified tool is already selected
- M122 command now includes the status of all TMC2660 drivers on the Duet WiFi and expansion boards
- Bug fix: if there were very many files in the /gcodes folder of the SD card then DWC would give and Ajax error when trying to load the files list (thanks chrishamm)

Upgrade notes:

- If you are using M42 then you will need to adjust the pin numbers in your M42 commands
- If you are using a type 4 Z probe (i.e. switch connected to E0 endstop input) and you are using M574 E0 S0 to invert the polarity, you will need to use the I1 parameter on the M558 command instead
- If you are using a Duet 0.8.5 and you were relying on Fan 1 being thermostatic by default, you will need to configure it yourself by adding command M106 P1 T45 H1:2:3:4:5:6 to config.g. 
- The recommended web interface is DWC 1.13
- You can use either DuetWebServer-1.03-ch or DuetWebServer-1.02 with this release. You may find that the -1.03-ch version is faster and provides a more reliable connection.

Version 1.15
============

- Implemented automatic heating model calibration and PID tuning. See [https://duet3d.com/wiki/Tuning_the_heater_temperature_control] for details and instructions.
- Implemented much better heater safety monitoring, based on a model (first-order-plus-time-delay) of each heater/sensor system, using default models if none have been configured
- Implemented M38 (thanks Chrishamm).
- Added support for an SD card socket on the SPI bus, and implemented M21/M22 to support this. This was done primarily for the Duet WiFi and has not yet been tested on the wired Duets.
- Added support for new Duet Web Control functions, in particular the sys file editor (thanks chrishamm)
- Increased the PWM resolution
- Included the status of the SD card-detect signal in the M122 response
- Further improvements to the speed of step pulse generation
- Improved motion smoothness when printing circles and complex shapes at high speed
- Bug fix: PID parameters entered using M301 had the I parameter doubled and the D parameter halved, and vice versa for PID parameters reported by M301
- Bug fix: the object height was sometimes incorrectly extracted from gcode files
- Bug fix: slicer comments in gcode files that included certain characters (e.g. '\') would give ajax errors when the file information was returned
- Bug fix (1.15e): using some nonzero values of M572 pressure advance could cause incorrect extruder movement and layer shifts
- Bug fix (1.15e): step errors were not always recorded
- Further changes specific to the Duet WiFi firmware build, see [https://www.duet3d.com/forum/thread.php?pid=1104#p1104] for details

Upgrade notes:

- You may get heating faults reported if you do not tune the heaters
- If you want to continur using tour own PID parameters instead of auto-tuning, you must double your I parameters and halve your D parameters in your M301 commands
- The recommended web interface is DWC 1.12

Version 1.14
============

- When multiple motors are due to step, generate all the step pulses simultaneously
- Added support for multiple drivers for a single axis (M584)
- Added support for extended step pulse width when using external drivers (T parameter on M569 command)
- Removed XYZE parameters from M569 (use M584 instead)
- Added M913 command (set motor % of normal current), allowing the motor current to be temporarily reduced e.g. for homing and loading filament
- Added Z probe type 5, which is a normally-closed switch or active-high 3.3V digital signal connected to the IN pin of the Z probe connector on the Duet 0.8.5 and Duet WiFi
- Duet WiFi default Z probe threshold is now 500 (the recommende dvalue for most types of Z probe)
- Duet WiFi defaults to using a low end homing switch on the X axis (Ormerod and Huxley, users take note!)

Upgrade notes:

- If you are using M569 commands to remap axes and extruders to different drives, that will no longer work. Use the new M584 command instead.
- If you use the M584 command, it should come before any M350 and M906 commands in your config.g file.

Version 1.13
============

* First firmware release for the new Duet WiFi
* Added support for M581 and M582 commands, including 'only if printing a file' condition in M581 command
* Fixed M117 for PanelDue (needs PanelDue firmware version 1.14)
* Firmware update messages are now sent to USB and PanelDue (needs PanelDue version 1.14 firmware to display them)
* M122 responses are now sent only to the comms channel that requested them
* Added gcode queue underrun counter, displayed in M122 Move diagnostic info
* SD card interface speed is included in M122 Platform diagnostic info
* Added support in M997 command to update additional firmware modules on Duet WiFi
* Corrected a possible problem with multiple incompatible gcodes being executed concurrently
* Support H parameter on M0 and M1 commands
* Treat M25 within the file being printed the same as M226
* Added additional M37 simulation modes to help identify bottlenecks
* Process M0 and M1 when in simulation mode
* When executing M0/M1 commands and no print is paused, execute stop.g/sleep.g if they exist
* Reduced interrupt latency, by not disabling all interrupts when starting a new move, to avoid losing characters sent by PanelDue
* Temporary fix for RTD temperature spikes causing failed prints
* Don't print extruder positions in M114 because they are always zero
* Allow setting of fan PWM from 50% upwards when a fan is in thermostatic mode
* Reduced default extruder heater PWM frequency to 500Hz
* Axes are flagged as not homed after using M350 to set microstepping

Upgrade notes:

The recommended web interface is DWC 1.11 for the Duet, and 1.11a-dc42 for the Duet WiFi.

Version 1.12
============

* PT100 and other RTD sensors are now supported
* When a print is paused and then cancelled, the firmware now attempts to run file sys/cancel.g. It only turns the heaters off if that file is not found.
* The file upload speed over the web interface has been increased. Those who were getting slow file upload speeds are likely to see the most benefit.
* If a chamber heater is configured, it is now exempted from the heater timeout, just like the bed heater
* Two changes have been made to improve print quality when printing at high speeds. First, enabling extruder pressure advance is less likely to result in sequences of short moves having sawtooth velocity profiles. Second, a "Maximum average printing acceleration" can now be configured (M201 P parameter). Using this to restrict average acceleration will flatten out any remaining sawtooth velocity profiles.
* The firmware now reports itself to the USB subsystem as "Duet 3D printer control electronics",instead of as an Arduino Due. A Windows driver file is provided.
* The default maximum hot end temperature is reduced to 260C.
* Bug fix: the USB interface exposed by firmware version 1.11 was not recognised by some versions of Windows on some computers, resulting in a driver installation error
* Bug fix: simulation mode (M37) did not work
* Bug fix: setting the motor current on the non-existent 9th driver on a Duet 0.6 might have undesirable side-effects

Upgrade notes:

- If you are using Windows, please install the driver at [https://github.com/dc42/RepRapFirmware/tree/dev/Driver]. This driver does not install any new binaries, it simply tells Windows to use its usbser.sys driver to communicate with a Duet. If you are using Windows 10 then you don't need to install the driver if you don't want to, but then the Duet will show up as "USB Serial Device".
- If you print with hot end temperatures above 260C then you will need to add a M143 command in config.g, for example M143 S280 will increase the temperature limit to 280C.
- The recommended web interface is DWC 1.11.
