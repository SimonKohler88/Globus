import RPi.GPIO as GPIO


def main():

    GPIO.setmode(GPIO.BCM)
    GPIO.setup(22, GPIO.OUT)
    GPIO.output(22, GPIO.LOW)


if __name__ == "__main__":
        main()
