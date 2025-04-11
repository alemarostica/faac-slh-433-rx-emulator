# FAAC XR2 Emulator for Flipper Zero

As part of my bachelor degree in Computer Science I am creating an emulator of a FAAC XR2 receiver.

This serves no practical purpose and is, in fact, just a collection of facts and functionalities I observed through experimentation on a genuine FAAC XR2 device.
The Flipper Zero has made the research significantly easier thanks to the implementation of the FAAC SLH protocol in the [Unleashed Firmware](https://github.com/DarkFlippers/unleashed-firmware).
This app is for educational purposes only.

## Requirements
- A Flipper Zero with [Unleashed Firmware](https://github.com/DarkFlippers/unleashed-firmware) customized as follows:
    - In file ``unleashed-firmware/lib/subghz/protocols/faac_slh.c`` add:
        - under ```static bool allow_zero_seed = false;``` at line 25 add ```static bool already_programmed = false;```
        - under ```SubGhzProtocolDecoderFaacSLH* instance = context;``` at line 462 (463 after adding previous line) add ```already_programmed = false;```
        - ```instance->seed = data_prg[5] << 24 | data_prg[4] << 16 | data_prg[3] << 8 | data_prg[2];``` at line 592 (594 after adding previous lines) should become:
```c
if(!already_programmed) {
    instance->seed = data_prg[5] << 24 | data_prg[4] << 16 | data_prg[3] << 8 | data_prg[2];
    already_programmed = true;
}
```
For simplicity use [my fork](https://github.com/alemarostica/unleashed-firmware?tab=readme-ov-file) of the Unleashed Firmware.

### Explanation
The lib file is made so that every key received from a FAAC SLH remote updates the status of the Flipper Zero's receiver. We do not want this in the emulator of the receiver since receiving a pragramming mode key would mess up the status of the decoder for a previously memorized remote. To fix this we make it so when a remote has been memorized the internal seed on the receiver is not updated if another programming mode key is received. The seed is reset when the application is closed (the receiver is deallocated).
Notice that this will make everything that decodes a FAAC SLH Master Remote Prog signal not behave correctly.