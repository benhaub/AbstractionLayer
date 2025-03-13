################################################################################
#Date: March 12th, 2025                                                        #
#File: common.py                                                               #
#Authour: Ben Haubrich                                                         #
#Synopsis: Common functions for Raspbian 12 PWM driver scripts                 #
################################################################################
import argparse
import subprocess
from pathlib import Path

def numberOfActiveOverlays():
    output = subprocess.check_output(['dtoverlay', '-l']).decode('utf-8').strip()
    #Minus 1 because the first line is informational and does not document an active overlay.
    return len(output.split('\n')) - 1

def allChannelsUnexported():
    #Set Shell=True so that the shell can interpret the pattern matching expression.
    cmakeCommand = ['ls',
                    '/sys/class/pwm/pwmchip0/pwm[0-9]', Shell=True]
    p = subprocess.run(cmakeCommand)
    return p.returncode == 0
