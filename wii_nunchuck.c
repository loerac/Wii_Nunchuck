#include <stdio.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>

#define BUTTON_Z(a)   (a & 0x01)
#define BUTTON_C(a)   ((a & 0x02) >> 1)
#define ACCEL_X(a, b) ((a << 2) | ((b & 0x0c) >> 2))
#define ACCEL_Y(a, b) ((a << 2) | ((b & 0x40) >> 4))
#define ACCEL_Z(a, b) ((a << 2) | ((b & 0xc0) >> 6))

void sigintHandler(int file, int sigNum) {
	if (file > 0)
		close(file);
	psignal(sigNum, "");
	exit(0);
}

int writeReg(int file) {
	char buf[1];
	buf[0] = 0x00;

	if (write(file, buf, 1) < 0) {
		close(file);
		return -1;
	}
}

int readReg(int file, char buf[]) {
	if (read(file, buf, 6) < 0)
		return -1;
	writeReg();
	return 0;
}

int main() {
	char buf[6] = {0, 0, 0, 0, 0, 0};
	int accelX, accelY, accelZ, buttonC, buttonZ, file;

	signal(SIGINT, sigintHandler);
	signal(SIGTERM, sigintHandler);

	if ((file = open("/dev/i2c-1", O_RDWR)) < 0) {
		printf("Failed to open the bus");
		return 1;
	}
	if (ioctl(file, I2C_SLAVE, 0x52) < 0) {
		printf("Failed to connect to the sensor");
		close(file);
		return 1;
	}

	while (1) {
		if (readReg(file, buf) < 0)
			printf("Failed to read in the full buffer");
			
		accelX = ACCEL_X(buf[2], buf[5]);
		accelY = ACCEL_X(buf[3], buf[5]);
		accelZ = ACCEL_X(buf[4], buf[5]);

		buttonZ = BUTTON_Z(buf[5]);
		buttonC = BUTTON_C(buf[5]);

		printf ("JX: %d JY: %d; AX: %d AY: %d AZ: %d; BZ: %s BC: %s\n", buf[0], buf[1], accelX, accelY, accelZ, (buttonZ ? "z" : "Z"), (buttonC ? "c" : "C"));
		//usleep(1000000);	//For my slow eyes
		usleep(100000);		//For fastpase
	} return 0;
}