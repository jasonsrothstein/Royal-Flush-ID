MAIN.C:
--Main prototype code that produces correct output



reference_code.c:
-- I pulled this from the ECE 362 Lab 9 template. it outlines basic STM32 functionality for sending data over I2C. Its the beginnings of what I want to do with interfacing with our LCD

LiquidCrystal_I2C.h:
-- From the arduino/rpi library for this LCD screen. It lists all of the command variables, which is a great reference for knowing what bits to send for what outcome.

LiquidCrystal_I2C.cpp:
-- The actual functionality of the LCD library. It's written in C++, which is similar to C, but will not run. I tried to see if I could just convert it to C by hand, but there are too many structural differences. It is a good reference for general procedure/function
