#include "audioMixer_template.h"
#include "beat_mode.h"
#include "i2cHandler.h"
#include "joystick_control.h"

int main(){
	AudioMixer_init();

	pthread_t beatThread; 
	// pthread_t audioThread;
	pthread_t i2cHandlerThread;
	pthread_t joystickThread;

	printf("Creating Thread...\n");
	pthread_create(&beatThread, NULL, beatBoxThread, NULL);
	pthread_create(&joystickThread, NULL, joystickthread, NULL);
	pthread_create(&i2cHandlerThread, NULL, i2cHandlerInit, NULL);

	printf("Joining Thread...]\n");
	pthread_join(beatThread, NULL);
	pthread_join(joystickThread, NULL);
	pthread_join(i2cHandlerThread, NULL);

	printf("Cleaning up...\n");
	AudioMixer_cleanup();
	return 0;
}
