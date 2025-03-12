################################################################################
#Date: March 11th, 2025                                                        #
#File: init.py                                                                 #
#Authour: Ben Haubrich                                                         #
#Synopsis: Initialize the PWM on Raspbian 12                                   #
################################################################################
import argparse
import subprocess
from pathlib import Path

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog="InitPwm",
                                     description="Initialize the PWM",
                                     epilog="Created by Ben Haubrich March 11th, 2025")

    #This first positional argument holds one or more arguments (nargs='+') so that when new positional commands are add below
    #They are contained within the list of arguments for the first positional argument. In this way, a list of possible
    #commands can be searched through by the name of the commands given.
    parser.add_argument('command', type=ascii, nargs='+', default='None',
                            help=argparse.SUPPRESS
                        )

    #By not providing a default value these are made required.
    parser.add_argument('peripheral', type=int, choices=[0, 1],
                            help="PWM peripheral number."
                       )
    parser.add_argument('period', type=int,
                            help="PWM period in nanoseconds."
                       )
    parser.add_argument('duty', type=float,
                            help="PWM duty cycle in percentage."
                       )

    args = parser.parse_args()

    #Uncomment for help with debugging.
    #print("{}".format(args))

    #https://raspberrypi.stackexchange.com/questions/143643/how-can-i-use-dtoverlay-pwm/143644#143644
    #https://www.kernel.org/doc/html/v5.10/driver-api/pwm.html#using-pwms-with-the-sysfs-interface
    if (0 == args.peripheral):
        cmakeCommand = ['dtoverlay',
                        'pwm']
        subprocess.run(cmakeCommand)

        pwmOverlayExport = Path('/sys/class/pwm/pwmchip0/export')
        pwmOverlayPeriod = Path('/sys/class/pwm/pwmchip0/pwm0/period')
        pwmOverlayDuty = Path('/sys/class/pwm/pwmchip0/pwm0/duty_cycle')
        pwmOverlayEnable = Path('/sys/class/pwm/pwmchip0/pwm0/enable')

        #None of the below files will be present until we export.
        with pwmOverlayExport.open('w') as export:
            export.write('0')

        with pwmOverlayPeriod.open('w') as setPeriod:
            setPeriod.write(str(args.period))

        with pwmOverlayDuty.open('w') as setDuty:
            setDuty.write(str(args.duty))

        with pwmOverlayEnable.open('w') as setEnable:
            setEnable.write('1')

    elif (1 == args.peripheral):
        cmakeCommand = ['dtoverlay',
                        'pwm1']
        subprocess.run(cmakeCommand)

        pwmOverlayPeriod = Path('/sys/class/pwm/pwm1/period')
        pwmOverlayDuty = Path('/sys/class/pwm/pwm1/duty_cycle')
        pwmOverlayEnable = Path('/sys/class/pwm/pwm1/enable')

        with pwmOverlayPeriod.open('w') as setPeriod:
            setPeriod.write(str(args.period))

        with pwmOverlayDuty.open('w') as setDuty:
            setDuty.write(str(args.duty))

        with pwmOverlayEnable.open('w') as setEnable:
            setEnable.write('1')
    else:
        print("Invalid peripheral number.")
        exit(1)