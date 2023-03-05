# lipo_battery_drainer
Hardware to safely discharge LIPO batteries to storage levels

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
