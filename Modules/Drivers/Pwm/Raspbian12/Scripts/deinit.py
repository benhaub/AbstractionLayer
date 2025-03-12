################################################################################
#Date: March 12th, 2025                                                        #
#File: deinit.py                                                                 #
#Authour: Ben Haubrich                                                         #
#Synopsis: Deinitialize the PWM on Raspbian 12                                   #
################################################################################
import argparse
import subprocess
from pathlib import Path

def numberOfActiveOverlays():
    output = subprocess.check_output(['dtoverlay', '-l']).decode('utf-8').strip()
    return len(output.split('\n'))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog="DeinitPwm",
                                     description="Deinitialize the PWM",
                                     epilog="Created by Ben Haubrich March 12th, 2025")

    #By not providing a default value these are made required.
    parser.add_argument('peripheral', type=int, choices=[0, 1],
                            help="PWM peripheral number."
                       )

    args = parser.parse_args()

    #Uncomment for help with debugging.
    #print("{}".format(args))

    #https://raspberrypi.stackexchange.com/questions/143643/how-can-i-use-dtoverlay-pwm/143644#143644
    #https://www.kernel.org/doc/html/v5.10/driver-api/pwm.html#using-pwms-with-the-sysfs-interface
    if (0 == args.peripheral):
        cmakeCommand = ['dtoverlay',
                        '-r',
                        '0']
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