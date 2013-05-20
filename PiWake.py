#!/usr/bin/env python3

from quick2wire.i2c import I2CMaster, writing_bytes, reading
import argparse, os

i2cAddress = 0x41
#commands
i2cWakeMins = 0x10
i2cWakeHours = 0x11
i2cShutOffDelay = 0x12
i2cAddressChange = 0x13
i2cCancel = 0x14
i2cCamera = 0x15
#response
i2cOK = 0x01
i2cError = 0x02

command = False

#parse args
parser = argparse.ArgumentParser(description='Ask Arduino to wake Pi in X mins/hours. NOTE this script will call poweroff to start R-R-Pi shutdown')
group = parser.add_mutually_exclusive_group()
group.add_argument('-m', '--mins', type=int, help='Mins till wake up')
group.add_argument('-o', '--hours', type=int, help='Hours till wake up')
group.add_argument('-d', '--delay', type=int, help='Change power off delay, in mins')
group.add_argument('-a', '--camera', type=int, help='Hold Camera swicth for x millis')
group.add_argument('-c', '--cancel', help='Cancel shut off and wake', action='store_const', const=42)

args = parser.parse_args()

if args.cancel:
    print('cancel')
    command = i2cCancel
elif args.mins:
    print('mins')
    command = i2cWakeMins
    value = args.mins
elif args.hours:
    print('hours')
    command = i2cWakeHours
    value = args.hours
elif args.delay:
    print('delay')
    command = i2cShutOffDelay
    value = args.delay
elif args.camera:
    print('camera')
    command = i2cCamera
    value = args.camera

# send i2c
if command:
    with I2CMaster() as master:
        if command == i2cCancel:
            results = master.transaction(writing_bytes(i2cAddress, command), reading(i2cAddress, 1))
        else:
            results = master.transaction(writing_bytes(i2cAddress, command, value), reading(i2cAddress, 1))

        if results[0][0] == i2cOK:
            print('OK')
                # if wake mins or hours
            if command == i2cWakeHours or command == i2cWakeMins:
                # now call shut down command
                os.system( "sudo poweroff" )
        elif results[0][0] == i2cError:
            print('Error')