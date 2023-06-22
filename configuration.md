## Configuration

Profiles can be edited by connecting a computer to the unit via USB and starting
a termnial program.  On my Linux laptop, I used:

    minicom -b 115200 -P /dev/ttyUSB0

There are two types of configuration: global and profile.

### Global Configuration

Some settings are always used, no matter what profile is selected.

#### Current calibration

The `ical` command is used to calibrate the current measurements.  It's
deault value is 133 milliOhms.  By measuring the current with a multimeter,
you can calibrate this value to match your specific hardware:

    ical 0.130

Would set the resistance to 130 milliOhms 

#### Voltage calibration

With a battery connected, measure the voltage at the vcal test point and enter
it with the `vcal` command.  For example:

    vcal 25.4

Note there is a related `vdrop` parameter but this parameter is  per-profile to
accommodate different breakout boards (e.g. whether they have protection diodes).

#### Voltage sag measurements

You can control how often to take voltage sag measurements.

First is `vsag_interval_seconds` which determines how often to take a baseline
measurement.  For example:

    vsag_interval_seconds 15

Next is `vsag_settle_ms` which determines how long to allow the battery to recover before taking a measurement.  For example:

    vsag_settle_ms 1000

#### Fan Settings

Use `fan_celsius <min_percent> <min_celsius> <max_celsius>` to control any connected fan(s)
At `min_celsius`, the fan(s) will run at `min_percent` and smoothly ramp to
100% speed at and beyond `max_celsius`. Example:

    fan_t 20 40 80 

Use `fan_watts <min_percent> <min_watts> <max_watts>` to control fans by power output.

The settings for `fan_t` and `fan_p` are active at the same time.  Whichever
leads to the higher fan speed at a given moment will be used.

Note that on may fans, using too low of a percent leads to no rotation.

#### Finish Display Time

Use `finish_display <mah_ratio>` to configure the number of seconds
that the finish stats are shown before the unit shuts down.  Example:

    finish_display 0.5

In the example above, if 1000 mAh were pulled from the battery then the display would be active for 1000 * 0.5 = 500 seconds.  If 100 mAh were pulled, then
the display would be active for 100 * 0.5 = 50 seconds.

### Responsiveness

The system looks to open up the FETs as much as possible while keeping under
the current limits of:

* Max voltage sag
* Max current
* Max power
* Max temperature

Thus the goal is to reach one of these maximums while staying under the maximum
for all others.  Which maximum is reached will vary by battery, fan settings, room
temperature, etc.

Coming up with and tuning an optimal PID algorithm that meets the above
requirements would be daunting due to the large number of free variables. For
example we would need to combine all four terms above to define an "error" as a
starting point, then would still need to determine the PID constants.

Fortunately, we do not need to system to converge in minimum time.  By relaxing
this requirement, we can get away with less complex calculations. 

The tuning algorithm is an annealing type algorithm where we use the terms
"velocity" and "acceleration" as a mental model.  As an analogy, imagine trying
to adjust the volume knob on a friend's stereo.  You would likely start with
some gross adjustment until you overshoot the mark, then hunt with increasingly
fine movements until you are satisfied.  If a "loud" commercial comes on,
you will need to adjust it again - again starting a bit fast and fine tuning.

With that in mind, here are the parameters of the algorithm:

* *max_velocity*  The maximum (and starting) velocity in percent / second
* *min_velocity* The minumum allow change velocity in percent / second.
* *deceleration_factor* the amount to decrease the velocity on an overshoot
  (going past the intended mark): from 0 to max_velocity
* *acceleration* the amount to increase the velocity (not reaching
  the intended mark) in percent / second. from 0.0 to max_velocity

Example:

  * fet_max_velocity = 30
  * fet_min_velocity = 0.05
  * fet_deceleration = 0.5
  * fet_acceleration = 0.1

  say the per sample time is 100ms

  The PWM will scale from 0 percent to 100% at 3% per sample, taking 33 samples
  or 3.3 seconds to get there.

  Say instead we hit a limit on the 10th sample.  PWN at this point would
  be 3 * 10 = 30%.  The velocity is reversed and decelerated, becoming
  to (-30 + 0.5) * 0.1 = -2.95% per sample

  On the next sample, PWM will change to 30 - 2.95 = 27.05%  Say that now
  everything is below threshold.  This means that we may have backed off
  too far.  Thus we reverse and decelerate again.  The new velocity becomes
  (29.5 - 0.5) * 0.1 = 2.9% per sample.

  One the next sample, PWM will change to 27.05 + 2.9 = 29.95.  Say that we
  are still too low.  In that case, the acceleration factor kicks in and
  the new velocity is not inverted, becoming (29 + 0.1) * 0.1 = 2.91% per sample.

  and so on.

### Profile Management

#### Create a profile

You can create a profile with the `new` command and a name.  For example:

    new

Profiles are named "New" as a starting point.  Alternately you can duplicate an
existing profile under a new name to use it's settings as a starting point:

    duplicate 0

would take whatever profile is at index zero and copy it under the next
available index.

#### Rename a profile

Change the name of a profile with the `name` command.  e.g.

    name 0 "LIPO 3.8 percell"

#### Delete a profile

Use the delete command with the profile index.  e.g.

    delete 1

#### Move a profile

Move a profile from one index to a different one via `move <source_idx> <dest_idx>`.  e.g.

    move 3 0

#### View profiles

Use the `list` command to view profile names and indexes:

    list

Use the `show` command to view settings for particular indexes or to dump all indexes:

    show
    show 0 2
    show 1 2 3

### Profile Configuration

#### Voltage drop configuration

Use `vdrop <profile_index> <voltage>` to set the voltage difference between the connected
battery and the vcal test point.  This will take into consideration various FETs and
diodes that exist between the two points. Example:

    vdrop 1 0.75

#### Cell count

Use `cell_count <profile_index> <count>` to adjust the number of cells.  If set at zero,
the number of cells will be automatically calculated at start by dividing the voltage
by the `per_cell_target_volts` (see below).  If the automatic calcualtion would lead to incorrect
results in your situation, you'll need to fix the value.

Note that you can also use a value of `1` if you don't want to think in terms of cells
but only the full voltage.  Examples:

    cell_count 1 0
    cell_count 1 1
    cell_count 1 6

#### Target voltage

Sets the per-cell target voltage.  When the target voltage is reached, the unit
will shut off.  For example:

    per_cell_target_volts 1 3.8

Would set the target voltage to 3.8 if `cell_count` is 1, 7.6 if `cell_count` is 2,
etc.

    per_cell_target_volts 1 0.0

Will run the drainer until it can no longer power itself, regardless of the cell count.

#### Damage voltage warning

If the target voltage is set below the `damage_voltage`, the user will be told that
they are about to destroy the connected batteries and will ask for confirmation.
The default settings is for LIPO batteries and is set to 3.3 volts.  Change it
with the `damage_voltage` command:

    damage_voltage 1 3.0

#### Max current

Use `max_amps <profile_index> <current>` to set the maximum allowed current.  Example

    max_amps 1 10.0

#### Max per-cell voltage sag

Use `per_cell_max_vsag <profile_index> <sag>` to determine the maximum amount of 
per-cell voltage sag that is allowed.  For example:

    per_cell_max_vsag 1 0.4

Would allow up to 0.4 * 4 = 1.6V of sag on a 4 cell pack.

#### Max temperature

Use `max_celsius <profile_index> <temp>` to change the maximum allowed heatsink termerature.  Example:

    max_celsius 1 80.0

#### Max Power

Use `max_watts <profile_indx> <watts>` to change the maximum allowed wattage (voltage * current).  Example:

    max_watts 1 150

### Testing

The console provides a `fake_mv` command.  If used, the voltage sense logic will provide that value instead
of the actual voltage reading.  It is useful for doing testing with no battery connected (since you can't
otherwise make it to the charging screen)  Using this setting with a battery connected is not recommended.
