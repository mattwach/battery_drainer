# lipo_battery_drainer
Hardware to safely discharge LIPO batteries to storage levels

## Schematic Overview

Note that the images below are a snapshot of the schematic and may not be fully up-tp-date.  See the kicad original for the latest design.

### Power Dissipation

This is a set of 4 P-Channel MOSFETs connected in parallel:

![power fets](images/power_fets.png)

More or less could also work.  More means more overall current and better heat
dissipation but also additional cost and board space.

These are intended to be run in the "constant" current region of the FETs which
is controlled by the gate-to-source voltage (Vgs) as exampled in the FQP27P06
datasheet graph below:

![vgs curve](images/vgs_curve.png)

### Vgs Control

To get the proper Vgs for the MOSFETs, we use an RC circuit as detailed below:

![rc circuit](images/rc_circuit.png)

The main element here is the 10u capacitor on the right side of the image.  This capacitor is filled and emptied with charge to set the Vgs that each MOSFET will see.

Filling the capacitor is the 5k resistor, R16.  If only this resistor and the capacitor existed, then the RC constant would be 5000 * 10e-6 = 50ms.  When the
capacitor is fully charged, the MOSFETs will be turned off.

The two transistor networks are used to drain the capacitor.  The one on the left
is the "slow" drain and the one on the right is the "fast" drain.  The size of the resistors (R17 and R20) at the collector determines the drain speed.  A microcontroller feeds in a PWM signal with a varying duty cycle to control how
much charge they pull from the capacitor thus determines the Vgs value.

In the steady state, we can assume that SLOW and FAST are not driven at all (high Z).  In this state the two 50k pulldowns (R14, R18) turn off Q6 and Q9
allowing the capacitor to fill up and turn off all MOSFETs.

## Inrush protection

There is inrush current potential on initial plugin as the 100ms or so it takes
the capacitor (C4) to charge via R16 could allow a high current to pass through
the FETs for that time period.  This is mitigated by the following inrush
protection circuitry:

![inrush](images/inrush.png)

Fully understanding this block will require studying the full schematic.  A
partial understand can be gained from the image above.  We basically have
another way to fill the capacitor C4 which is "enabled" when the battery is
plugged in but the user has not pressed the power button yet.  The way this
works is that the source of the MOSFET (2) is connected to the battery and the
small 10 ohm R13 allows for a rapid fill.  R8 is connected to the
microcontroller power and is thus grounded on initial power on, turning "on" the
FET.

When the user powers on the device, there is voltage near-battery going to R8
which turns off the FET and disables this subcircuit.

### Current Sense

![i sense](images/i_sense.png)

One of the three ways the microcontroller decides where to set Vgs is by
monitoring the current flowing through the FETs  (the other two are voltage and
power).  This is done with a low-side shunt which is a resistor that indicates
the current via a voltage drop.  This voltage drop is measured with an ADC on
the micorcontroller.  In many cases, minimizing the power loss through this
shunt is desirable so a small resistance will be chosen and the corresponding
low voltage drop will be amplified in an attempt to get enough ADC resolution.

Becuase powerloss is the goal here, we instead choose power resistors that directly provide a full (3V) drop at around 30V input.  To safely get there, I chose 3 35W 0.4 ohm resistors connected in parallel for a total dissipation capability of ~100W and an equivilent resistance of 0.133 ohm.

A zener diode (U5) is used to protect the ADC of the microcontroller in the event
that the divided voltage is too high (> 3V).  The micorcontroller firmware may have to know that any measurement above 3V translates to >= 3V and not exactly that.  In normal use, this case should not occur.

### Voltage Sense

![v sense](images/v_sense.png)

The microcontroller monitors the overall battery voltage to determine a sag value and to determine when the drain process is completed.

The circuit is a simple voltage divider with a capacitor to help stabilize the reading.

Sag is determined by periodically turning off the FETs (say for one second every 15 seconds) and measuring the "unloaded" voltage of the pack.  This can then be
used a reference value to determine the sag.

The voltags at R9 is not exactly the battery voltage as it has to pass through a a diode and FET.  The micorcontroller calculations will attempt to compensate for the drop.

### Power cutoff

The circuit is designed to draw nearly zero power (outside of parasitic losses) which it is off, including after the charging has completed.  Thus the user can leave the unit unattended (assuming the needed precautions have been taken) without concern of overdraining.

This is implemented with the following circuit:

![power cutoff](images/power_cutoff.png)

The Q2 FET is key.  It determines if the miccontroller gets any power.  If the microcontroller has no power, then the power FETs described earlier naturally enter an "off" state.

Q2 is off by default, turned off by R2.  There are two ways to turn it on:

1. If the user presses SW5, then the FET will be pulled to ground turning on Q2
2. If the microntroller activates Q1 via "EN", then the FET will be turned on as well.  The microtroller does this as soon as it can (in a split second) and hold it on until the mocrocontroller decides that it is time to power down.

While the unit is already on, the state of SW5 can be determined by checking the "ON" net, which is connected to an input of the microcontroller.  It's up to the
firmware to decided what to do (if anything) when the switch is pressed.

### ADC reference

![adc reference](images/adc_reference.png)

When using an ADC, one can typically choose from several different sources for the ADC max voltage with the internally-generated 3.3V source as the most convenient.  The downside of choosing this source is accuracy as the source is not optimized for the application.

This design uses the alternate `ADC_VREF` input with a `LM4040` voltage reference
to allow for more accurate measurements.

### OLED connection

![oled](images/oled.png)

The design breaks out an I2C connection that is typical for an I2C OLED.  A 128x64 design is the intended unit but anything that supports I2C could be supported with appropriate firmware.

### Fan connection

The LIPO generator effectively converts battery energy to heat, thus you'll need a cooling strategy to avoid overheating and damaging the discharge circuit.  Like cooling other devices such as CPUs and GPUs, a passive solution is sometimes adequate and a active solution is often needed.

The circuitry below supports a PWM-based fan controller.

![fan](images/fan.png)

This controller feeds the full battery voltage into the fan as PWM pulses.  These pulses interact with the natural inductance of the motor to create a basic "buck converter" circuit.  The diode "D1" acts as a flywheel, allowing the motor to power itself when the Q3 is closed and avoiding negative voltage spikes at the drain of Q3.

There are many motor controllers ICs available on the market.  The reason they were not chosen here is because the maximum battery input voltage (30V) exceeds the specifications of most of these controllers.  Also many of these controllers provide an H-bridge solution, which is overkill here (we don't need to run the fan in reverse).

### Optional Buttons

Finally, we have a bank of optional buttons that may be used for various things.  This will be decided by the firmware.

![buttons](images/buttons.png)

## Calibration

### Voltage sense

#### Path

* say the original battery voltage is 20V
* It will be reduced passing theough the protection diode, say 0.4V
* It will take some hit through the PNP FET
* This voltage will be divided according to the resistor ratio
* and finally converted into a 12 bit value

We can easily convert the 12 bit value into a voltage from 0-3V

Calibration:

1. Connect a battery but do not connect to a computer.  Do not start discharging.
2. Measure actual battery voltage
3. Measure GND to CalV

Calculated constants:

* ADC Scalar: Sadc = CalV / ADC
* Voltage Delta: Vd = Vb - Calv

Example:

Say resistor values are 9000 top, 1000 bottom.  Voltage ratio in this case would be 1000 / 10000 = 0.1
Adc range is 0-4096 over a 0-3V range

Say you measure 20V at the battery and 19V at Calv
Divided voltage is 19 * 0.1 = 1.9
ADC measures 4096 * 1.9 / 3.0 = 2594

Sadc = 19 / 2594 = 0.007325
Vd = 1

Say you measure 19V at the battery at 18V at Calv
Divided voltage is 18 * 0.1 = 1.8
ADC measures 4096 * 1.8 / 3.0 = 2457

Sadc = 18 / 2457 = 0.007326
Vd = 1

Now we have a ADC value or 2000, what is the voltage?
2000 * 0.007325 = 14.65 V
Plus 1V Vd = 15.64 volts

### Current Sense

