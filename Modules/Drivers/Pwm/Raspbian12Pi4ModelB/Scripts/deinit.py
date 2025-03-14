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

    parser.add_argument('channel', type=int, choices=[0, 1],
                            help="PWM channel number."
                       )

    args = parser.parse_args()

    #Uncomment for help with debugging.
    #print("{}".format(args))

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
    if (allChannelsUnexported()):
        print("All channels unexported. Removing overlay")
        cmakeCommand = ['dtoverlay',
                        '-r',
                        'pwm']
        subprocess.run(cmakeCommand)

    exit(0)
