#pragma once

//Sound utils

#include "ofMain.h"

struct ofxSoundUtils {
	static vector<float> load_sound_raw_mono16(string file_name);
	static void save_sound_raw_mono16(vector<float> &sound, string file_name);
	static void save_sound_raw_stereo16_split(vector<float> &sound_stereo, string file_nameL, string file_nameR);

	//x - amplitude, y - velocity of amplitude
	static ofPoint get_sound_amp_vel(vector<float> &sound, int i);
	static ofPoint get_sound_amp_vel(float *sound, int i, int n);

	//interpolated
	static float interpolate_stereo(vector<float> &sound_stereo, float pos, int channel);

	//mu_law coding (log extension), based on formula in an article about WaveNet
	static vector<float> mu_law(vector<float> &sound_mono, float mu = 255);
	static vector<unsigned char> mu_law8(vector<float> &sound_mono, float mu = 255);
	//mu_law decoding 
	static vector<float> mu_law_decode(vector<float> &sound_mu_mono, float mu = 255);


	//from ofxKuFiles:
	static bool file_exists(string fileName, bool use_data_path = true);
	static int file_size(string fileName, bool use_data_path = true);
	
	//getting RMS
	static float get_RMS(vector<float> &sound);

	//if RMS exceeds value, limit it
	static void limit_RMS(vector<float> &sound, float max_rms);

	//linear to exponential volume, 0..1
	//https://www.dr-lex.be/info-stuff/volumecontrols.html
	static float volume_linear_to_exp(float v);
};


//Filter: lo-pass, hi-pass, band-pass
//works sample-by-sample
// By Paul Kellett
// http://www.musicdsp.org/showone.php?id=29
// http://www.martin-finke.de/blog/articles/audio-plugins-013-filter/

struct ofxSoundUtilsFilter {

	static const int FILTER_MODE_BYPASS = 0;
	static const int FILTER_MODE_LOWPASS = 1;
	static const int FILTER_MODE_BANDPASS = 2;
	static const int FILTER_MODE_HIGHPASS = 3;

	void process_resetted(vector<float> &sound, float cutoff, int mode) {
		buf0 = 0;
		buf1 = 0;
		for (int i = 0; i < sound.size(); i++) {
			sound[i] = process(sound[i], cutoff, mode);
		}
	}

	float process(float inputValue, float cutoff, int mode) {
		buf0 += cutoff * (inputValue - buf0);
		buf1 += cutoff * (buf0 - buf1);
		switch (mode) {
		case FILTER_MODE_BYPASS:
			return inputValue;
		case FILTER_MODE_LOWPASS:
			return buf1;
		case FILTER_MODE_HIGHPASS:
			return inputValue - buf0;
		case FILTER_MODE_BANDPASS:
			return buf0 - buf1;
		default:
			return 0.0;
		}
	}

	float buf0 = 0;
	float buf1 = 0;

};

