################################################################################
#Date: March 11th, 2025                                                        #
#File: init.py                                                                 #
#Authour: Ben Haubrich                                                         #
#Synopsis: Initialize the PWM on Raspbian 12                                   #
################################################################################
#pip imports
import argparse
import subprocess
from pathlib import Path
#Local imports
from common import allChannelsUnexported

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog="InitPwm",
                                     description="Initialize the PWM",
                                     epilog="Created by Ben Haubrich March 11th, 2025")

    parser.add_argument('channel', type=int, choices=[0, 1],
                            help="PWM channel number."
                       )
    parser.add_argument('outputPin', type=int, choices=[12, 18, 13, 19],
                            help="PWM output pin number."
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

    function = ''
    if (12 == args.outputPin):
        function = 4
    elif (18 == args.outputPin):
        function = 2
    elif (13 == args.outputPin):
        function = 4
    elif (19 == args.outputPin):
        function = 2

    #https://raspberrypi.stackexchange.com/questions/143643/how-can-i-use-dtoverlay-pwm/143644#143644
    #https://www.kernel.org/doc/html/v5.10/driver-api/pwm.html#using-pwms-with-the-sysfs-interface
    dtOverlayCommand = ['dtoverlay',
                        'pwm-2chan' + ',' + 'pin=' + str(args.outputPin),
                        'func=' + str(function)]
    subprocess.run(dtOverlayCommand)

    pwmOverlayExport = Path('/sys/class/pwm/pwmchip0/export')
    pwmOverlayPeriod = Path('/sys/class/pwm/pwmchip0/' + 'pwm' + str(args.channel) + '/period')
    pwmOverlayDuty = Path('/sys/class/pwm/pwmchip0/' + 'pwm' + str(args.channel) + '/duty_cycle')
    pwmOverlayEnable = Path('/sys/class/pwm/pwmchip0/' + 'pwm' + str(args.channel) + '/enable')

    #None of the below files will be present until we export.
    with pwmOverlayExport.open('w') as export:
        export.write(str(args.channel))

    with pwmOverlayPeriod.open('w') as setPeriod:
        setPeriod.write(str(args.period))

    with pwmOverlayDuty.open('w') as setDuty:
        dutyCycle = ((args.duty) / 100) * args.period
        setDuty.write(str(int(dutyCycle)))

    with pwmOverlayEnable.open('w') as enable:
        enable.write('1')

    exit(0)
