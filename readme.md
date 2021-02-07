# ArcWelderLib:  Anti Stutter Libraries and Binaries

Converts G0/G1 GCode commands to G2/G3 (arc) commands and back again.  This can greatly compress most GCode files and potentially improve quality by preventing planner starvation.  It is especially useful when streaming over a serial connection (OctoPrint, Pronterface, Slicer Direct Printing), but has some potential advantages in other cases too depending on your firmware and board.

This software was designed for 3D printers but should also be useful in other cases.  Please note that the current version does not convert travel moves to G2/G3, but this will be remedied soon.

# Installation

Installation is a simple matter of downloading and extracting the latest release.  You can find all of the releases [here](https://github.com/FormerLurker/ArcWelderLib/releases), with the latest release on top.  Simply scroll to the bottom of the release and download the proper archive for your OS.

The archive will contain two folders:

* **bin** - Here you will find the two console applications:  ArcWelder and ArcStraightener.  Just copy these to your local machine and you can run them from the command line, from a script, or directly within most slicers.  See the sections below for instructions on how to use these applications.
* **lib** - This folder contains pre-compiled ArcWelder libraries that can be integrated into applications.  The GcodeProcessorLib library has functions for parsing gcode, tracking the printer's state and position, as well as several other goodies.  The ArcWelder library (requires GcodeProcessorLib) contains the core welding algorithm.

# Arc Welder Console Application

This is a multiplatform console application that can be used to run the Arc Welder algorithm from a command prompt.  Binaries are available for Windows, Linux, Raspbian, and MacOs.  See the [installation][#installation] section for information on how to download the console application.

## Running Arc Welder Console

Once ArcWelder (or ArcWelder.exe for Windows) is downloaded and copied to your machine, you can execute it in the following form:

```
{Path_To_Arc_Welder} {Options_Parameters} {Source_Path} {Optional_Target_Path}
```

**Overwriting the source file**

If you want to overwrite the source file, no target path is needed.  For example, if you are running the Windows version from the root of your C drive and want to weld a file called C:\thing.gcode, you could run the following command from the terminal:

```
C:\ArcWelder.exe C:\thing.gcode
```

That will replace thing.gcode with a welded version of the file.

**Creating a new file**

If you want your original file preserved, you can specify a target path like so:

```
C:\ArcWelder.exe C:\thing.gcode c:\thing.aw.gcode
```

That would create a new file called thing.aw.gcode and would leave the thing.gcode file alone.

## ArcWelder Console Help

The console program will output all of the options with the following command for Windows:

```
ArcWelder --help
```

## Arcwelder Version Information
Run the following command to view the current version, which is very useful for debugging purposes, or if you submit a github issue:

```
ArcWelder --version
```

## Arcwelder Console Arguments

ArcWelder has good defaults for most applications, but occationally you may want more control.  Many options are available that might be helpful depending on your situation.

ArcWelder uses the [TCLAP Templatized C++ Command Line Parser](https://github.com/mirror/tclap) for implementing the arguments, and I've been most impressed with it.

### G90 Influences Extruder
For some printers, sending a G90 or G91 command also changes the E axis mode.  This is required for any printer running Marlin 2+ or a fork of Marlin 2, for Smoothieware, and for Prusa Buddy Firmware (Prusa Mini).  Do NOT add this flag if you are running any other firmware.  If your firmware is not on this list but requires this parameter, please create an issue, and I will update the documentation.

* Type: Flag
* Default: Disabled
* Short Parameter: -g
* Long Parameter: --g90-influences-extruder
* Example: ```ArcWelder -g "C:\thing.gcode"```

### Resolution (Maximum Path Deviation)
ArcWelder is able to compress line segments into gcode by taking advantage of the fact that a bunch of tiny line segments can, when viewed from a distance, approximate a curve.  However, a true curved path will never match up exactly with a bunch of straight lines, so ArcWelder needs a bit of play in order to create arc commands.  The *resolution argument* tells ArcWelder how much leeway it has with the original toolpath to make an arc.  Increasing this value will result in more compression, and reducing it will improve accuracy.  It is a trade-off, but one that most slicers implement anyway in order to prevent too many tiny movements from overwhelming your firmware.  In fact, ArcWelder can produce toolpaths that are more accurate than simply merging short segments together, making it less 'lossy' than slicer resolution settings.

The default resolution is 0.05mm, which means your toolpaths can deviate by plus or minus 0.025mm.  In general, this produces excellent results and toolpaths that are indistinguishable from the originals with the naked eye.  For extremely high precision parts, you may decrease this value, but this will reduce the amount of compression that can be achieved.

Values above 0.1 are not recommended, as you may encounter overlapping toolpaths.  When using values above 0.1, I recommend you use a visualizer that supports arcs before running your print.

* Type: Value (millimeters)
* Default: 0.05 (+- 0.025)
* Short Parameter: -r=<decimal_value>
* Long Parameter: --resolution-mm=<decimal_value>
* Example: ```ArcWelder -r=0.1```

### Path Tolerance Percent (length)
This parameter allows you control how much the length of the final arc can deviate from the original toolpath.  The default value of 5% is absolutely fine in most cases, even though that sounds like a lot.  The key thing to remember here is that your firmware will break the G2/G3 commands into many small segments, essentially reversing the process, so the path length in your firmware will match the original path much more closely.

Originally, this setting was added as a safety feature to prevent prevent bad arcs from being generated in some edge cases.  However, since then a new error detection algorithm was added that makes this unnecessary.  In some cases, especially if your resolution parameter is large (above 0.1), this setting can be used to fine tune the generated arcs, so I left this setting in as is.  99+% of the time, no adjustments will be necessary here.

* Type: Value (percent)
* Default: 0.05 (5%)
* Short Parameter: -t=<decimal_value>
* Long Parameter: --path-tolerance-percent=<decimal_value>
* Example: ```ArcWelder --path-tolerance-percent=0.10```

### Maximum Arc Radius
Allows you to control the maximum radius arc that will be generated with ArcWelder.  This was added as a safety feature to prevent giant arcs from being generated for essentially straight lines.  ArcWelder does have built-in detection to prevent colinear lines from being turned into arcs, but slight deviations due to the precision of the gcodes (usually fixed to 3 decimal places) can cause arcs to be generated where straight lines would do.  Typically no adjustments are necessary from the defaults, but you can adjust this value if you want.

* Type: Value (percent)
* Default: 1000000.0 (1000000.0 millimeters or 1 kilometer)
* Short Parameter: -m=<decimal_value>
* Long Parameter: --max-radius-mm=<decimal_value>
* Example: ```ArcWelder --max-radius-mm=1000.0```

### Allow 3D Arcs
This option allows G2/G3 commands to be generated when using vase mode.  This is an experimental option, and it's possible that there are some unknown firmware issues when adding Z coordinates to arc commands.  That being said, I've gotten pretty good results from this option.  At some point, this will be enabled by default.

* Type: Flag
* Default: Disabled
* Short Parameter: -z
* Long Parameter: --allow-3d-arcs
* Example: ```ArcWelder --allow-3d-arcs```

### Allow Dynamic Precision
Not all gcode has the same precision for X, Y, and Z parameters.  Enabling this option will cause the precision to grow as ArcWelder encounters gcodes with higher precision.  This may increase gcode size somewhat, depending on the precision of the gcode commands in your file.

**Important Note**: This option used to be the default, but in some cases I've seen files with unusually high precision.  If it gets too high, the resulting gcode may overrun the gcode buffer size, causing prints to fail.  For that reason, this option has been disabled by default.  I've only seen a few cases where this happens, and it's always been due to custom start/end gcode with extremely high precision.

* Type: Flag
* Default: Disabled
* Short Parameter: -d
* Long Parameter: --allow-dynamic-precision
* Example: ```ArcWelder --allow-dynamic-precision```

### Default XYZ Precision
ArcWelder outputs fixed precision for X, Y, Z, I, and J parameters.  99% of the time the default of 3 decimal places is just fine.  If you need (want) more or less precision, you can alter this value.  In general, I do not recommend a value below 3 or above 5.

Note that when combined with the --allow-dynamic-precision argument, this represents the minimum precision.  It will grow if Arc Welder encounters gcode commands with a higher precision.

* Type: Value (integer decimal places)
* Default: 3 (3 decimals, example: 1.001)
* Short Parameter: -x=<integer_value>
* Long Parameter: --default-xyz-precision=<integer_value>
* Example: ```ArcWelder --default-xyz-precision=5```

### Default E Precision
Arc Welder outputs fixed precision for the E parameter (extruder travel).  99% of the time the default of 5 decimal places is what you want.  If you need (want) more or less precision, you can alter this value.  In general, I do not recommend a value below 3 or above 5.

Note, that when combined with the --allow-dynamic-precision argument, this represents the minimum precision.  It will grow if Arc Welder encounters gcode commands with a higher precision.

* Type: Value (integer decimal places)
* Default: 5 (5 decimals, example: 1.00001)
* Short Parameter: -e=<integer_value>
* Long Parameter: --default-e-precision=<integer_value>
* Example: ```ArcWelder --default-e-precision=3```

### Firmware Compensation
**Important**: Do **NOT** enable firmware compensation unless you are sure you need it!  Print quality and compression will suffer if it is enabled needlessly.

Some firmware does not handle arcs with a small radius (under approximately 5mm depending on your settings), which will appear flat instead of curved.  If larger arcs appear flat, it's likely that G2/G3 is disabled.  See [this closed issue for more details](https://github.com/FormerLurker/ArcWelderLib/issues/18), including some illustrations showing what firmware compensation does to your gcode.

This applies to Marlin 1.x (but NOT Marlin 2), Klipper (can be fixed by changing settings), and a few others.  If you notice small radius arcs that print with a flat edge, you may need to enable firmware compensation.  Note that compression may be reduced (perhaps drastically) when firmware compensation is enabled.

This feature is just a workaround, and the best solution will always be to either upgrade your firmware, which is especially important for people running Marlin 1.x or forks, or to adjust your arc interpolation settings (Marlin 2.x and above, Klipper, and others).  If you absolutely cannot upgrade your firmware, this may be your only options.

There are two arguments that need to be added to enable firmware compensation:

#### Millimeters Per Arc Segment
This is the default length of a segment in your firmware.  This setting MUST match your firmware setting exactly.  99% of the time this setting should be 1.0 for firmware compensation to work.

* Type: Value (millimeters)
* Default: 0 (disabled)
* Short Parameter: -s=<decimal_value>
* Long Parameter: --mm-per-arc-segment=<decimal_value>
* Example: ```ArcWelder --mm-per-arc-segment=1.0```
#### Minimum Arc Segments
This specifies the minimum number of segments that a circle of the same radius must have and is the parameter that determines how much compensation will be applied.  This setting was inspired by the Marlin 2.0 arc interpolation algorithm and attempts to follow it as closely as possible.  The higher the value, the more compensation will be applied, and the less compression you will get.  A minimum of 14 is recommended.  Values above 24 are NOT recommended.  In general, this should be set as low as possible.

If ArcWelder detects that a generated arc would have fewer segments than specified, it will reject the arc and output regular G0/G1 codes instead.  It's possible that a single arc will be broken into several G2/G3 commands as well, depending on the exact situation.  Note that ArcWelder will never increase the number of GCodes used, so it is limited by the resolution of the source gcode file.

* Type: Value
* Default: 0 (disabled)
* Short Parameter: -a=<integer_value>
* Long Parameter: --min-arc-segments=<integer_value>
* Example: ```ArcWelder --min-arc-segments=14```

#### Firmware Compensation Example
If you need to enable firmware compensation because you notice that small arcs appear flat, I recommend you start with the following settings:
```
ArcWelder --mm-per-arc-segment=1.0 --min-arc-segments=14
```

This should produce much more rounded small arcs.  However, in some cases you will want more detail (again, at the cost of compression, which reduces the effectiveness of ArcWelder), increase --min-arc-segments up to around 24.  I don't recommend going higher than this since you will start to get lots of uncompressed gcode in areas that need it.

## Slicer Integrations

It's easy to integrate ArcWelder with most slicers.

### Running ArcWelder From Cura

There is no way to plug the ArcWelder console application into Cura.  Fortunately there is a [plugin](https://github.com/fieldOfView/Cura-ArcWelderPlugin) in the [marketplace](https://marketplace.ultimaker.com/app/cura/plugins/fieldofview/ArcWelderPlugin) that integrates ArcWelder into Cura, developed by [FieldOfView](https://github.com/fieldOfView), so no worries there!

Note that if you are running Marlin 2, a fork of Marlin 2, Prusa Buddy (Prusa Mini), or Smoothieware, enable the 'G90 Influences Extruder' setting.  See the [G90 Influences Extruder](#g90-influences-extruder) section for more info.

If you are running Marlin 1 or any fork of Marlin 1 (Prusa Firmware for Mk2/Mk3, for example) or Klipper, you might consider enabling Firmware Compensation.  This is dependent on your firmware settings, but in general you will want to set *MM Per Arc Segment* to 1.0 and *Min Arc Segments* to 14 to enable firmware compensation.  Setting either to 0 will disable this.  See the [firmware compensation](#firmware-compensation) section for more info.

### Running ArcWelder from Slic3r, Slic3rPE, PrusaSlicer, and SuperSlicer

There are slight differences between these different slicers, but fortunately the method for integrating ArcWelder is the same.

1. Put the ArcWelder binary somewhere on your PC and copy the full path (see the installation section above).  For example:  ```C:\ArcWelder```
2. Select 'Print Settings'.
3. Depending on which slicer your are using, you may have to enable 'Advanced' or 'Expert' mode (PrusaSlicer requires this at least).
4. Select the 'Output options' sub menu.
5. In the 'Post-processing scripts', add the following command (your path to ArcWelder may be different):
```
c:\ArcWelder.exe
```
That will run ArcWelder on each sliced print with the default parameters.

6. Save your settings changes.
7. Slice a test file and open it, checking for the ArcWelder header at the top, which should look like this:
```
; Postprocessed by [ArcWelder](https://github.com/FormerLurker/ArcWelderLib)
```

Now you should be good to go!  

There are some parameters you might want to add.  For example, if you are using a printer running Marlin 1.0 or a fork of Marlin 1, you may want to enable firmware compensation.  Here's an example of that:
```
{path_to_arc_welder_here}\ArcWelder.exe --mm-per-arc-segment=1.0 --min-arc-segments=14
```

See the [firmware compensation](#firmware-compensation) section for more info.

If you are running Marlin 2.0, a fork of Marlin 2.0, Smoothieware, or Prusa Buddy (for the Prusa Mini), you will want to add the --g90-influences-extruder like so:
```
{path_to_arc_welder_here}\ArcWelder.exe --g90-influences-extruder
```

See the [G90 Influences Extruder](#g90-influences-extruder) section for more info.

### Running ArcWelder from Simplify3D

1. Edit your current process settings.
2. Click on the *Scripts* tab.
3. Add the following command (your path to ArcWelder may be different) like so:
```
c:\ArcWelder.exe "[output_filepath]"
```
**Important Note**: The quotes around the [output_filepath] tag are CRITICAL!  Omitting these will lead to intermittent issues depending on your gocde path and file name.  Also, **DO NOT** hard code an output path or file name.  Simplify3D will automatically replace the token **[output_filepath]** with the path and file name of our output file.

4. Now click 'OK' to save your settings change, load and slice a file, and save it somewhere.  Open it up and verify that the ArcWelder header appears at the top of your file.  It should look like this:
```
; Postprocessed by [ArcWelder](https://github.com/FormerLurker/ArcWelderLib)
```

Now you should be producing welded files directly from Simplify3D!

There are some parameters you might want to add.  For example, if you are using a printer running Marlin 1.0 or a fork of Marlin 1, you may want to enable firmware compensation.  Here's an example of that:

```
c:\ArcWelder.exe --mm-per-arc-segment=1.0 --min-arc-segments=14 "[output_filepath]"
```

See the [firmware compensation](#firmware-compensation) section for more info.

If you are running Marlin 2.0, a fork of Marlin 2.0, Smoothieware, or Prusa Buddy (for the Prusa Mini), you will want to add the --g90-influences-extruder like so:

```
c:\ArcWelder.exe --g90-influences-extruder "[output_filepath]"
```

See the [G90 Influences Extruder](#g90-influences-extruder) section for more info.

# Arc Straightener
This is the opposite of ArcWelder.  It will find any G2/G3 commands and replace them with G1 commands.  This is useful for testing firmware settings and generally seeing what the firmware is doing with your arc commands.

The arc interpolation algorithm used here is a port of the Marlin 2.x interpolation code.  This makes it especially easy to work with.  

I will add better documentation for this soon, but since it is a development tool, you'll probably be able to figure it out by using the --help argument like so:

```
ArcStraightener --help
```

