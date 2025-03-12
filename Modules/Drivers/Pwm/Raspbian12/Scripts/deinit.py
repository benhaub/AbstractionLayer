################################################################################
#Date: March 12th, 2025                                                        #
#File: deinit.py                                                               #
#Authour: Ben Haubrich                                                         #
#Synopsis: Deinitialize the PWM on Raspbian 12                                 #
################################################################################
import argparse
import subprocess
from pathlib import Path

def numberOfActiveOverlays():
    output = subprocess.check_output(['dtoverlay', '-l']).decode('utf-8').strip()
    #Minus 1 because the first line is informational and does not document an active overlay.
    return len(output.split('\n')) - 1


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

    pwmOverlay = 'pwm' if 0 == args.peripheral else 'pwm' + str(args.peripheral)

    #https://raspberrypi.stackexchange.com/questions/143643/how-can-i-use-dtoverlay-pwm/143644#143644
    #https://www.kernel.org/doc/html/v5.10/driver-api/pwm.html#using-pwms-with-the-sysfs-interface
    pwmOverlayUnexport = Path('/sys/class/pwm/pwmchip0/unexport')
    pwmOverlayEnable = Path('/sys/class/pwm/pwmchip0/' + 'pwm' + str(args.peripheral) + '/enable')

    try:
        with pwmOverlayEnable.open('w') as enable:
            enable.write('0')

        with pwmOverlayUnexport.open('w') as unexport:
            unexport.write(str(args.peripheral))
    except FileNotFoundError:
        pass

    cmakeCommand = ['dtoverlay',
                    '-r',
                    pwmOverlay]
    subprocess.run(cmakeCommand)

    exit(0)
