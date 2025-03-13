################################################################################
#Date: March 12th, 2025                                                        #
#File: deinit.py                                                               #
#Authour: Ben Haubrich                                                         #
#Synopsis: Deinitialize the PWM on Raspbian 12                                 #
################################################################################
#pip imports
import argparse
import subprocess
from pathlib import Path
#Local imports
from common import allChannelsUnexported

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog="DeinitPwm",
                                     description="Deinitialize the PWM",
                                     epilog="Created by Ben Haubrich March 12th, 2025")

    #By not providing a default value these are made required.
    parser.add_argument('peripheral', type=int, choices=[0],
                            help="PWM peripheral number."
                       )
    parser.add_argument('channel', type=int, choices=[0, 1],
                            help="PWM channel number."
                       )

    args = parser.parse_args()

    #Uncomment for help with debugging.
    #print("{}".format(args))

    #Only zero is allowed by this script's argparse, but leaving this here in case using pwm1 is implemented in the future.
    #`dtoverlay pwm1` doesn't seem to activate the overlay on Raspbian 12, leaving only PWM0 with either channel 0 or 1 to use.
    pwmPeripheral = 'pwm' if 0 == args.peripheral else 'pwm' + str(args.peripheral)

    #https://raspberrypi.stackexchange.com/questions/143643/how-can-i-use-dtoverlay-pwm/143644#143644
    #https://www.kernel.org/doc/html/v5.10/driver-api/pwm.html#using-pwms-with-the-sysfs-interface
    pwmOverlayUnexport = Path('/sys/class/pwm/pwmchip0/unexport')
    pwmOverlayEnable = Path('/sys/class/pwm/pwmchip0/' + 'pwm' + str(args.channel) + '/enable')

    try:
        with pwmOverlayEnable.open('w') as enable:
            enable.write('0')

        with pwmOverlayUnexport.open('w') as unexport:
            unexport.write(str(args.channel))
    except FileNotFoundError:
        pass

    #Only remove the overlay if no other PWMs are active.
    if (True == allChannelsUnexported()):
        cmakeCommand = ['dtoverlay',
                        '-r',
                        pwmPeripheral]
        subprocess.run(cmakeCommand)

    exit(0)
