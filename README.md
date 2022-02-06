# DigiPressController

This is the firmware for a boiler pressure controller. The hardware will provide the following setting controls:

* Potentiometer to control max PSI for boiler
* Potentiometer to control differential PSI (subtractive)
* Pressure transducer.

## Expected Behavior

The user will set the cutoff PSI using the appropriate potentiometer. Once that PSI is reached (measured by sensor), the relay will cut-off power to the boilers firing system. This state will persis until the PSI has reached a level below the setting minus the differential.

Currently the firmware limits a PSI target to no more than 5 PSI, and a differential to no more than 2 PSI. A well tuned steam system should not need more than a 2 PSI target.
