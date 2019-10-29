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
	
};

