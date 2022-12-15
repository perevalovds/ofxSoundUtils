#pragma once

//Sound utils
//- Conversions notes to Hz, Db to amp, mu law encoding
//- Load/save raw PCM
//- Low,high,band-pass filters
//- Delay effect

#include "ofMain.h"

struct ofxSoundUtils {
	// Note - Hz conversion
	//Convert midi note to frequency, note 0..127
	static int note_to_hz_int(int midi_note);
	static float note_to_hz_float(float midi_note);

	static int hz_to_note(float hz);

	// Db - amp conversions
	// db <= 0, for example -40..0
	static float db_to_amp(float db);

	// Time - samples conversion
	static int ms_to_samples(float ms, int sample_rate);
	static float sample_rate_to_phase_shift(int sample_rate);

	static vector<float> load_sound_raw_mono16(string file_name);
	static void save_sound_raw_mono16(vector<float> &sound, string file_name);
	static void save_sound_raw_stereo16_split(vector<float> &sound_stereo, string file_nameL, string file_nameR);
	//to save WAVs see my fork of ofxAudioFile: https://github.com/perevalovds/ofxAudioFile.git

	// Compute momentary velocity
	//x - amplitude, y - velocity of amplitude
	static ofPoint get_sound_amp_vel(vector<float> &sound, int i);
	static ofPoint get_sound_amp_vel(float *sound, int i, int n);

	// Interpolation
	static float interpolate_stereo(vector<float> &sound_stereo, float pos, int channel);

	// mu_law 
	// Coding (log extension), based on formula in an article about WaveNet
	static vector<float> mu_law(vector<float> &sound_mono, float mu = 255);
	static vector<unsigned char> mu_law8(vector<float> &sound_mono, float mu = 255);
	// Decoding 
	static vector<float> mu_law_decode(vector<float> &sound_mu_mono, float mu = 255);


	// Files (from ofxKuFiles)
	static bool file_exists(string fileName, bool use_data_path = true);
	static int file_size(string fileName, bool use_data_path = true);
	
	// RMS
	static float get_RMS(vector<float> &sound);

	//if RMS exceeds value, limit it
	static void limit_RMS(vector<float> &sound, float max_rms);
};


//Filter: lo-pass, hi-pass, band-pass
//works sample-by-sample
// By Paul Kellett
// http://www.musicdsp.org/showone.php?id=29
// http://www.martin-finke.de/blog/articles/audio-plugins-013-filter/
// Lowpass-resonance filter from LowPassFilter.h, created by Marek Bereza on 11/08/2013.


struct ofxSoundUtilsFilter {

	static const int FILTER_MODE_BYPASS = 0;
	static const int FILTER_MODE_LOWPASS = 1;
	static const int FILTER_MODE_BANDPASS = 2;
	static const int FILTER_MODE_HIGHPASS = 3;
	static const int FILTER_MODE_LOWPASS_RESO = 4;

	void process_resetted(vector<float> &sound, float cutoff, int mode) {
		buf0 = 0;
		buf1 = 0;

		x = y = r = c = 0;
		set_params_lopass_reso(cutoff, resonance);


		for (int i = 0; i < sound.size(); i++) {
			sound[i] = process(sound[i], cutoff, mode);
		}
	}

	void set_params_lopass_reso(float cut_hz, float reso, float sample_rat = 44100) {
		cutoff = cut_hz;
		resonance = reso;
		sample_rate = sample_rat;

		if (cutoff < 50) cutoff = 50;
		if (cutoff > sample_rate/2) cutoff = sample_rate/2;
		float z = cos(TWO_PI / sample_rate * cutoff);
		c = 2 - 2 * z;
		float zzz = z - 1;
		zzz = zzz * zzz * zzz;
		r = (sqrt(2) * sqrt(-zzz) + resonance * (z - 1)) / (resonance * (z - 1));
	}

	//cutoff used except FILTER_MODE_LOWPASS_RESO
	float process(float inputValue, float cutoff, int mode) {
		switch (mode) {
		case FILTER_MODE_BYPASS:
			buf0 += cutoff * (inputValue - buf0);
			buf1 += cutoff * (buf0 - buf1);
			return inputValue;
		case FILTER_MODE_LOWPASS:
			buf0 += cutoff * (inputValue - buf0);
			buf1 += cutoff * (buf0 - buf1);
			return buf1;
		case FILTER_MODE_HIGHPASS:
			buf0 += cutoff * (inputValue - buf0);
			buf1 += cutoff * (buf0 - buf1);
			return inputValue - buf0;
		case FILTER_MODE_BANDPASS:
			buf0 += cutoff * (inputValue - buf0);
			buf1 += cutoff * (buf0 - buf1);
			return buf0 - buf1;
		case FILTER_MODE_LOWPASS_RESO:
		{
			x += (inputValue - y) * c;
			y += x;
			x *= r;
			return x;
		}
		default:
			return 0.0;
		}
	}


	float buf0 = 0;
	float buf1 = 0;

	float cutoff = 0;
	float resonance = 0;
	float sample_rate = 44100;
	float x = 0;
	float y = 0;
	float r = 0;
	float c = 0;

};


//Delay effect
struct ofxSoundUtilsDelayStereo {
	void setup(int sample_rate, float buffer_length_sec);
	void set_volume(float vol);
	void set_delay_time(float time_sec);
	void set_feedback(float feedback);
	void set_cross_stereo(float cross);
	void process_add(float inputL, float inputR, float &outputL, float &outputR);
protected:
	int sample_rate_ = 44100;
	int n_ = 0;
	float vol_ = 0.1;
	int time_delay_ = 100;
	float feedback_ = 0.05;
	float cross_ = 0;

	vector<float> bufferL_, bufferR_;
	int pos_ = 0;
};

