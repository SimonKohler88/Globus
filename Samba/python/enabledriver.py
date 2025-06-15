import RPi.GPIO as GPIO
import lgpio

def main():

    #h = lgpio.gpiochip_open(0)
    #lgpio.gpio_claim_output(h, 22)
    #lgpio.gpio_write(h, 22, 1)

    GPIO.setmode(GPIO.BCM)
    GPIO.setup(22, GPIO.OUT)
    GPIO.output(22, GPIO.HIGH)


if __name__ == "__main__":
        main()
