// Incomplete implementation of an audio mixer. Search for "REVISIT" to find things
// which are left as incomplete.
// Note: Generates low latency audio on BeagleBone Black; higher latency found on host.
#include "audioMixer_template.h"
#include <alsa/asoundlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <alloca.h> // needed for mixer
#include <time.h>



static snd_pcm_t *handle;

#define DEFAULT_VOLUME 80
#define DEFAULT_BPM 60

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 1
#define SAMPLE_SIZE (sizeof(short)) 			// bytes per sample
// Sample size note: This works for mono files because each sample ("frame') is 1 value.
// If using stereo files then a frame would be two samples.

static unsigned long playbackBufferSize = 0;
static short *playbackBuffer = NULL;

static int mode = 1;

// Currently active (waiting to be played) sound bites
#define MAX_SOUND_BITES 30
typedef struct {
	// A pointer to a previously allocated sound bite (wavedata_t struct).
	// Note that many different sound-bite slots could share the same pointer
	// (overlapping cymbal crashes, for example)
	wavedata_t *pSound;

	// The offset into the pData of pSound. Indicates how much of the
	// sound has already been played (and hence where to start playing next).
	int location;
} playbackSound_t;
static playbackSound_t soundBites[MAX_SOUND_BITES];

// Playback threading
void* playbackThread(void* arg);
static _Bool stopping = false;
static pthread_t playbackThreadId;
static pthread_mutex_t audioMutex = PTHREAD_MUTEX_INITIALIZER;

static int volume = 0;
static double BPM = DEFAULT_BPM;

void AudioMixer_init(void)
{
	AudioMixer_setVolume(DEFAULT_VOLUME);

	// Initialize the currently active sound-bites being played
	// REVISIT:- Implement this. Hint: set the pSound pointer to NULL for each
	//     sound bite.

	for (int i =0; i<MAX_SOUND_BITES; i++) {
		soundBites[i].pSound = NULL;
		soundBites[i].location = 0; 
	}


	// Open the PCM output
	int err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	// Configure parameters of PCM output
	err = snd_pcm_set_params(handle,
			SND_PCM_FORMAT_S16_LE,
			SND_PCM_ACCESS_RW_INTERLEAVED,
			NUM_CHANNELS,
			SAMPLE_RATE,
			1,			// Allow software resampling
			50000);		// 0.05 seconds per buffer
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	// Allocate this software's playback buffer to be the same size as the
	// the hardware's playback buffers for efficient data transfers.
	// ..get info on the hardware buffers:
 	unsigned long unusedBufferSize = 0;
	snd_pcm_get_params(handle, &unusedBufferSize, &playbackBufferSize);
	// ..allocate playback buffer:
	playbackBuffer = malloc(playbackBufferSize * sizeof(*playbackBuffer));
	// Launch playback thread:
	pthread_create(&playbackThreadId, NULL, playbackThread, NULL);
}


// Client code must call AudioMixer_freeWaveFileData to free dynamically allocated data.
void AudioMixer_readWaveFileIntoMemory(char *fileName, wavedata_t *pSound)
{
	assert(pSound);

	// The PCM data in a wave file starts after the header:
	const int PCM_DATA_OFFSET = 44;

	// Open the wave file
	FILE *file = fopen(fileName, "r");
	if (file == NULL) {
		fprintf(stderr, "ERROR: Unable to open file %s.\n", fileName);
		exit(EXIT_FAILURE);
	}

	// Get file size
	fseek(file, 0, SEEK_END);
	int sizeInBytes = ftell(file) - PCM_DATA_OFFSET;
	pSound->numSamples = sizeInBytes / SAMPLE_SIZE;

	// Search to the start of the data in the file
	fseek(file, PCM_DATA_OFFSET, SEEK_SET);

	// Allocate space to hold all PCM data
	pSound->pData = malloc(sizeInBytes);
	if (pSound->pData == 0) {
		fprintf(stderr, "ERROR: Unable to allocate %d bytes for file %s.\n",
				sizeInBytes, fileName);
		exit(EXIT_FAILURE);
	}

	// Read PCM data from wave file into memory
	int samplesRead = fread(pSound->pData, SAMPLE_SIZE, pSound->numSamples, file);
	if (samplesRead != pSound->numSamples) {
		fprintf(stderr, "ERROR: Unable to read %d samples from file %s (read %d).\n",
				pSound->numSamples, fileName, samplesRead);
		exit(EXIT_FAILURE);
	}
}

void AudioMixer_freeWaveFileData(wavedata_t *pSound)
{
	pSound->numSamples = 0;
	free(pSound->pData);
	pSound->pData = NULL;
}

void AudioMixer_queueSound(wavedata_t *pSound)
{
	// Ensure we are only being asked to play "good" sounds:
	assert(pSound->numSamples > 0);
	assert(pSound->pData);

	// Insert the sound by searching for an empty sound bite spot
	/*
	 * REVISIT: Implement this:
	 * 1. Since this may be called by other threads, and there is a thread
	 *    processing the soundBites[] array, we must ensure access is threadsafe.
	 * 2. Search through the soundBites[] array looking for a free slot.
	 * 3. If a free slot is found, place the new sound file into that slot.
	 *    Note: You are only copying a pointer, not the entire data of the wave file!
	 * 4. After searching through all slots, if no free slot is found then print
	 *    an error message to the console (and likely just return vs asserting/exiting
	 *    because the application most likely doesn't want to crash just for
	 *    not being able to play another wave file.
	 */


	//Find empty sound bite spot:

	int empty = 0;
	pthread_mutex_lock(&audioMutex);
	while (soundBites[empty].pSound!=NULL && empty< MAX_SOUND_BITES) {
		empty++;
	}
	if (empty == MAX_SOUND_BITES) {
		perror("ERROR: Sounds bites all occupied!\n");
		pthread_mutex_unlock(&audioMutex);
		return;
	}
	soundBites[empty].pSound = pSound;
	pthread_mutex_unlock(&audioMutex);
	printf("Soundfile inserted into soundbites at %d\n", empty);
	return;

}

void AudioMixer_cleanup(void)
{
	printf("Stopping audio...\n");

	// Stop the PCM generation thread
	stopping = true;
	pthread_join(playbackThreadId, NULL);

	// Shutdown the PCM output, allowing any pending sound to play out (drain)
	snd_pcm_drain(handle);
	snd_pcm_close(handle);

	// Free playback buffer
	// (note that any wave files read into wavedata_t records must be freed
	//  in addition to this by calling AudioMixer_freeWaveFileData() on that struct.)
	free(playbackBuffer);
	playbackBuffer = NULL;

	printf("Done stopping audio...\n");
	fflush(stdout);
}


int AudioMixer_getVolume()
{
	// Return the cached volume; good enough unless someone is changing
	// the volume through other means and the cached value is out of date.
	return volume;
}

// Function copied from:
// http://stackoverflow.com/questions/6787318/set-alsa-master-volume-from-c-code
// Written by user "trenki".
void AudioMixer_setVolume(int newVolume)
{
	// Ensure volume is reasonable; If so, cache it for later getVolume() calls.
	if (newVolume < 0 || newVolume > AUDIOMIXER_MAX_VOLUME) {
		printf("ERROR: Volume must be between 0 and 100.\n");
		return;
	}
	volume = newVolume;

    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "PCM";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

    snd_mixer_close(handle);
}


// Fill the playbackBuffer array with new PCM values to output.
//    playbackBuffer: buffer to fill with new PCM data from sound bites.
//    size: the number of values to store into playbackBuffer
static void fillPlaybackBuffer(short *playbackBuffer, int size)
{
	/*
	 * REVISIT: Implement this
	 * 1. Wipe the playbackBuffer to all 0's to clear any previous PCM data.
	 *    Hint: use memset()
	 * 2. Since this is called from a background thread, and soundBites[] array
	 *    may be used by any other thread, must synchronize this.
	 * 3. Loop through each slot in soundBites[], which are sounds that are either
	 *    waiting to be played, or partially already played:
	 *    - If the sound bite slot is unused, do nothing for this slot.
	 *    - Otherwise "add" this sound bite's data to the play-back buffer
	 *      (other sound bites needing to be played back will also add to the same data).
	 *      * Record that this portion of the sound bite has been played back by incrementing
	 *        the location inside the data where play-back currently is.
	 *      * If you have now played back the entire sample, free the slot in the
	 *        soundBites[] array.
	 *
	 * Notes on "adding" PCM samples:
	 * - PCM is stored as signed shorts (between SHRT_MIN and SHRT_MAX).
	 * - When adding values, ensure there is not an overflow. Any values which would
	 *   greater than SHRT_MAX should be clipped to SHRT_MAX; likewise for underflow.
	 * - Don't overflow any arrays!
	 * - Efficiency matters here! The compiler may do quite a bit for you, but it doesn't
	 *   hurt to keep it in mind. Here are some tips for efficiency and readability:
	 *   * If, for each pass of the loop which "adds" you need to change a value inside
	 *     a struct inside an array, it may be faster to first load the value into a local
	 *      variable, increment this variable as needed throughout the loop, and then write it
	 *     back into the struct inside the array after. For example:
	 *           int offset = myArray[someIdx].value;
	 *           for (int i =...; i < ...; i++) {
	 *               offset ++;
	 *           }
	 *           myArray[someIdx].value = offset;
	 *   * If you need a value in a number of places, try loading it into a local variable
	 *          int someNum = myArray[someIdx].value;
	 *          if (someNum < X || someNum > Y || someNum != Z) {
	 *              someNum = 42;
	 *          }
	 *          ... use someNum vs myArray[someIdx].value;
	 *
	 */

	memset(playbackBuffer, 0, size * sizeof(*playbackBuffer) );

	//find all the sound bytes that is not null
	playbackSound_t* temp = NULL;
	int playbackCount = 0;
	int audio_len = 0;
	int audio_loc = 0;
	int val = 0;
	pthread_mutex_lock(&audioMutex);
	{
		//given that each sample is sizeof(short)
		for (int i =0 ; i<MAX_SOUND_BITES; i++) {
			if (soundBites[i].pSound != NULL) {
				//add the PCM values into a playback buffer!
				temp = &soundBites[i];
				playbackCount = 0;
				audio_len = temp->pSound->numSamples;
				audio_loc= temp->location;
				for (; audio_loc < audio_len && playbackCount < playbackBufferSize; audio_loc++) {
					val = temp->pSound->pData[audio_loc] + playbackBuffer[playbackCount];
					if (val > SHRT_MAX)
						val = SHRT_MAX;
					if ( val < SHRT_MIN)
						val = SHRT_MIN;
					playbackBuffer[playbackCount] =  (short) val;
					playbackCount++;
				}
				//printf("pos = %d   audio loc = %d, audio len = %d\n", i, audio_loc, audio_len);
				
				soundBites[i].location = audio_loc;
				//check if soundByte has finished playing, free the soundByte and set it to null!
				if (audio_loc >= audio_len) {
					soundBites[i].pSound = NULL;
					soundBites[i].location = 0;
					printf("freeing the soundbyte!\n");
				}
			}
		}
	}
	pthread_mutex_unlock(&audioMutex);
}


void* playbackThread(void* arg)
{

	while (!stopping) {
		// Generate next block of audio
		fillPlaybackBuffer(playbackBuffer, playbackBufferSize);
		// Output the audio
		// for (int i =0; i< playbackBufferSize; i++) {
		// 	if (playbackBuffer[i]!=0) {
		// 		printf("val = %hi\n", playbackBuffer[i]);
		// 	}
		// }
		snd_pcm_sframes_t frames = snd_pcm_writei(handle,
				playbackBuffer, playbackBufferSize);
		// Check for (and handle) possible error conditions on output
		if (frames < 0) {
			fprintf(stderr, "AudioMixer: writei() returned %li\n", frames);
			frames = snd_pcm_recover(handle, frames, 1);
		}
		if (frames < 0) {
			fprintf(stderr, "ERROR: Failed writing audio with snd_pcm_writei(): %li\n",
					frames);
			exit(EXIT_FAILURE);
		}
		if (frames > 0 && frames < playbackBufferSize) {
			printf("Short write (expected %li, wrote %li)\n",
					playbackBufferSize, frames);
		}
	}

	return NULL;
}

void AudioMixer_standardRockBeat(wavedata_t* Base, wavedata_t* Hi_hat, wavedata_t* Snare) { //THere will be a thread that plays this beat
	printf("ROCK!!\n");
	long msec = (long)( 60/BPM/2 * 1000);
	struct timespec delay;
	delay.tv_sec = msec / 1000;
	delay.tv_nsec = (msec % 1000) * 1000000;
	printf("ms = %ld, BPM = %lf\n", msec, BPM);
	int count = 1;
	int it = 0;
	while (it<1) {	
		if (count==1) {
			AudioMixer_queueSound(Hi_hat);
			AudioMixer_queueSound(Base);
		}
		else if (count==2) {
			AudioMixer_queueSound(Hi_hat);
		}
		else if (count==3) {
			AudioMixer_queueSound(Hi_hat);
			AudioMixer_queueSound(Snare);
		}
		else if (count==4) {
			AudioMixer_queueSound(Hi_hat);
			count = 0;
			it++;
		}
		printf("test\n");	
		nanosleep(&delay, (struct timespec*)NULL);  //todo: this needs to be fixed
		count++;
	}
}



void AudioMixer_noBeat(void){
    return;
}

void AudioMixer_customBeat(wavedata_t* Base, wavedata_t* Hi_hat, wavedata_t* Snare){
    //Make a beat
	printf("Custom!!\n");
    long msec = (long)( 60/BPM/2 * 1000);
	struct timespec delay;
	delay.tv_sec = msec / 1000;
	delay.tv_nsec = (msec % 1000) * 1000000;
    AudioMixer_queueSound(Snare);
    AudioMixer_queueSound(Snare);
    // AudioMixer_queueSound(Hi_hat);
    nanosleep(&delay, (struct timespec *)NULL);

	AudioMixer_queueSound(Snare);
    AudioMixer_queueSound(Snare);
    // AudioMixer_queueSound(Hi_hat);
    nanosleep(&delay, (struct timespec *)NULL);

    AudioMixer_queueSound(Hi_hat);
    nanosleep(&delay, (struct timespec *)NULL);

    AudioMixer_queueSound(Base);
    nanosleep(&delay, (struct timespec *)NULL);

    AudioMixer_queueSound(Hi_hat);
    nanosleep(&delay, (struct timespec *)NULL);

    AudioMixer_queueSound(Base);
    nanosleep(&delay, (struct timespec *)NULL);


    AudioMixer_queueSound(Hi_hat);
    nanosleep(&delay, (struct timespec *)NULL);

    AudioMixer_queueSound(Base);
    nanosleep(&delay, (struct timespec *)NULL);

    AudioMixer_queueSound(Snare);
    nanosleep(&delay, (struct timespec *)NULL);
}

double AudioMixer_getBPM(void) {
	return BPM;
}

void AudioMixer_setBPM(double num) {
	BPM =  num;
	if (BPM > AUDIOMIXER_MAX_BPM) {
		BPM = AUDIOMIXER_MAX_BPM;
	}
	else if (BPM < AUDIOMIXER_MIN_BPM) {
		BPM = AUDIOMIXER_MIN_BPM;
	}
}

void AudioMixer_next(void){
	mode++;
	if(mode > 2){
		mode = 0;
	}
}

void AudioMixer_prev(void){
	mode--;
	if(mode<0){
		mode =2;
	}
}

int AudioMixer_getMode(void){
	return mode;
}















