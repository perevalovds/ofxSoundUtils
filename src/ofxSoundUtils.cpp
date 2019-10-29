#include "ofxSoundUtils.h"

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
