#include "ofxSoundUtils.h"

//--------------------------------------------------------------------------------
int ofxSoundUtils::ms_to_samples(float ms, int sample_rate)
{
	return int(ms * sample_rate);
}

//--------------------------------------------------------------------------------
float ofxSoundUtils::sample_rate_to_phase_shift(int sample_rate)
{
	if (sample_rate <= 0) return 0;
	return 1.0f / sample_rate;
}

//--------------------------------------------------------------------------------
bool ofxSoundUtils::file_exists(string fileName, bool use_data_path)
{
	if (use_data_path) fileName = ofToDataPath(fileName);
	ifstream inp;
	inp.open(fileName.c_str(), ifstream::in);
	inp.close();
	return !inp.fail();
}

//--------------------------------------------------------------------------------
int ofxSoundUtils::file_size(string fileName, bool use_data_path)
{
	if (!file_exists(fileName, use_data_path)) {
		return 0;
	}
	if (use_data_path) fileName = ofToDataPath(fileName);
	FILE *file = fopen(fileName.c_str(), "rb");
	fseek(file, 0, SEEK_END);
	int size = ftell(file);	//TODO use size_t for big files
	fclose(file);
	return size;
}

//--------------------------------------------------------------------------------

vector<float> ofxSoundUtils::load_sound_raw_mono16(string file_name) {
	vector<float> sound;
	cout << "Loading " << file_name << endl;
	if (!file_exists(file_name)) {
		cout << "No file " << file_name << endl;
		return sound;
	}
	int n = file_size(file_name) / sizeof(int16_t);
	if (n == 0) {
		cout << "Empty file " << file_name << endl;
		return sound;
	}
	FILE *file = fopen(ofToDataPath(file_name).c_str(), "rb");
	if (file) {
		vector<int16_t> data(n);
		fread(&data[0], sizeof(int16_t), n, file);
		sound.resize(n);
		for (int i = 0; i < n; i++) {
			sound[i] = ofClamp(data[i] / 32768.0f, -1, 1);
		}

		fclose(file);
		return sound;
	}
	else {
		cout << "Error reading file " << file_name << endl;
		return sound;
	}

}

//--------------------------------------------------------------------------------
void ofxSoundUtils::save_sound_raw_mono16(vector<float> &sound, string file_name) {
	cout << "Saving " << file_name << endl;
	typedef int16_t type;
	int n = sound.size();

	vector<type> data(n);
	for (int i = 0; i < n; i++) {
		data[i] = int(sound[i] * 32767);
	}

	FILE *file = fopen(ofToDataPath(file_name).c_str(), "wb");
	if (file) {
		fwrite(&data[0], sizeof(type), n, file);
		fclose(file);
		return;
	}
	else {
		cout << "Error writing file " << file_name << endl;
		return;
	}

}

//--------------------------------------------------------------------------------
void ofxSoundUtils::save_sound_raw_stereo16_split(vector<float> &sound_stereo, string file_nameL, string file_nameR) {
	cout << "Saving stareo raw " << file_nameL << " " << file_nameR << " " << endl;
	int n = sound_stereo.size()/2;

	vector<float> soundL(n);
	vector<float> soundR(n);
	for (int i = 0; i < n; i++) {
		soundL[i] = sound_stereo[i*2];
		soundR[i] = sound_stereo[i * 2 + 1];
	}
	save_sound_raw_mono16(soundL, file_nameL);
	save_sound_raw_mono16(soundR, file_nameR);
}

//--------------------------------------------------------------------------------
ofPoint ofxSoundUtils::get_sound_amp_vel(vector<float> &sound, int i) {
	if (i < 1 || i >= sound.size() - 1) {
		return ofPoint();
	}
	return ofPoint(sound[i], sound[i] - sound[i - 1]);
}

//--------------------------------------------------------------------------------
ofPoint ofxSoundUtils::get_sound_amp_vel(float *sound, int i, int n) {
	if (i < 1 || i >= n - 1) {
		return ofPoint();
	}
	return ofPoint(sound[i], sound[i] - sound[i - 1]);

}

//--------------------------------------------------------------------------------
float ofxSoundUtils::interpolate_stereo(vector<float> &sound_stereo, float pos, int channel) {
	float posq = max(pos, 0.0f);

	int n = int(sound_stereo.size())/2;
	int posi0 = min(int(posq), n - 1);
	int posi1 = min(posi0 + 1, n - 1);
	float t = posq - posi0;
	return ofLerp(sound_stereo[posi0 * 2 + channel], sound_stereo[posi1 * 2 + channel], t); 
}


//--------------------------------------------------------------------------------
//mu_law coding (log extension), based on formula in an article about WaveNet
//f(x) = sign(x)*ln(1+mu*|x|)/ln(1+mu), -1<x<1, mu=255 
vector<float> ofxSoundUtils::mu_law(vector<float> &sound_mono, float mu) {
	int n = sound_mono.size();
	vector<float> data(n);

	for (int i = 0; i < n; i++) {
		float x = ofClamp(sound_mono[i], -1, 1);
		data[i] = ofSign(x) * log(1 + mu * fabs(x)) / log(1 + mu);
		//cout << int(data[i]) << " ";
	}

	return data;
}

//--------------------------------------------------------------------------------
vector<float> ofxSoundUtils::mu_law_decode(vector<float> &sound_mu_mono, float mu) {
	//y = sign(x)*log(1+mu|x|)/log(1+mu),
	//sign(x) = sign(y),
	//|y| = log(1+mu|x|)/log(1+mu),
	//log(1+mu|x|) = |y|*log(1+mu),
	//1+mu|x| = exp(|y|*log(1+mu))
	//|x| = (exp(|y|*log(1+mu))-1) / mu
	int n = sound_mu_mono.size();
	vector<float> data(n);

	for (int i = 0; i < n; i++) {
		float y = ofClamp(sound_mu_mono[i], -1, 1);
		data[i] = ofSign(y) * (exp(fabs(y)*log(1 + mu)) - 1) / mu;
		//cout << int(data[i]) << " ";
	}

	return data;
}


//--------------------------------------------------------------------------------
vector<unsigned char> ofxSoundUtils::mu_law8(vector<float> &sound_mono, float mu) {
	int n = sound_mono.size();
	vector<unsigned char> data(n);

	for (int i = 0; i < n; i++) {
		float x = ofClamp(sound_mono[i], -1, 1);
		float f = ofSign(x) * log(1 + mu * fabs(x)) / log(1 + mu);
		data[i] = int(ofMap(f, -1, 1, 0, 255, true));
		//cout << int(data[i]) << " ";
	}

	return data;
}

//--------------------------------------------------------------------------------
//getting RMS
float ofxSoundUtils::get_RMS(vector<float> &sound) {
	double vol = 0.0;

	int num = 0;
	//lets go through each sample and calculate the root mean square which is a rough way to calculate volume	
	for (size_t i = 0; i < sound.size(); i++) {
		float v = sound[i];
		vol += v*v;
		num++;
	}

	if (num > 0) {
		vol /= num;
	}

	vol = sqrt(vol);
	return vol;
}


//--------------------------------------------------------------------------------
//if RMS exceeds value, limit it
void ofxSoundUtils::limit_RMS(vector<float> &sound, float max_rms) {
	float vol = get_RMS(sound);
	if (vol > max_rms) {
		float mul = (vol > 0) ? max_rms / vol : 0;
		for (int i = 0; i < sound.size(); i++) {
			sound[i] *= mul;
		}
	}
}

//--------------------------------------------------------------
// https://hydrogenaud.io/index.php?topic=86414.0
// db = 20*log10(amplitude)
//      6.02 * log2(Amplitude)
//amplitude = pow(2, dB/6.02), or pow(10, dB/20)
float ofxSoundUtils::db_to_amp(float db)
{
	return pow(10.0f, db / 20.0f);
}

//--------------------------------------------------------------
// https://newt.phys.unsw.edu.au/jw/notes.html
// 69 is A, 440Hz, m=0..127
// m = 12*log2(fm/440 Hz) + 69 and fm = 2^((m-69)/12) * (440 Hz).

//Convert midi note to frequency, note 0..127
int ofxSoundUtils::note_to_hz_int(int midi_note) {
	return note_to_hz_float(midi_note);
}

//float version
float ofxSoundUtils::note_to_hz_float(float midi_note) {
	return pow(2.f, (midi_note - 69.f) / 12.f) * 440.f; //Hz
	//note: for midi_note>100 sometimes occurs overflow
}

int ofxSoundUtils::hz_to_note(float hz) {
	return 12.f * log2f(hz / 440.f) + 69;
}

//--------------------------------------------------------------------------------
void ofxSoundUtilsDelayStereo::setup(int sample_rate, float buffer_length_sec) {
	sample_rate_ = sample_rate;
	n_ = int(buffer_length_sec * sample_rate_);

	time_delay_ = 1;
	feedback_ = 0.05;
	cross_ = 0;

	bufferL_.resize(n_);
	bufferR_.resize(n_);
	for (int i = 0; i < n_; i++) {
		bufferL_[i] = bufferR_[i] = 0;
	}
	pos_ = 0;

}

//--------------------------------------------------------------------------------
void ofxSoundUtilsDelayStereo::set_volume(float vol) {
	vol_ = vol;
}

//--------------------------------------------------------------------------------
void ofxSoundUtilsDelayStereo::set_delay_time(float time_sec) {
	time_delay_ = int(time_sec * sample_rate_);
}

//--------------------------------------------------------------------------------
void ofxSoundUtilsDelayStereo::set_feedback(float feedback) {
	feedback_ = feedback;
}

//--------------------------------------------------------------------------------
void ofxSoundUtilsDelayStereo::set_cross_stereo(float cross) {
	cross_ = cross;
}

//--------------------------------------------------------------------------------
void ofxSoundUtilsDelayStereo::process_add(float inputL, float inputR, float &outputL, float &outputR) {
	pos_ %= n_;
	int wr = (pos_ + time_delay_) % n_;
	float &L = bufferL_[pos_];
	float &R = bufferR_[pos_];
	outputL += L * vol_; 
	outputR += R * vol_; 

	bufferL_[wr] = inputL + feedback_ * (L * (1 - cross_) + R * cross_);
	bufferR_[wr] = inputR + feedback_ * (L * cross_ + R * (1 - cross_));

	pos_++;

}

//--------------------------------------------------------------------------------
