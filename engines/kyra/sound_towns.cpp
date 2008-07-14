/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */


#include "common/system.h"
#include "kyra/resource.h"
#include "kyra/sound.h"
#include "kyra/screen.h"

#include "sound/audiocd.h"
#include "sound/audiostream.h"

#include "common/util.h"

#define		EUPHONY_FADEOUT_TICKS		600

namespace Kyra {

enum EnvelopeState { s_ready, s_attacking, s_decaying, s_sustaining, s_releasing };

class Towns_EuphonyChannel : public MidiChannel {
public:
	Towns_EuphonyChannel() {}
	~Towns_EuphonyChannel() {}

	virtual void nextTick(int32 *outbuf, int buflen) = 0;
	virtual void rate(uint16 r) = 0;

protected:
	uint16 _rate;
};

class Towns_EuphonyFmChannel : public Towns_EuphonyChannel {
public:
	Towns_EuphonyFmChannel();
	virtual ~Towns_EuphonyFmChannel();

	void nextTick(int32 *outbuf, int buflen);
	void rate(uint16 r);

	// MidiChannel interface
	MidiDriver *device() { return 0; }
	byte getNumber() { return 0; }
	void release() { }
	void send(uint32 b) { }
	void noteOff(byte note);
	void noteOn(byte note, byte onVelo);
	void programChange(byte program) {}
	void pitchBend(int16 value);
	void controlChange(byte control, byte value);
	void pitchBendFactor(byte value) { }
	void sysEx_customInstrument(uint32 unused, const byte *instr);

protected:
	Voice2612 *_voice;
};

class Towns_EuphonyPcmChannel : public Towns_EuphonyChannel {
public:
	void nextTick(int32 *outbuf, int buflen);
	void rate(uint16 r);

	Towns_EuphonyPcmChannel();
	virtual ~Towns_EuphonyPcmChannel();

	// MidiChannel interface
	MidiDriver *device() { return 0; }
	byte getNumber() { return 0; }
	void release() { }
	void send(uint32 b) { }
	void noteOff(byte note);
	void noteOn(byte note, byte onVelo);
	void programChange(byte program) {}
	void pitchBend(int16 value);
	void controlChange(byte control, byte value);
	void pitchBendFactor(byte value) { }
	void sysEx_customInstrument(uint32 type, const byte *instr);

protected:
	void velocity(int velo);
	void panPosition(int8 pan);
	void evpNextTick();

	int _ctrl7_volume;
	int16 _velocity;
	int16 _note;
	int32 _frequencyOffs;
	float _phase;
	int8 _current;

	struct Voice {
		char name[9];
		uint16 split[8];
		uint32 id[8];
		struct Snd {
			char name[9];
			int32 id;
			int32 numSamples;
			int32 loopStart;
			int32 loopLength;
			int32 samplingRate;
			int32 keyOffset;
			int32 keyNote;
			const int8 *_samples;
		} *_snd[8];
		struct Env {
			EnvelopeState state;
			int32 currentLevel;
			int32 rate;
			int32 tickCount;
			int32 totalLevel;
			int32 attackRate;
			int32 decayRate;
			int32 sustainLevel;
			int32 sustainRate;
			int32 releaseLevel;
			int32 releaseRate;
			int32 rootKeyOffset;
			int32 size;
		} *_env[8];
	} *_voice;
};

class Towns_EuphonyTrackQueue {
public:
	Towns_EuphonyTrackQueue(Towns_EuphonyDriver *driver, Towns_EuphonyTrackQueue *last);
	~Towns_EuphonyTrackQueue() {}

	Towns_EuphonyTrackQueue *release();
	void initDriver();
	void loadDataToCurrentPosition(uint8 *trackdata, uint32 size, bool loop = 0);
	void loadDataToEndOfQueue(uint8 *trackdata, uint32 size, bool loop = 0);
	void setPlayBackStatus(bool playing);
	bool isPlaying() {return _playing; }
	uint8 *trackData() {return _trackData; }

	bool _loop;
	Towns_EuphonyTrackQueue *_next;

private:
	uint8 *_trackData;
	uint8 *_used;
	uint8 *_fchan;
	uint8 *_wchan;
	bool _playing;
	Towns_EuphonyDriver *_driver;
	Towns_EuphonyTrackQueue *_last;
};

class Towns_EuphonyParser : public MidiParser {
public:
	Towns_EuphonyParser(Towns_EuphonyTrackQueue * queue);
	bool loadMusic (byte *data, uint32 size);
	int32 calculateTempo(int16 val);

protected:
	void parseNextEvent (EventInfo &info);
	void resetTracking();
	void setup();

	byte *_enable;
	byte *_mode;
	byte *_channel;
	byte *_adjVelo;
	int8 *_adjNote;

	uint8 _firstBaseTickStep;
	uint8 _nextBaseTickStep;
	uint32 _initialTempo;
	uint32 _baseTick;

	byte _tempo[3];
	Towns_EuphonyTrackQueue *_queue;
};

class Towns_EuphonyDriver : public MidiDriver_Emulated {
public:
	Towns_EuphonyDriver(Audio::Mixer *mixer);
	virtual ~Towns_EuphonyDriver();

	int open();
	void close();
	void send(uint32 b);
	void send(byte channel, uint32 b);
	uint32 property(int prop, uint32 param) { return 0; }

	void setPitchBendRange(byte channel, uint range) { }
	void loadFmInstruments(const byte *instr);
	void loadWaveInstruments(const byte *instr);

	Towns_EuphonyTrackQueue *queue() { return _queue; }

	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }

	void assignFmChannel(uint8 midiChannelNumber, uint8 fmChannelNumber);
	void assignWaveChannel(uint8 midiChannelNumber, uint8 waveChannelNumber);
	void removeChannel(uint8 midiChannelNumber);

	void setVolume(int val = -1) { if (val >= 0) _volume = val; }
	int getVolume(int val = -1) { return _volume; }

	// AudioStream API
	bool isStereo() const { return true; }
	int getRate() const { return _mixer->getOutputRate(); }

	void fading(bool status = true);

protected:
	void nextTick(int16 *buf1, int buflen);
	void rate(uint16 r);

	void generateSamples(int16 *buf, int len);

	Towns_EuphonyFmChannel *_fChannel[6];
	Towns_EuphonyPcmChannel *_wChannel[8];
	Towns_EuphonyChannel *_channel[16];
	Towns_EuphonyTrackQueue *_queue;

	int _volume;
	bool _fading;
	int16 _fadestate;

	uint8 *_fmInstruments;
	uint8 *_waveInstruments;
	int8 * _waveSounds[10];
};

Towns_EuphonyFmChannel::Towns_EuphonyFmChannel() {
	_voice = new Voice2612;
}

Towns_EuphonyFmChannel::~Towns_EuphonyFmChannel() {
	delete _voice;
}

void Towns_EuphonyFmChannel::noteOn(byte note, byte onVelo) {
	_voice->noteOn(note, onVelo);
}

void Towns_EuphonyFmChannel::noteOff(byte note) {
	_voice->noteOff(note);
}

void Towns_EuphonyFmChannel::controlChange(byte control, byte value) {
	if (control == 121) {
		// Reset controller
		delete _voice;
		_voice = new Voice2612;
	} else if (control == 10) {
		// pan position
	} else {
		_voice->setControlParameter(control, value);
	}
}

void Towns_EuphonyFmChannel::sysEx_customInstrument(uint32, const byte *fmInst) {
	_voice->_rate = _rate;
	_voice->setInstrument(fmInst);
}

void Towns_EuphonyFmChannel::pitchBend(int16 value) {
	_voice->pitchBend(value);
}

void Towns_EuphonyFmChannel::nextTick(int32 *outbuf, int buflen) {
	_voice->nextTick((int*) outbuf, buflen);
}

void Towns_EuphonyFmChannel::rate(uint16 r) {
	_rate = r;
	_voice->_rate = r;
}

Towns_EuphonyPcmChannel::Towns_EuphonyPcmChannel() {
	_voice = new Voice;
	for (uint8 i = 0; i < 8; i++) {
		_voice->_env[i] = new Voice::Env;
		_voice->_snd[i] = 0;
	}

	_ctrl7_volume = 127;
	velocity(0);
	_frequencyOffs = 0x2000;
	_current = -1;
}

Towns_EuphonyPcmChannel::~Towns_EuphonyPcmChannel() {
	for (uint8 i = 0; i < 8; i++) {
		if (_voice->_snd[i])
			delete _voice->_snd[i];
		delete _voice->_env[i];
	}
	delete _voice;
}

void Towns_EuphonyPcmChannel::noteOn(byte note, byte onVelo) {
	_note = note;
	velocity(onVelo);
	_phase = 0;

	for (_current = 0; _current < 7; _current++) {
		if (note <=	_voice->split[_current])
			break;
	}

	_voice->_env[_current]->state = s_attacking;
	_voice->_env[_current]->currentLevel = 0;
	_voice->_env[_current]->rate = _rate;
	_voice->_env[_current]->tickCount = 0;
}

void Towns_EuphonyPcmChannel::noteOff(byte note) {
    if (_current == -1)
		return;
	if (_voice->_env[_current]->state == s_ready)
		return;

	_voice->_env[_current]->state = s_releasing;
	_voice->_env[_current]->releaseLevel = _voice->_env[_current]->currentLevel;
	_voice->_env[_current]->tickCount = 0;
}

void Towns_EuphonyPcmChannel::controlChange(byte control, byte value) {
	switch (control) {
		case 0x07:
			// volume
			_ctrl7_volume = value;
			break;
		case 0x0A:
			// pan position
			break;
		case 0x79:
			// Reset controller
			for (uint8 i = 0; i < 8; i++) {
				if (_voice->_snd[i])
					delete _voice->_snd[i];
				delete _voice->_env[i];
			}
			delete _voice;
			_voice = new Voice;
			for (uint8 i = 0; i < 8; i++) {
				_voice->_env[i] = new Voice::Env;
				_voice->_snd[i] = 0;
			}
			break;
		case 0x7B:
			noteOff(_note);
			break;
		default:
			break;
	}
}

void Towns_EuphonyPcmChannel::sysEx_customInstrument(uint32 type, const byte *fmInst) {
	if (type == 0x80) {
		for (uint8 i = 0; i < 8; i++) {
			const byte * const* pos = (const byte * const*) fmInst;
			for (uint8 ii = 0; ii < 10; ii++) {
				if (_voice->id[i] == *(pos[ii] + 8)) {
					if (!_voice->_snd[i])
						_voice->_snd[i] = new Voice::Snd;
					memset (_voice->_snd[i]->name, 0, 9);
					memcpy (_voice->_snd[i]->name, (const char*) pos[ii], 8);
					_voice->_snd[i]->id = READ_LE_UINT32(pos[ii] + 8);
					_voice->_snd[i]->numSamples = READ_LE_UINT32(pos[ii] + 12);
					_voice->_snd[i]->loopStart = READ_LE_UINT32(pos[ii] + 16);
					_voice->_snd[i]->loopLength = READ_LE_UINT32(pos[ii] + 20);
					_voice->_snd[i]->samplingRate = READ_LE_UINT16(pos[ii] + 24);
					_voice->_snd[i]->keyOffset = READ_LE_UINT16(pos[ii] + 26);
					_voice->_snd[i]->keyNote = *(const uint8*)(pos[ii] + 28);
					_voice->_snd[i]->_samples = (const int8*)(pos[ii] + 32);
				}
			}
		}
	} else {
		memset (_voice->name, 0, 9);
		memcpy (_voice->name, (const char*) fmInst, 8);

		for (uint8 i = 0; i < 8; i++) {
			_voice->split[i] = READ_LE_UINT16(fmInst + 16 + 2 * i);
			_voice->id[i] = READ_LE_UINT32(fmInst + 32 + 4 * i);
			_voice->_snd[i] = 0;
			_voice->_env[i]->state = s_ready;
			_voice->_env[i]->currentLevel = 0;
			_voice->_env[i]->totalLevel = *(fmInst + 64 + 8 * i);
			_voice->_env[i]->attackRate = *(fmInst + 65 + 8 * i) * 10;
			_voice->_env[i]->decayRate = *(fmInst + 66 + 8 * i) * 10;
			_voice->_env[i]->sustainLevel = *(fmInst + 67 + 8 * i);
			_voice->_env[i]->sustainRate = *(fmInst + 68 + 8 * i) * 20;
			_voice->_env[i]->releaseRate = *(fmInst + 69 + 8 * i) * 10;
			_voice->_env[i]->rootKeyOffset = *(fmInst + 70 + 8 * i);
		}
	}
}

void Towns_EuphonyPcmChannel::pitchBend(int16 value) {
	_frequencyOffs = value;
}

void Towns_EuphonyPcmChannel::nextTick(int32 *outbuf, int buflen) {
	if (_current == -1 || !_voice->_snd[_current] || !_voice->_env[_current]->state || !_velocity) {
		velocity(0);
		_current = -1;
		return;
	}

	float phaseStep = SoundTowns::semitoneAndSampleRate_to_sampleStep(_note, _voice->_snd[_current]->keyNote -
		_voice->_env[_current]->rootKeyOffset, _voice->_snd[_current]->samplingRate, _rate, _frequencyOffs);

	int32 looplength = _voice->_snd[_current]->loopLength;
	int32 numsamples = _voice->_snd[_current]->numSamples;
	const int8 * samples = _voice->_snd[_current]->_samples;

	for (int i = 0; i < buflen; i++) {
		if (looplength > 0) {
			while (_phase >= numsamples)
				_phase -= looplength;
		} else {
			if (_phase >= numsamples) {
				velocity(0);
				_current = -1;
				break;
			}
		}

		int32 output;

		int32 phase0 = int32(_phase);
		int32 phase1 = int32(_phase + 1);
		if (phase1 >= numsamples)
			phase1 -= looplength;
		float weight0 = _phase - phase0;
		float weight1 = phase1 - _phase;
		output = int32(samples[phase0] * weight0 + samples[phase1] * weight1);

		output *= _velocity;
		output <<= 1;

		evpNextTick();
		output *= _voice->_env[_current]->currentLevel;
		output >>= 7;
		output *= _ctrl7_volume;
		output >>= 7;

		output *= 185;
		output >>= 8;
		outbuf[i] += output;
		_phase += phaseStep;
	}
}

void Towns_EuphonyPcmChannel::evpNextTick() {
	switch (_voice->_env[_current]->state) {
		case s_ready:
			_voice->_env[_current]->currentLevel = 0;
			return;

		case s_attacking:
			if (_voice->_env[_current]->attackRate == 0)
				_voice->_env[_current]->currentLevel = _voice->_env[_current]->totalLevel;
			else if (_voice->_env[_current]->attackRate >= 1270)
				_voice->_env[_current]->currentLevel = 0;
			else
				_voice->_env[_current]->currentLevel = (_voice->_env[_current]->totalLevel *
					_voice->_env[_current]->tickCount++ * 1000) /
						(_voice->_env[_current]->attackRate * _voice->_env[_current]->rate);

			if (_voice->_env[_current]->currentLevel >= _voice->_env[_current]->totalLevel) {
				_voice->_env[_current]->currentLevel = _voice->_env[_current]->totalLevel;
				_voice->_env[_current]->state = s_decaying;
				_voice->_env[_current]->tickCount = 0;
			}
			break;

		case s_decaying:
			if (_voice->_env[_current]->decayRate == 0)
				_voice->_env[_current]->currentLevel = _voice->_env[_current]->sustainLevel;
			else if (_voice->_env[_current]->decayRate >= 1270)
				_voice->_env[_current]->currentLevel = _voice->_env[_current]->totalLevel;
			else {
				_voice->_env[_current]->currentLevel = _voice->_env[_current]->totalLevel;
				_voice->_env[_current]->currentLevel -= ((_voice->_env[_current]->totalLevel -
					_voice->_env[_current]->sustainLevel) * _voice->_env[_current]->tickCount++ * 1000) /
						(_voice->_env[_current]->decayRate * _voice->_env[_current]->rate);
			}

			if (_voice->_env[_current]->currentLevel <= _voice->_env[_current]->sustainLevel) {
				_voice->_env[_current]->currentLevel = _voice->_env[_current]->sustainLevel;
				_voice->_env[_current]->state = s_sustaining;
				_voice->_env[_current]->tickCount = 0;
			}
			break;

			case s_sustaining:
				if (_voice->_env[_current]->sustainRate == 0)
					_voice->_env[_current]->currentLevel = 0;
				else if (_voice->_env[_current]->sustainRate >= 2540)
					_voice->_env[_current]->currentLevel = _voice->_env[_current]->sustainLevel;
				else {
					_voice->_env[_current]->currentLevel = _voice->_env[_current]->sustainLevel;
					_voice->_env[_current]->currentLevel -= (_voice->_env[_current]->sustainLevel *
						_voice->_env[_current]->tickCount++ * 1000) / (_voice->_env[_current]->sustainRate *
							_voice->_env[_current]->rate);
				}

				if (_voice->_env[_current]->currentLevel <= 0) {
					_voice->_env[_current]->currentLevel = 0;
					_voice->_env[_current]->state = s_ready;
					_voice->_env[_current]->tickCount = 0;
				}
				break;

			case s_releasing:
				if (_voice->_env[_current]->releaseRate == 0)
					_voice->_env[_current]->currentLevel = 0;
				else if (_voice->_env[_current]->releaseRate >= 1270)
					_voice->_env[_current]->currentLevel = _voice->_env[_current]->releaseLevel;
				else {
					_voice->_env[_current]->currentLevel = _voice->_env[_current]->releaseLevel;
					_voice->_env[_current]->currentLevel -= (_voice->_env[_current]->releaseLevel *
						_voice->_env[_current]->tickCount++ * 1000) / (_voice->_env[_current]->releaseRate *
							_voice->_env[_current]->rate);
				}

				if (_voice->_env[_current]->currentLevel <= 0) {
					_voice->_env[_current]->currentLevel = 0;
					_voice->_env[_current]->state = s_ready;
				}
				break;

			default:
			break;
	}
}

void Towns_EuphonyPcmChannel::rate(uint16 r) {
	_rate = r;
}

void Towns_EuphonyPcmChannel::velocity(int velo) {
	_velocity = velo;
}

Towns_EuphonyDriver::Towns_EuphonyDriver(Audio::Mixer *mixer)
	: MidiDriver_Emulated(mixer) {
	_volume = 255;
	_fadestate = EUPHONY_FADEOUT_TICKS;
	_queue = 0;

	MidiDriver_YM2612::createLookupTables();

	for (uint8 i = 0; i < 6; i++)
		_channel[i] = _fChannel[i] = new Towns_EuphonyFmChannel;
	for (uint8 i = 0; i < 8; i++)
		_channel[i + 6] = _wChannel[i] = new Towns_EuphonyPcmChannel;
	_channel[14] = _channel[15] = 0;

	_fmInstruments = _waveInstruments = 0;
	memset(_waveSounds, 0, sizeof(uint8*) * 10);

	rate(getRate());
	fading(0);

	_queue = new Towns_EuphonyTrackQueue(this, 0);
}

Towns_EuphonyDriver::~Towns_EuphonyDriver() {
	for (int i = 0; i < 6; i++)
		delete _fChannel[i];
	for (int i = 0; i < 8; i++)
		delete _wChannel[i];

	MidiDriver_YM2612::removeLookupTables();

	if (_fmInstruments) {
		delete[] _fmInstruments;
		_fmInstruments = 0;
	}

	if (_waveInstruments) {
		delete[] _waveInstruments;
		_waveInstruments = 0;
	}

	for (int i = 0; i < 10; i++) {
		if (_waveSounds[i]) {
			delete[] _waveSounds[i];
			_waveSounds[i] = 0;
		}
	}

	if (_queue) {
		_queue->release();
		delete _queue;
		_queue = 0;
	}
}

int Towns_EuphonyDriver::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;
	MidiDriver_Emulated::open();

	_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_mixerSoundHandle,
		this, -1, Audio::Mixer::kMaxChannelVolume, 0, false, true);

	return 0;
}

void Towns_EuphonyDriver::close() {
	if (!_isOpen)
		return;
	_isOpen = false;
	_mixer->stopHandle(_mixerSoundHandle);
}

void Towns_EuphonyDriver::send(uint32 b) {
	send(b & 0xF, b & 0xFFFFFFF0);
}

void Towns_EuphonyDriver::send(byte chan, uint32 b) {
	byte param2 = (byte) ((b >> 16) & 0xFF);
	byte param1 = (byte) ((b >>  8) & 0xFF);
	byte cmd    = (byte) (b & 0xF0);
	if (chan > ARRAYSIZE(_channel))
		return;

	switch (cmd) {
	case 0x80:// Note Off
		if (_channel[chan])
			_channel[chan]->noteOff(param1);
		break;
	case 0x90: // Note On
		if (_channel[chan])
			_channel[chan]->noteOn(param1, param2);
		break;
	case 0xA0: // Aftertouch
		break; // Not supported.
	case 0xB0: // Control Change
		if (param1 == 0x79) {
			fading(0);
			for (int i = 0; i < 15; i++) {
				if (_channel[i]) {
					_channel[i]->controlChange(param1, param2);
					_channel[i]->programChange(0);
				}
			}
		} else if (param1 == 0x7B) {
			for (int i = 0; i < 15; i++) {
				if (_channel[i])
					_channel[i]->controlChange(param1, param2);
			}
		} else {
			if (_channel[chan])
				_channel[chan]->controlChange(param1, param2);
		}
		break;
	case 0xC0: // Program Change
        for (int i = 0; i < 6; i++) {
			if (_channel[chan] == _fChannel[i]) {
				_channel[chan]->sysEx_customInstrument(0, _fmInstruments + param1 * 0x30);
				break;
			}
		}
		for (int i = 0; i < 8; i++) {
			if (_channel[chan] == _wChannel[i]) {
				_channel[chan]->sysEx_customInstrument(0, _waveInstruments + param1 * 0x80);
				_channel[chan]->sysEx_customInstrument(0x80, (const byte*) _waveSounds);
				break;
			}
		}
		break;
	case 0xD0: // Channel Pressure
		break; // Not supported.
	case 0xE0: // Pitch Bend
		if (_channel[chan])
			_channel[chan]->pitchBend((param1 | (param2 << 7)) - 0x2000);
		break;
	default:
		warning("Towns_EuphonyDriver: Unknown send() command 0x%02X", cmd);
	}
}

void Towns_EuphonyDriver::loadFmInstruments(const byte *instr) {
	if (_fmInstruments)
		delete[] _fmInstruments;
	_fmInstruments = new uint8[0x1800];
	memcpy(_fmInstruments, instr, 0x1800);
}

void Towns_EuphonyDriver::loadWaveInstruments(const byte *instr) {
	if (_waveInstruments)
		delete[] _waveInstruments;
	_waveInstruments = new uint8[0x1000];
	memcpy(_waveInstruments, instr, 0x1000);

	const uint8 *pos = (const uint8 *)(instr + 0x1000);

	for (uint8 i = 0; i < 10; i++) {
		if (_waveSounds[i])
			delete[] _waveSounds[i];
		uint32 numsamples = READ_LE_UINT32(pos + 0x0C);
		_waveSounds[i] = new int8[numsamples + 0x20];
        memcpy(_waveSounds[i], pos, 0x20);
		pos += 0x20;
		for (uint32 ii = 0; ii < numsamples; ii++) {
			uint8 s = *(pos + ii);
			s = (s < 0x80) ? 0x80 - s : s;
			_waveSounds[i][ii + 0x20] = s ^ 0x80;
		}
		pos += numsamples;
	}
}


void Towns_EuphonyDriver::assignFmChannel(uint8 midiChannelNumber, uint8 fmChannelNumber) {
	_channel[midiChannelNumber] = _fChannel[fmChannelNumber];
}

void Towns_EuphonyDriver::assignWaveChannel(uint8 midiChannelNumber, uint8 waveChannelNumber) {
	_channel[midiChannelNumber] = _wChannel[waveChannelNumber];
}

void Towns_EuphonyDriver::removeChannel(uint8 midiChannelNumber) {
	_channel[midiChannelNumber] = 0;
}

void Towns_EuphonyDriver::generateSamples(int16 *data, int len) {
	memset(data, 0, 2 * sizeof(int16) * len);
	nextTick(data, len);
}

void Towns_EuphonyDriver::nextTick(int16 *buf1, int buflen) {
	int32 *buf0 = (int32 *)buf1;

	for (int i = 0; i < ARRAYSIZE(_channel); i++) {
		if (_channel[i])
			_channel[i]->nextTick(buf0, buflen);
	}

	for (int i = 0; i < buflen; ++i) {
		int s = int( float(buf0[i] * _volume) * float((float)_fadestate / EUPHONY_FADEOUT_TICKS) );
		buf1[i*2] = buf1[i*2+1] = (s >> 9) & 0xffff;
	}

	if (_fading) {
		if (_fadestate) {
			_fadestate--;
		} else {
			_fading = false;
			_queue->setPlayBackStatus(false);
		}
	}
}

void Towns_EuphonyDriver::rate(uint16 r) {
	for (uint8 i = 0; i < 16; i++) {
		if (_channel[i])
			_channel[i]->rate(r);
	}
}

void Towns_EuphonyDriver::fading(bool status) {
	_fading = status;
	if (!_fading)
		_fadestate = EUPHONY_FADEOUT_TICKS;
}

Towns_EuphonyParser::Towns_EuphonyParser(Towns_EuphonyTrackQueue * queue) : MidiParser(),
	_firstBaseTickStep(0x33), _nextBaseTickStep(0x33) {
		_initialTempo = calculateTempo(0x5a);
		_queue = queue;
}

void Towns_EuphonyParser::parseNextEvent(EventInfo &info) {
	byte *pos = _position._play_pos;

	if (_queue->_next) {
		if (info.ext.type == 0x2F) {
			unloadMusic();
			memset(&info, 0, sizeof(EventInfo));
			pos = _position._play_pos = _tracks[0] = _queue->trackData() + 0x806;
		} else if (_active_track == 255) {
			_queue = _queue->_next;
			setup();
			setTrack(0);
			_queue->setPlayBackStatus(true);
			return;
		} else if (!_queue->isPlaying()) {
			unloadMusic();
			_queue = _queue->_next;
			setup();
			setTrack(0);
			_queue->setPlayBackStatus(true);
			return;
		}
	}

	while (true) {
		byte cmd = *pos;
		byte evt = (cmd & 0xF0);

		if (evt == 0x90) {
			byte chan = pos[1];

			if (_enable[chan]) {
				uint16 tick = (pos[2] | ((uint16) pos[3] << 7)) + _baseTick;
				info.start = pos + 6;
				uint32 last = _position._last_event_tick;
				info.delta = (tick < last) ? 0 : (tick - last);

				info.event = 0x90 | _channel[chan];
				info.length = pos[7] | (pos[8] << 4) | (pos[9] << 8) | (pos[10] << 12);

				int8 note = (int8) pos[4];
				if (_adjNote[chan]) {
					note = (note & 0x7f) & _adjNote[chan];
					if (note > 0x7c)
						note -= 0x0c;
					else if (note < 0)
						note += 0x0c;
				}
				info.basic.param1 = (byte) note;

				uint8 onVelo = (pos[5] & 0x7f) + _adjVelo[chan];
				if (onVelo > 0x7f)
					onVelo = 0x7f;
				if (onVelo < 1)
					onVelo = 1;
				info.basic.param2 = onVelo;

				pos += 12;
				break;
			} else {
				pos += 6;
			}
		} else if (evt == 0xB0 || evt == 0xC0 || evt == 0xe0) {
			byte chan = pos[1];

			if (_enable[chan]) {
				info.start = pos;
				uint16 tick = (pos[2] | ((uint16) pos[3] << 7)) + _baseTick;
				uint32 last = _position._last_event_tick;
				info.delta = (tick < last) ? 0 : (tick - last);
				info.event = evt | _channel[chan];
				info.length = 0;
				info.basic.param1 = pos[4];
				info.basic.param2 = pos[5];
				pos += 6;
				break;
			} else {
				pos += 6;
			}
		} else if (cmd == 0xF2) {
			static const uint16 tickTable[] = { 0x180, 0xC0, 0x80, 0x60, 0x40, 0x30, 0x20, 0x18 };
			_baseTick += tickTable[_nextBaseTickStep >> 4] * ((_nextBaseTickStep & 0x0f) + 1);
			_nextBaseTickStep = pos[1];
			pos += 6;
		} else if (cmd == 0xF8) {
			int32 tempo = calculateTempo(pos[4] | (pos[5] << 7));
			info.event = 0xff;
			info.length = 3;
			info.ext.type = 0x51;
			_tempo[0] = (tempo >> 16) & 0xff;
			_tempo[1] = (tempo >> 8) & 0xff;
			_tempo[2] = tempo & 0xff;
			info.ext.data = (byte*) _tempo;
			pos += 6;
			break;
		} else if (cmd == 0xFD || cmd == 0xFE) {
			// End of track.
			if (_autoLoop) {
				unloadMusic();
				_queue->setPlayBackStatus(true);
				pos = info.start = _tracks[0];
			} else {
				info.start = pos;
			}

			uint32 last = _position._last_event_tick;
			uint16 tick = (pos[2] | ((uint16) pos[3] << 7)) + _baseTick;
			info.delta = (tick < last) ? 0 : (tick - last);
			info.event = 0xFF;
			info.ext.type = 0x2F;
			info.ext.data = pos;
			break;
		} else {
			error("Unknown Euphony music event 0x%02X", (int)cmd);
			memset(&info, 0, sizeof(info));
			pos = 0;
			break;
		}
	}
	_position._play_pos = pos;
}

bool Towns_EuphonyParser::loadMusic(byte *data, uint32 size) {
	bool loop = _autoLoop;

	if (_queue->isPlaying() && !_queue->_loop) {
		_queue->loadDataToEndOfQueue(data, size, loop);
	} else {
		unloadMusic();
		_queue = _queue->release();
		_queue->loadDataToCurrentPosition(data, size, loop);
		setup();
		setTrack(0);
		_queue->setPlayBackStatus(true);
	}
	return true;
}

int32 Towns_EuphonyParser::calculateTempo(int16 val) {
	int32 tempo = val;

	if (tempo < 0)
		tempo = 0;
	if (tempo > 0x1F4)
		tempo = 0x1F4;

	tempo = 0x4C4B4 / (tempo + 0x1E);
	while (tempo < 0x451)
		tempo <<= 1;
	tempo <<= 8;

	return tempo;
}

void Towns_EuphonyParser::resetTracking() {
	MidiParser::resetTracking();

	_nextBaseTickStep = _firstBaseTickStep;
	_baseTick = 0;
	setTempo(_initialTempo);
	_queue->setPlayBackStatus(false);
}

void Towns_EuphonyParser::setup() {
	uint8 *data = _queue->trackData();
	if (!data)
		return;
	_queue->initDriver();

	_enable = data + 0x354;
	_mode = data + 0x374;
	_channel = data + 0x394;
	_adjVelo = data + 0x3B4;
	_adjNote = (int8*) data + 0x3D4;

	_nextBaseTickStep = _firstBaseTickStep = data[0x804];
	_initialTempo = calculateTempo((data[0x805] > 0xfc) ? 0x5a : data[0x805]);

	property(MidiParser::mpAutoLoop, _queue->_loop);

	_num_tracks = 1;
	_ppqn = 120;
	_tracks[0] = data + 0x806;
}

Towns_EuphonyTrackQueue::Towns_EuphonyTrackQueue(Towns_EuphonyDriver * driver, Towns_EuphonyTrackQueue * last) {
	_trackData = 0;
	_next = 0;
	_driver = driver;
	_last = last;
	_used = _fchan = _wchan = 0;
	_playing = false;
}

void Towns_EuphonyTrackQueue::setPlayBackStatus(bool playing) {
	Towns_EuphonyTrackQueue * i = this;
	do {
		i->_playing = playing;
		i = i->_next;
	} while (i);
}

void Towns_EuphonyTrackQueue::loadDataToCurrentPosition(uint8 * trackdata, uint32 size, bool loop) {
	if (_trackData)
		delete[] _trackData;
	_trackData = new uint8[0xC58A];
	memset(_trackData, 0, 0xC58A);
	Screen::decodeFrame4(trackdata, _trackData, size);

	_used = _trackData + 0x374;
	_fchan = _trackData + 0x6d4;
	_wchan = _trackData + 0x6dA;
	_loop = loop;
	_playing = false;
}

void Towns_EuphonyTrackQueue::loadDataToEndOfQueue(uint8 * trackdata, uint32 size, bool loop) {
	if (!_trackData) {
		loadDataToCurrentPosition(trackdata, size, loop);
		return;
	}

	Towns_EuphonyTrackQueue * i = this;
	while (i->_next)
		i = i->_next;

	i = i->_next = new Towns_EuphonyTrackQueue(_driver, i);
	i->_trackData = new uint8[0xC58A];
	memset(i->_trackData, 0, 0xC58A);
	Screen::decodeFrame4(trackdata, i->_trackData, size);

	i->_used = i->_trackData + 0x374;
	i->_fchan = i->_trackData + 0x6d4;
	i->_wchan = i->_trackData + 0x6dA;
	i->_loop = loop;
	i->_playing = _playing;
}

Towns_EuphonyTrackQueue *Towns_EuphonyTrackQueue::release() {
	Towns_EuphonyTrackQueue *i = this;
	while (i->_next)
		i = i->_next;

	Towns_EuphonyTrackQueue *res = i;

	while (i) {
		i->_playing = false;
		i->_used = i->_fchan = i->_wchan = 0;
		if (i->_trackData) {
			delete[] i->_trackData;
			i->_trackData = 0;
		}
		i = i->_last;
		if (i) {
			res = i;
			if (i->_next) {
				delete i->_next;
				i->_next = 0;
			}
		}
	}

	if (res->_trackData) {
		delete[] res->_trackData;
		res->_trackData = 0;
	}

	return res;
}

void Towns_EuphonyTrackQueue::initDriver() {
	for (uint8 i = 0; i < 6; i++) {
		if (_used[_fchan[i]])
			_driver->assignFmChannel(_fchan[i], i);
	}

	for (uint8 i = 0; i < 8; i++) {
		if (_used[_wchan[i]])
			_driver->assignWaveChannel(_wchan[i], i);
	}

	for (uint8 i = 0; i < 16; i++) {
		if (!_used[i])
			_driver->removeChannel(i);
	}
	_driver->send(0x79B0);
}

class TownsPC98_OpnOperator {
public:
	TownsPC98_OpnOperator(double rate, const uint8 *rateTable,
		const uint8 *shiftTable, const uint8 *attackDecayTable, const uint32 *frqTable,
		const uint32 *sineTable, const int32 *tlevelOut, const int32 *detuneTable);
	~TownsPC98_OpnOperator() {}

	void keyOn();
	void keyOff();
	void frequency(int freq);
	void updatePhaseIncrement();
	void recalculateRates();
	void generateOutput(int phasebuf, int *_feedbuf, int &out);

	void feedbackLevel(int32 level) {_feedbackLevel = level ? level + 6 : 0; }
	void detune(int value) { _detn = &_detnTbl[value << 5]; }
	void multiple(uint32 value) { _multiple = value ? (value << 1) : 1;	}
	void attackRate(uint32 value) { _specifiedAttackRate = value; }
	bool scaleRate(uint8 value);
	void decayRate(uint32 value) { _specifiedDecayRate = value;	recalculateRates();	}
	void sustainRate(uint32 value) { _specifiedSustainRate = value;	recalculateRates();	}
	void sustainLevel(uint32 value) { _sustainLevel = (value == 0x0f) ? 0x3e0 : value << 5; }
	void releaseRate(uint32 value) { _specifiedReleaseRate = value;	recalculateRates();	}
	void totalLevel(uint32 value) { _totalLevel = value << 3; }
	void reset();

protected:
	EnvelopeState _state;
	uint32 _feedbackLevel;
	uint32 _multiple;
	uint32 _totalLevel;
	uint8 _keyScale1;
	uint8 _keyScale2;
	uint32 _specifiedAttackRate;
	uint32 _specifiedDecayRate;
	uint32 _specifiedSustainRate;
	uint32 _specifiedReleaseRate;
	uint32 _tickCount;
	uint32 _sustainLevel;

	uint32 _frequency;
	uint8 _kcode;
	uint32 _phase;
	uint32 _phaseIncrement;
	const int32 *_detn;

	const uint8 *_rateTbl;
	const uint8 *_rshiftTbl;
	const uint8 *_adTbl;
	const uint32 *_fTbl;
	const uint32 *_sinTbl;
	const int32 *_tLvlTbl;
	const int32 *_detnTbl;

	const double _tickLength;
	double _tick;
	int32 _currentLevel;

	struct EvpState {
		uint8 rate;
		uint8 shift;
	} fs_a, fs_d, fs_s, fs_r;
};

TownsPC98_OpnOperator::TownsPC98_OpnOperator(double rate, const uint8 *rateTable, 
	const uint8 *shiftTable, const uint8 *attackDecayTable,	const uint32 *frqTable,
	const uint32 *sineTable, const int32 *tlevelOut, const int32 *detuneTable) :
	_rateTbl(rateTable), _rshiftTbl(shiftTable), _adTbl(attackDecayTable), _fTbl(frqTable),
	_sinTbl(sineTable), _tLvlTbl(tlevelOut), _detnTbl(detuneTable), _tickLength(rate * 65536.0),
	_specifiedAttackRate(0), _specifiedDecayRate(0), _specifiedReleaseRate(0), _specifiedSustainRate(0),
	_phase(0), _state(s_ready) {
	
	reset();
}

void TownsPC98_OpnOperator::keyOn() {
	_state = s_attacking;
	_phase = 0;
}

void TownsPC98_OpnOperator::keyOff() {
	if (_state != s_ready)
		_state = s_releasing;
}

void TownsPC98_OpnOperator::frequency(int freq) {
	uint8 block = (freq >> 11);
	uint16 pos = (freq & 0x7ff);
	uint8 c = pos >> 7;
	_kcode = (block << 2) | ((c < 7) ? 0 : ((c > 8) ? 3 : c - 6 ));
	_frequency = _fTbl[pos << 1] >> (7 - block);
}

void TownsPC98_OpnOperator::updatePhaseIncrement() {
	_phaseIncrement = ((_frequency + _detn[_kcode]) * _multiple) >> 1;
	uint8 keyscale = _kcode >> _keyScale1;
	if (_keyScale2 != keyscale) {
		_keyScale2 = keyscale;
		recalculateRates();
	}
}

void TownsPC98_OpnOperator::recalculateRates() {
	int k = _keyScale2;
	int r = _specifiedAttackRate ? (_specifiedAttackRate << 1) + 0x20 : 0;
	fs_a.rate = ((r + k) < 94) ? _rateTbl[r + k] : 136;
	fs_a.shift = ((r + k) < 94) ? _rshiftTbl[r + k] : 0;

	r = _specifiedDecayRate ? (_specifiedDecayRate << 1) + 0x20 : 0;
	fs_d.rate = _rateTbl[r + k];
	fs_d.shift = _rshiftTbl[r + k];

	r = _specifiedSustainRate ? (_specifiedSustainRate << 1) + 0x20 : 0;
	fs_s.rate = _rateTbl[r + k];
	fs_s.shift = _rshiftTbl[r + k];

	r = (_specifiedReleaseRate << 2) + 0x22;
	fs_r.rate = _rateTbl[r + k];
	fs_r.shift = _rshiftTbl[r + k];
}

void TownsPC98_OpnOperator::generateOutput(int phasebuf, int *_feedbuf, int &out) {
	if (_state == s_ready)
		return;

	_tick += _tickLength;
	while (_tick > 0x30000) {
		_tick -= 0x30000;
		++_tickCount;

		int32 levelIncrement = 0;
		uint32 targetTime = 0;
		int32 targetLevel = 0;
		EnvelopeState next_state = s_ready;

		switch (_state) {
			case s_ready:
				return;
			case s_attacking:
				next_state = s_decaying;
				targetTime = (1 << fs_a.shift) - 1;
				targetLevel = 0;
				levelIncrement = (~_currentLevel * _adTbl[fs_a.rate + ((_tickCount >> fs_a.shift) & 7)]) >> 4;
				break;
			case s_decaying:
				targetTime = (1 << fs_d.shift) - 1;
				next_state = s_sustaining;
				targetLevel = _sustainLevel;
				levelIncrement = _adTbl[fs_d.rate + ((_tickCount >> fs_d.shift) & 7)];
				break;
			case s_sustaining:
				targetTime = (1 << fs_s.shift) - 1;
				next_state = s_ready;
				targetLevel = 1023;
				levelIncrement = _adTbl[fs_s.rate + ((_tickCount >> fs_s.shift) & 7)];
				break;
			case s_releasing:
				targetTime = (1 << fs_r.shift) - 1;
				next_state = s_ready;
				targetLevel = 1023;
				levelIncrement = _adTbl[fs_r.rate + ((_tickCount >> fs_r.shift) & 7)];
				break;
		}

		if (!(_tickCount & targetTime)) {
			_currentLevel += levelIncrement;
			if ((!targetLevel && _currentLevel <= targetLevel) || (targetLevel && _currentLevel >= targetLevel)) {
				if (_state != s_decaying)
					_currentLevel = targetLevel;
				if (_state != s_sustaining)
					_state = next_state;
			}
		}
	}

	uint32 lvlout = _totalLevel + (uint32) _currentLevel;

	int outp = 0;
	int *i = &outp, *o = &outp;
	int phaseShift = 0;

	if (_feedbuf) {
		o = &_feedbuf[0];
		i = &_feedbuf[1];
		phaseShift = _feedbackLevel ? ((_feedbuf[0] + _feedbuf[1]) << _feedbackLevel) : 0;
		if (phasebuf == -1)
			*i = 0;
		*o = *i;
	} else {
		phaseShift = phasebuf << 15;
	}		

	if (lvlout < 832) {
		uint32 index = (lvlout << 3) + _sinTbl[(((int32)((_phase & 0xffff0000)
			+ phaseShift)) >> 16) & 0x3ff];
		*i = ((index < 6656) ? _tLvlTbl[index] : 0);
	} else {
		*i = 0;
	}

	_phase += _phaseIncrement;
	out += *o;
	if (out > 32767)
		out = 32767;
	if (out < -32767)
		out = -32767;
}

void TownsPC98_OpnOperator::reset(){
	keyOff();
	_tick = 0;
	_keyScale2 = 0;
	_currentLevel = 1023;

	frequency(0);
	detune(0);
	scaleRate(0);
	multiple(0);
	updatePhaseIncrement();
	attackRate(0);
	decayRate(0);
	releaseRate(0);
	sustainRate(0);
	feedbackLevel(0);	
	totalLevel(127);
}

bool TownsPC98_OpnOperator::scaleRate(uint8 value) {
	value = 3 - value;
	if (_keyScale1 != value) {
		_keyScale1 = value;
		return true;
	}

	int k = _keyScale2;
	int r = _specifiedAttackRate ? (_specifiedAttackRate << 1) + 0x20 : 0;
	fs_a.rate = ((r + k) < 94) ? _rateTbl[r + k] : 136;
	fs_a.shift = ((r + k) < 94) ? _rshiftTbl[r + k] : 0;
	return false;
}

class TownsPC98_OpnDriver;
class TownsPC98_OpnChannel {
public:
	TownsPC98_OpnChannel(TownsPC98_OpnDriver *driver, uint8 regOffs, uint8 flgs, uint8 num,
		uint8 key, uint8 prt, uint8 id);
	virtual ~TownsPC98_OpnChannel();
	virtual void init();

	typedef bool (TownsPC98_OpnChannel::*ControlEventFunc)(uint8 para);

	typedef enum channelState {
		CHS_RECALCFREQ		=	0x01,
		CHS_KEYOFF			=	0x02,
		CHS_SSG				=	0x04,
		CHS_PITCHWHEELOFF	=	0x08,
		CHS_ALL_BUT_EOT		=	0x0f,
		CHS_EOT				=	0x80
	} ChannelState;

	virtual void loadData(uint8 *data);
	virtual void processEvents();
	virtual void processFrequency();
	bool processControlEvent(uint8 cmd);
	void writeReg(uint8 regAdress, uint8 value);

	virtual void keyOn();
	virtual void keyOff();	
	
	void setOutputLevel();
	void fadeStep();
	void reset();

	void updateEnv();
	void generateOutput(int16 &leftSample, int16 &rightSample, int *del, int *feed);

	bool _enableLeft;
	bool _enableRight;
	bool _updateEnvelopes;
	const uint8 _idFlag;
	int _feedbuf[3];

protected:
	bool control_dummy(uint8 para);
	bool control_f0_setPatch(uint8 para);
	bool control_f1_presetOutputLevel(uint8 para);
	bool control_f2_setKeyOffTime(uint8 para);
	bool control_f3_setFreqLSB(uint8 para);
	bool control_f4_setOutputLevel(uint8 para);
	bool control_f5_setTempo(uint8 para);
	bool control_f6_repeatSection(uint8 para);
	bool control_f7_setupPitchWheel(uint8 para);
	bool control_f8_togglePitchWheel(uint8 para);
	bool control_fa_writeReg(uint8 para);
	bool control_fb_incOutLevel(uint8 para);
	bool control_fc_decOutLevel(uint8 para);
	bool control_fd_jump(uint8 para);
	bool control_ff_endOfTrack(uint8 para);

	bool control_f0_setPatchSSG(uint8 para);
	bool control_f1_setTotalLevel(uint8 para);
	bool control_f4_setAlgorithm(uint8 para);
	bool control_f9_unkSSG(uint8 para);
	bool control_fb_incOutLevelSSG(uint8 para);
	bool control_fc_decOutLevelSSG(uint8 para);
	bool control_ff_endOfTrackSSG(uint8 para);

	uint8 _ticksLeft;
	uint8 _algorithm;
	uint8 _instrID;
	uint8 _totalLevel;
	uint8 _frqBlockMSB;
	int8 _frqLSB;
	uint8 _keyOffTime;
	bool _protect;
	uint8 *_dataPtr;
	uint8 _ptchWhlInitDelayLo;
	uint8 _ptchWhlInitDelayHi;
	int16 _ptchWhlModInitVal;
	uint8 _ptchWhlDuration;
	uint8 _ptchWhlCurDelay;
	int16 _ptchWhlModCurVal;
	uint8 _ptchWhlDurLeft;
	uint16 frequency;
	uint8 _regOffset;
	uint8 _flags;
	uint8 _ssg1;
	uint8 _ssg2;

	const uint8 _chanNum;
	const uint8 _keyNum;
	const uint8 _part;

	TownsPC98_OpnDriver *_drv;
	TownsPC98_OpnOperator **_opr;
	uint16 _frqTemp;

	const ControlEventFunc *controlEvents;
};

class TownsPC98_OpnChannelSSG : public TownsPC98_OpnChannel {
public:
	TownsPC98_OpnChannelSSG(TownsPC98_OpnDriver *driver, uint8 regOffs,
		uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id);
	~TownsPC98_OpnChannelSSG() {}
	void init();

	void processEvents();
	void processFrequency();

	void keyOn();
	void keyOff();
	void loadData(uint8 *data);

private:
	void opn_SSG_UNK(uint8 a);
};


class TownsPC98_OpnDriver : public Audio::AudioStream {
friend class TownsPC98_OpnChannel;
friend class TownsPC98_OpnChannelSSG;
public:
	enum OpnType {
		OD_TOWNS,
		OD_TYPE26,
		OD_TYPE86
	};

	TownsPC98_OpnDriver(Audio::Mixer *mixer, OpnType type);
	~TownsPC98_OpnDriver();

	bool init();
	void loadData(uint8 *data, bool loadPaused = false);
	void reset();
	void fadeOut();
	
	void pause() { _playing = false; }
	void cont() { _playing = true; }

	void callback();
	void nextTick(int16 *buffer, uint32 bufferSize);

	bool looping() { return _looping == _updateChannelsFlag ? true : false; }

	// AudioStream interface
	int inline readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return true; }
	bool endOfData() const { return false; }
	int getRate() const { return _mixer->getOutputRate(); }

protected:
	void generateTables();

	TownsPC98_OpnChannel **_channels;
	TownsPC98_OpnChannelSSG **_ssgChannels;
	//TownsPC98_OpnChannel *_adpcmChannel;

	void setTempo(uint8 tempo);

	void lock() { _mutex.lock(); }
	void unlock() { _mutex.unlock(); }

	Audio::Mixer *_mixer;
	Common::Mutex _mutex;
	Audio::SoundHandle _soundHandle;

	const uint8 *_opnCarrier;
	const uint8 *_opnFreqTable;
	const uint8 *_opnFxCmdLen;
	const uint8 *_opnLvlPresets;

	uint8 *_oprRates;
	uint8 *_oprRateshift;
	uint8 *_oprAttackDecay;
	uint32 *_oprFrq;
	uint32 *_oprSinTbl;
	int32 *_oprLevelOut;
	int32 *_oprDetune;

	uint8 *_trackData;
	uint8 *_patches;

	uint8 _cbCounter;
	uint8 _updateChannelsFlag;
	uint8 _finishedChannelsFlag;
	uint16 _tempo;
	bool _playing;
	bool _fading;
	uint8 _looping;
	uint32 _tickCounter;

	bool _updateEnvelopes;
	int _ssgFlag;

	int32 _samplesTillCallback;
	int32 _samplesTillCallbackRemainder;
	int32 _samplesPerCallback;
	int32 _samplesPerCallbackRemainder;

	const int _numChan;
	const int _numSSG;
	const bool _hasADPCM;
	const bool _hasStereo;

	double _baserate;
	static const uint8 _drvTables[];
	static const uint32 _adtStat[];
	bool _ready;
};

TownsPC98_OpnChannel::TownsPC98_OpnChannel(TownsPC98_OpnDriver *driver, uint8 regOffs, uint8 flgs, uint8 num,
	uint8 key, uint8 prt, uint8 id) : _drv(driver), _regOffset(regOffs), _flags(flgs), _chanNum(num), _keyNum(key),
	_part(prt), _idFlag(id) {

	_ticksLeft = _algorithm = _instrID = _totalLevel = _frqBlockMSB = _keyOffTime = _ssg1 = _ssg2 = 0;
	_ptchWhlInitDelayLo = _ptchWhlInitDelayHi = _ptchWhlDuration = _ptchWhlCurDelay = _ptchWhlDurLeft = 0;
	_frqLSB = 0;
	_protect = _updateEnvelopes = false;
	_enableLeft = _enableRight = true;
	_dataPtr = 0;		
	_ptchWhlModInitVal = _ptchWhlModCurVal = 0;
	frequency = _frqTemp = 0;
	memset(&_feedbuf, 0, sizeof(int) * 3);
	_opr = 0;
}

TownsPC98_OpnChannel::~TownsPC98_OpnChannel() {
	if (_opr) {
		for (int i = 0; i < 4; i++)
			delete _opr[i];
		delete [] _opr;
	}
}

void TownsPC98_OpnChannel::init() {
	
	_opr = new TownsPC98_OpnOperator*[4];
	for (int i = 0; i < 4; i++)
		_opr[i] = new TownsPC98_OpnOperator(_drv->_baserate, _drv->_oprRates, _drv->_oprRateshift,
			_drv->_oprAttackDecay, _drv->_oprFrq, _drv->_oprSinTbl, _drv->_oprLevelOut, _drv->_oprDetune);

	#define Control(x)	&TownsPC98_OpnChannel::control_##x
	static const ControlEventFunc ctrlEvents[] = {
		Control(f0_setPatch),
		Control(f1_presetOutputLevel),
		Control(f2_setKeyOffTime),
		Control(f3_setFreqLSB),
		Control(f4_setOutputLevel),
		Control(f5_setTempo),
		Control(f6_repeatSection),
		Control(f7_setupPitchWheel),
		Control(f8_togglePitchWheel),
		Control(dummy),
		Control(fa_writeReg),
		Control(fb_incOutLevel),
		Control(fc_decOutLevel),
		Control(fd_jump),
		Control(dummy),
		Control(ff_endOfTrack)
	};
	#undef Control

	controlEvents = ctrlEvents;
}

void TownsPC98_OpnChannel::keyOff() {
	// all operators off
	uint8 value = _keyNum & 0x0f;
	uint8 regAdress = 0x28;
	writeReg(regAdress, value);
	_flags |= CHS_KEYOFF;
}

void TownsPC98_OpnChannel::keyOn() {
	// all operators on
	uint8 value = _keyNum | 0xf0;
	uint8 regAdress = 0x28;
	writeReg(regAdress, value);
}

void TownsPC98_OpnChannel::loadData(uint8 *data) {
	_flags = (_flags & ~CHS_EOT) | CHS_ALL_BUT_EOT;
	_ticksLeft = 1;
	_dataPtr = data;
	_totalLevel = 0x7F;

	uint8 *src_b = _dataPtr;
	int loop = 1;
	uint8 cmd = 0;
	while (loop) {
		if (loop == 1) {
			cmd = *src_b++;
			if (cmd < 0xf0) {
				src_b++;
				loop = 1;
			} else {
				if (cmd == 0xff) {
					loop = *src_b ? 2 : 0;
					if (READ_LE_UINT16(src_b))
						_drv->_looping |= _idFlag;
				} else if (cmd == 0xf6) {
					loop = 3;
				} else {
					loop = 2;
				}
			}
		} else if (loop == 2) {
			src_b += _drv->_opnFxCmdLen[cmd - 240];
			loop = 1;
		} else if (loop == 3) {
			src_b[0] = src_b[1];
			src_b += 4;
			loop = 1;
		}
	}
}

void TownsPC98_OpnChannel::processEvents() {
	if (_flags & CHS_EOT)
		return;

	if (_protect == false && _ticksLeft == _keyOffTime)
		keyOff();

	if (--_ticksLeft)
		return;

	if (_protect == false)
		keyOff();

	uint8 cmd = 0;
	bool loop = true;

	while (loop) {
		cmd = *_dataPtr++;
		if (cmd < 0xf0)
			loop = false;
		else if (!processControlEvent(cmd))
			return;
	}

	uint8 para = *_dataPtr++;

	if (cmd == 0x80) {
		keyOff();
		_protect = false;
	} else {
		keyOn();

		if (_protect == false || cmd != _frqBlockMSB)
			_flags |= CHS_RECALCFREQ;
	
		_protect = (para & 0x80) ? true : false;
		_frqBlockMSB = cmd;
	}

	_ticksLeft = para & 0x7f;
}

void TownsPC98_OpnChannel::processFrequency() {
	if (_flags & CHS_RECALCFREQ) {
		uint8 block = (_frqBlockMSB & 0x70) >> 1;
		uint16 bfreq = ((const uint16*)_drv->_opnFreqTable)[_frqBlockMSB & 0x0f];
		frequency = (bfreq + _frqLSB) | (block << 8);

		writeReg(_regOffset + 0xa4, (frequency >> 8));
		writeReg(_regOffset + 0xa0, (frequency & 0xff));

		_ptchWhlCurDelay = _ptchWhlInitDelayHi;
		if (_flags & CHS_KEYOFF) {
			_ptchWhlModCurVal = _ptchWhlModInitVal;
			_ptchWhlCurDelay += _ptchWhlInitDelayLo;
		}

		_ptchWhlDurLeft = (_ptchWhlDuration >> 1);
		_flags &= ~(CHS_KEYOFF | CHS_RECALCFREQ);
	}

	if (!(_flags & CHS_PITCHWHEELOFF)) {
		if (--_ptchWhlCurDelay)
			return;
		_ptchWhlCurDelay = _ptchWhlInitDelayHi;
		frequency += _ptchWhlModCurVal;

		writeReg(_regOffset + 0xa4, (frequency >> 8));
		writeReg(_regOffset + 0xa0, (frequency & 0xff));

		if(!--_ptchWhlDurLeft) {
			_ptchWhlDurLeft = _ptchWhlDuration;
			_ptchWhlModCurVal = -_ptchWhlModCurVal;
		}
	}
}

bool TownsPC98_OpnChannel::processControlEvent(uint8 cmd) {
	uint8 para = *_dataPtr++;
	return (this->*controlEvents[cmd & 0x0f])(para);
}

void TownsPC98_OpnChannel::setOutputLevel() {
	uint8 outopr = _drv->_opnCarrier[_algorithm];
	uint8 reg = 0x40 + _regOffset;

	for (int i = 0; i < 4; i++) {
		if (outopr & 1)
			writeReg(reg, _totalLevel);
		outopr >>= 1;
		reg += 4;
	}
}

void TownsPC98_OpnChannel::fadeStep() {
	_totalLevel += 3;
	if (_totalLevel > 0x7f)
		_totalLevel = 0x7f;
	setOutputLevel();
}

void TownsPC98_OpnChannel::reset() {
	for (int i = 0; i < 4; i++)
		_opr[i]->reset();

	_updateEnvelopes = false;
	_enableLeft = _enableRight = true;
	memset(&_feedbuf, 0, sizeof(int) * 3);
}

void TownsPC98_OpnChannel::updateEnv() {
	for (int i = 0; i < 4 ; i++)
		_opr[i]->updatePhaseIncrement();
}

void TownsPC98_OpnChannel::generateOutput(int16 &leftSample, int16 &rightSample, int *del, int *feed) {
	int phbuf1, phbuf2, output;
	phbuf1 = phbuf2 = output = 0;
	
	switch (_algorithm) {
		case 0:
			_opr[0]->generateOutput(0, feed, phbuf1);
			_opr[2]->generateOutput(*del, 0, phbuf2);
			*del = 0;
			_opr[1]->generateOutput(phbuf1, 0, *del);
			_opr[3]->generateOutput(phbuf2, 0, output);
			break;
		case 1:
			_opr[0]->generateOutput(0, feed, phbuf1);
			_opr[2]->generateOutput(*del, 0, phbuf2);
			_opr[1]->generateOutput(0, 0, phbuf1);					
			_opr[3]->generateOutput(phbuf2, 0, output);
			*del = phbuf1;
			break;
		case 2:
			_opr[0]->generateOutput(0, feed, phbuf2);
			_opr[2]->generateOutput(*del, 0, phbuf2);
			_opr[1]->generateOutput(0, 0, phbuf1);
			_opr[3]->generateOutput(phbuf2, 0, output);
			*del = phbuf1;
			break;
		case 3:
			_opr[0]->generateOutput(0, feed, phbuf2);
			_opr[2]->generateOutput(0, 0, *del);
			_opr[1]->generateOutput(phbuf2, 0, phbuf1);
			_opr[3]->generateOutput(*del, 0, output);
			*del = phbuf1;
			break;
		case 4:
			_opr[0]->generateOutput(0, feed, phbuf1);
			_opr[2]->generateOutput(0, 0, phbuf2);
			_opr[1]->generateOutput(phbuf1, 0, output);					
			_opr[3]->generateOutput(phbuf2, 0, output);
			*del = 0;
			break;
		case 5:
			*del = feed[1];
			_opr[0]->generateOutput(-1, feed, phbuf1);
			_opr[2]->generateOutput(*del, 0, output);
			_opr[1]->generateOutput(*del, 0, output);
			_opr[3]->generateOutput(*del, 0, output);
			break;
		case 6:
			_opr[0]->generateOutput(0, feed, phbuf1);
			_opr[2]->generateOutput(0, 0, output);
			_opr[1]->generateOutput(phbuf1, 0, output);
			_opr[3]->generateOutput(0, 0, output);
			*del = 0;
			break;
		case 7:
			_opr[0]->generateOutput(0, feed, output);
			_opr[2]->generateOutput(0, 0, output);
			_opr[1]->generateOutput(0, 0, output);
			_opr[3]->generateOutput(0, 0, output);
			*del = 0;
			break;
		};

	if (_enableLeft) {
		int l = output + leftSample;
		if (l > 32767)
			l = 32767;
		if (l < -32767)
			l = -32767;
		leftSample = (int16) l;
	}

	if (_enableRight) {
		int r = output + rightSample;
		if (r > 32767)
			r = 32767;
		if (r < -32767)
			r = -32767;
		rightSample = (int16) r;
	}
}

void TownsPC98_OpnChannel::writeReg(uint8 regAdress, uint8 value) {
	uint8 h = regAdress & 0xf0;
	uint8 l = (regAdress & 0x0f);
	static const uint8 oprOrdr[] = { 0, 2, 1, 3 };
	uint8 o = oprOrdr[(l - _regOffset) >> 2];
	
	switch (h) {
		case 0x00:
			// ssg
			warning("TownsPC98_OpnDriver: UNKNOWN ADDRESS %d", regAdress);
			break;
		case 0x10:
			// adpcm
			warning("TownsPC98_OpnDriver: UNKNOWN ADDRESS %d", regAdress);
			break;
		case 0x20:
			if (l == 8) {
				// Key on/off
				for (int i = 0; i < 4; i++) {
					if ((value >> (4 + i)) & 1)
						_opr[i]->keyOn();
					else
						_opr[i]->keyOff();
				}
			} else if (l == 2) {
				// LFO
				warning("TownsPC98_OpnDriver: TRYING TO USE LFO (NOT SUPPORTED)");
			} else if (l == 7) {
				// Timers; Ch 3/6 special mode
				warning("TownsPC98_OpnDriver: TRYING TO USE CH 3/6 SPECIAL MODE (NOT SUPPORTED)");
			} else if (l == 4 || l == 5) {
				// Timer A
				warning("TownsPC98_OpnDriver: TRYING TO USE TIMER_A (NOT SUPPORTED)");
			} else if (l == 6) {
				// Timer B
				warning("TownsPC98_OpnDriver: TRYING TO USE TIMER_B (NOT SUPPORTED)");
			} else if (l == 10 || l == 11) {
				// DAC
				warning("TownsPC98_OpnDriver: TRYING TO USE DAC (NOT SUPPORTED)");
			}
			break;

		case 0x30:
			// detune, multiple
			_opr[o]->detune((value >> 4) & 7);
			_opr[o]->multiple(value & 0x0f);
			_updateEnvelopes = true;
			break;

		case 0x40:
			// total level
			_opr[o]->totalLevel(value & 0x7f);
			break;

		case 0x50:
			// rate scaling, attack rate
			_opr[o]->attackRate(value & 0x1f);
			if (_opr[o]->scaleRate(value >> 6))
				_updateEnvelopes = true;
			break;

		case 0x60:
			// first decay rate, amplitude modulation
			_opr[o]->decayRate(value & 0x1f);
			if (value & 0x80)
				warning("TownsPC98_OpnDriver: TRYING TO USE AMP MODULATION (NOT SUPPORTED)");

			break;

		case 0x70:
			// secondary decay rate
			_opr[o]->sustainRate(value & 0x1f);
			break;

		case 0x80:
			// secondary amplitude, release rate;
			_opr[o]->sustainLevel(value >> 4);
			_opr[o]->releaseRate(value & 0x0f);
			break;

		case 0x90:
			// ssg
			warning("TownsPC98_OpnDriver: UNKNOWN ADDRESS %d", regAdress);
			break;

		case 0xa0:
			// frequency
			l -= _regOffset;
			if (l == 0) {
				_frqTemp = (_frqTemp & 0xff00) | value;
				_updateEnvelopes = true;
				for (int i = 0; i < 4; i++)
					_opr[i]->frequency(_frqTemp);
			} else if (l == 4) {
				_frqTemp = (_frqTemp & 0xff) | (value << 8);
			} else if (l == 8) {
				// Ch 3/6 special mode frq
				warning("TownsPC98_OpnDriver: TRYING TO USE CH 3/6 SPECIAL MODE FREQ (NOT SUPPORTED)");
			} else if (l == 12) {
				// Ch 3/6 special mode frq
				warning("TownsPC98_OpnDriver: TRYING TO USE CH 3/6 SPECIAL MODE FREQ (NOT SUPPORTED)");
			}
			break;

		case 0xb0:
			l -= _regOffset;
			if (l == 0) {
				// feedback, _algorithm
				_opr[0]->feedbackLevel((value >> 3) & 7);
				_opr[1]->feedbackLevel(0);
				_opr[2]->feedbackLevel(0);
				_opr[3]->feedbackLevel(0);
			} else if (l == 4) {
				// stereo, LFO sensitivity
				_enableLeft = value & 0x80 ? true : false;
				_enableRight = value & 0x40 ? true : false;
				uint8 ams = (value & 0x3F) >> 3;
				if (ams)
					warning("TownsPC98_OpnDriver: TRYING TO USE AMP MODULATION SENSITIVITY (NOT SUPPORTED)");
				uint8 fms = value & 3;
				if (fms)
					warning("TownsPC98_OpnDriver: TRYING TO USE FREQ MODULATION SENSITIVITY (NOT SUPPORTED)");
			}
			break;

		default:
			warning("TownsPC98_OpnDriver: UNKNOWN ADDRESS %d", regAdress);
			break;
	}
}

bool TownsPC98_OpnChannel::control_f0_setPatch(uint8 para) {
	_instrID = para;
	uint8 reg = _regOffset + 0x80;

	for (int i = 0; i < 4; i++) {
		// set release rate for each operator
		writeReg(reg, 0x0f);
		reg += 4;
	}

	const uint8 *tptr = _drv->_patches + ((uint32)_instrID << 5);
	reg = _regOffset + 0x30;

	// write registers 0x30 to 0x8f
	for (int i = 0; i < 6; i++) {
		writeReg(reg, tptr[0]);
		reg += 4;
		writeReg(reg, tptr[2]);
		reg += 4;
		writeReg(reg, tptr[1]);
		reg += 4;
		writeReg(reg, tptr[3]);
		reg += 4;
		tptr += 4;
	}

	reg = _regOffset + 0xB0;
	_algorithm = tptr[0] & 7;
	// set feedback and algorithm
	writeReg(reg, tptr[0]);

	setOutputLevel();
	return true;
}

bool TownsPC98_OpnChannel::control_f1_presetOutputLevel(uint8 para) {
	if (_drv->_fading)
		return true;

	_totalLevel = _drv->_opnLvlPresets[para];
	setOutputLevel();
	return true;
}

bool TownsPC98_OpnChannel::control_f2_setKeyOffTime(uint8 para) {
	_keyOffTime = para;
	return true;
}

bool TownsPC98_OpnChannel::control_f3_setFreqLSB(uint8 para) {
	_frqLSB = (int8) para;
	return true;
}

bool TownsPC98_OpnChannel::control_f4_setOutputLevel(uint8 para) {
	if (_drv->_fading)
		return true;

	_totalLevel = para;
	setOutputLevel();
	return true;
}

bool TownsPC98_OpnChannel::control_f5_setTempo(uint8 para) {
	_drv->setTempo(para);
	return true;
}

bool TownsPC98_OpnChannel::control_f6_repeatSection(uint8 para) {
	_dataPtr--;
	_dataPtr[0]--;

	if (*_dataPtr) {
		// repeat section until counter has reached zero
		_dataPtr = _drv->_trackData + READ_LE_UINT16(_dataPtr + 2);
	} else {
		// reset counter, advance to next section
		_dataPtr[0] = _dataPtr[1];
		_dataPtr += 4;
	}
	return true;
}

bool TownsPC98_OpnChannel::control_f7_setupPitchWheel(uint8 para) {
	_ptchWhlInitDelayLo = _dataPtr[0];
	_ptchWhlInitDelayHi = para;
	_ptchWhlModInitVal = (int16) READ_LE_UINT16(_dataPtr + 1);
	_ptchWhlDuration = _dataPtr[3];
	_dataPtr += 4;
	_flags = (_flags & ~CHS_PITCHWHEELOFF) | CHS_KEYOFF | CHS_RECALCFREQ;
	return true;
}

bool TownsPC98_OpnChannel::control_f8_togglePitchWheel(uint8 para) {
	if (para == 0x10) {
		if (*_dataPtr++) {
			_flags = (_flags & ~CHS_PITCHWHEELOFF) | CHS_KEYOFF;
		} else {
			_flags |= CHS_PITCHWHEELOFF;
		}
	} else {
		//uint8 skipChannels = para / 36;
		//uint8 entry = para % 36;
		//TownsPC98_OpnDriver::TownsPC98_OpnChannel *t = &chan[skipChannels];
		////// NOT IMPLEMENTED
		//t->unnamedEntries[entry] = *_dataPtr++;
	}
	return true;
}

bool TownsPC98_OpnChannel::control_fa_writeReg(uint8 para) {
	writeReg(para, *_dataPtr++);
	return true;
}

bool TownsPC98_OpnChannel::control_fb_incOutLevel(uint8 para) {
	_dataPtr--;
	if (_drv->_fading)
		return true;

	uint8 val = (_totalLevel + 3);
	if (val > 0x7f)
		val = 0x7f;

	_totalLevel = val;
	setOutputLevel();
	return true;
}

bool TownsPC98_OpnChannel::control_fc_decOutLevel(uint8 para) {
	_dataPtr--;
	if (_drv->_fading)
		return true;

	int8 val = (int8) (_totalLevel - 3);
	if (val < 0)
		val = 0;

	_totalLevel = (uint8) val;
	setOutputLevel();
	return true;
}

bool TownsPC98_OpnChannel::control_fd_jump(uint8 para) {
	uint8 *tmp = _drv->_trackData + READ_LE_UINT16(_dataPtr - 1);
	_dataPtr = (tmp[1] == 1) ? tmp : ++_dataPtr;
	return true;
}

bool TownsPC98_OpnChannel::control_dummy(uint8 para) {
	_dataPtr--;
	return true;
}

bool TownsPC98_OpnChannel::control_ff_endOfTrack(uint8 para) {
	uint16 val = READ_LE_UINT16(--_dataPtr);
	if (val) {
		// loop
		_dataPtr = _drv->_trackData + val;
		return true;
	} else {
		// quit parsing for active channel
		--_dataPtr;
		_flags |= CHS_EOT;
		_drv->_finishedChannelsFlag |= _idFlag;
		keyOff();
		return false;
	}
}

bool TownsPC98_OpnChannel::control_f0_setPatchSSG(uint8 para) {
	_instrID = para << 4;
	para = (para >> 3) & 0x1e;
	if (para)
		return control_f4_setAlgorithm(para | 0x40);
	return true;
}

bool TownsPC98_OpnChannel::control_f1_setTotalLevel(uint8 para) {
	if (!_drv->_fading)
		_totalLevel = para;
	return true;
}

bool TownsPC98_OpnChannel::control_f4_setAlgorithm(uint8 para) {
	_algorithm = para;
	return true;
}

bool TownsPC98_OpnChannel::control_f9_unkSSG(uint8 para) {
	_dataPtr += 5;
	return true;
}

bool TownsPC98_OpnChannel::control_fb_incOutLevelSSG(uint8 para) {
	_dataPtr--;
	if (_drv->_fading)
		return true;

	_totalLevel--;
	if ((int8)_totalLevel < 0)
		_totalLevel = 0;

	return true;
}

bool TownsPC98_OpnChannel::control_fc_decOutLevelSSG(uint8 para) {
	_dataPtr--;
	if (_drv->_fading)
		return true;

	if(_totalLevel + 1 < 0x10)
		_totalLevel++;

	return true;
}

bool TownsPC98_OpnChannel::control_ff_endOfTrackSSG(uint8 para) {
	uint16 val = READ_LE_UINT16(--_dataPtr);
	if (val) {
		// loop
		_dataPtr = _drv->_trackData + val;
		return true;
	} else {
		// quit parsing for active channel
		--_dataPtr;
		_flags |= CHS_EOT;
		//_finishedChannelsFlag |= _idFlag;
		keyOff();
		return false;
	}
}

TownsPC98_OpnChannelSSG::TownsPC98_OpnChannelSSG(TownsPC98_OpnDriver *driver, uint8 regOffs, 
		uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id) :
		TownsPC98_OpnChannel(driver, regOffs, flgs, num, key, prt, id) {
}

void TownsPC98_OpnChannelSSG::init() {
	_algorithm = 0x80;
	
	_opr = new TownsPC98_OpnOperator*[4];
	for (int i = 0; i < 4; i++)
		_opr[i] = new TownsPC98_OpnOperator(_drv->_baserate, _drv->_oprRates, _drv->_oprRateshift,
			_drv->_oprAttackDecay, _drv->_oprFrq, _drv->_oprSinTbl, _drv->_oprLevelOut, _drv->_oprDetune);

	#define Control(x)	&TownsPC98_OpnChannelSSG::control_##x
	static const ControlEventFunc ctrlEventsSSG[] = {
		Control(f0_setPatchSSG),
		Control(f1_setTotalLevel),
		Control(f2_setKeyOffTime),
		Control(f3_setFreqLSB),
		Control(f4_setOutputLevel),
		Control(f5_setTempo),
		Control(f6_repeatSection),
		Control(f7_setupPitchWheel),
		Control(f8_togglePitchWheel),
		Control(f9_unkSSG),
		Control(fa_writeReg),
		Control(fb_incOutLevelSSG),
		Control(fc_decOutLevelSSG),
		Control(fd_jump),
		Control(dummy),
		Control(ff_endOfTrackSSG)
	};
	#undef Control

	controlEvents = ctrlEventsSSG;
}

void TownsPC98_OpnChannelSSG::processEvents() {
	if (_flags & CHS_EOT)
		return;

	_drv->_ssgFlag = (_flags & CHS_SSG) ? -1 : 0;

	if (_protect == false && _ticksLeft == _keyOffTime)
		keyOff();

	if (--_ticksLeft)
		return;

	if (_protect == false)
		keyOff();

	uint8 cmd = 0;
	bool loop = true;

	while (loop) {
		cmd = *_dataPtr++;
		if (cmd < 0xf0)
			loop = false;
		else if (!processControlEvent(cmd))
			return;
	}

	uint8 para = *_dataPtr++;

	if (cmd == 0x80) {
		keyOff();
		_protect = false;
	} else {
		keyOn();

		if (_protect == false || cmd != _frqBlockMSB)
			_flags |= CHS_RECALCFREQ;
	
		_protect = (para & 0x80) ? true : false;
		_frqBlockMSB = cmd;
	}

	_ticksLeft = para & 0x7f;

	if (!(_flags & CHS_SSG)) {

	}
}

void TownsPC98_OpnChannelSSG::processFrequency() {
	if (_flags & CHS_RECALCFREQ) {
		uint8 block = (_frqBlockMSB & 0x70) >> 1;
		uint16 bfreq = ((const uint16*)_drv->_opnFreqTable)[_frqBlockMSB & 0x0f];
		frequency = (bfreq + _frqLSB) | (block << 8);

		writeReg(_regOffset + 0xa4, (frequency >> 8));
		writeReg(_regOffset + 0xa0, (frequency & 0xff));

		_ptchWhlCurDelay = _ptchWhlInitDelayHi;
		if (_flags & CHS_KEYOFF) {
			_ptchWhlModCurVal = _ptchWhlModInitVal;
			_ptchWhlCurDelay += _ptchWhlInitDelayLo;
		}

		_ptchWhlDurLeft = (_ptchWhlDuration >> 1);
		_flags &= ~(CHS_KEYOFF | CHS_RECALCFREQ);
	}

	if (!(_flags & CHS_PITCHWHEELOFF)) {
		if (--_ptchWhlCurDelay)
			return;
		_ptchWhlCurDelay = _ptchWhlInitDelayHi;
		frequency += _ptchWhlModCurVal;

		writeReg(_regOffset + 0xa4, (frequency >> 8));
		writeReg(_regOffset + 0xa0, (frequency & 0xff));

		if(!--_ptchWhlDurLeft) {
			_ptchWhlDurLeft = _ptchWhlDuration;
			_ptchWhlModCurVal = -_ptchWhlModCurVal;
		}
	}
}

void TownsPC98_OpnChannelSSG::keyOff() {
	// all operators off
	uint8 value = _keyNum & 0x0f;
	uint8 regAdress = 0x28;
	writeReg(regAdress, value);
	_flags |= CHS_KEYOFF;
}

void TownsPC98_OpnChannelSSG::keyOn() {
	// all operators on
	uint8 value = _keyNum | 0xf0;
	uint8 regAdress = 0x28;
	writeReg(regAdress, value);
}

void TownsPC98_OpnChannelSSG::loadData(uint8 *data) {
	_drv->_ssgFlag = (_flags & CHS_SSG) ? -1 : 0;
	opn_SSG_UNK(0);
	TownsPC98_OpnChannel::loadData(data);
	_algorithm = 0x80;
}

void TownsPC98_OpnChannelSSG::opn_SSG_UNK(uint8 a) {
	_ssg1 = a;
	uint16 h = (_totalLevel + 1) * a;
	if ((h >> 8) == _ssg2)
		return;
	_ssg2 = (h >> 8);
	writeReg(8 + _regOffset, _ssg2);
}

TownsPC98_OpnDriver::TownsPC98_OpnDriver(Audio::Mixer *mixer, OpnType type) :
	_mixer(mixer), _trackData(0), _playing(false), _fading(false), _channels(0), _ssgChannels(0),
	_looping(0), _opnCarrier(_drvTables + 76), _opnFreqTable(_drvTables + 84),
	_opnFxCmdLen(_drvTables + 36), _opnLvlPresets(_drvTables + (type == OD_TOWNS ? 52 : 220)) ,
	_oprRates(0), _oprRateshift(0), _oprAttackDecay(0), _oprFrq(0),	_oprSinTbl(0), _oprLevelOut(0),
	_oprDetune(0), _cbCounter(4), _tickCounter(0), _updateChannelsFlag(type == OD_TYPE26 ? 0x07 : 0x3F),
	_finishedChannelsFlag(0), _samplesTillCallback(0), _samplesTillCallbackRemainder(0), _ready(false),
	_numSSG(type == OD_TOWNS ? 0 : 3), _hasADPCM(type == OD_TYPE86 ? true : false),
	_numChan(type == OD_TYPE26 ? 3 : 6), _hasStereo(type == OD_TYPE26 ? false : true) {	
	setTempo(84);
	_baserate = (3579545.0 / (double)getRate()) / 144.0;
}

TownsPC98_OpnDriver::~TownsPC98_OpnDriver() {
	_mixer->stopHandle(_soundHandle);

	if (_channels) {
		for (int i = 0; i < _numChan; i++)
			delete _channels[i];
		delete [] _channels;
	}

	if (_ssgChannels) {
		for (int i = 0; i < _numSSG; i++)
			delete _ssgChannels[i];
		delete [] _ssgChannels;
	}

	delete [] _oprRates;
	delete [] _oprRateshift;
	delete [] _oprFrq;
	delete [] _oprAttackDecay;
	delete [] _oprSinTbl;
	delete [] _oprLevelOut;
	delete [] _oprDetune;	
}

bool TownsPC98_OpnDriver::init() {
	if (_ready) {
		reset();
		return true;
	}

	generateTables();

	if (_channels) {
		for (int i = 0; i < _numChan; i++) {
			if (_channels[i])
				delete _channels[i];
		}
		delete [] _channels;
	}
	_channels = new TownsPC98_OpnChannel*[_numChan];
	for (int i = 0; i < _numChan; i++) {
		int ii = i * 6;
		_channels[i] = new TownsPC98_OpnChannel(this, _drvTables[ii], _drvTables[ii + 1],
			_drvTables[ii + 2],	_drvTables[ii + 3],	_drvTables[ii + 4], _drvTables[ii + 5]);
		_channels[i]->init();
	}

	if (_ssgChannels) {
		for (int i = 0; i < _numSSG; i++) {
			if (_ssgChannels[i])
				delete _ssgChannels[i];
		}
		delete [] _ssgChannels;
	}
	if (_numSSG) {
		_ssgChannels = new TownsPC98_OpnChannelSSG*[_numSSG];
		for (int i = 0; i < _numSSG; i++) {
			int ii = i * 6;
			_ssgChannels[i] = new TownsPC98_OpnChannelSSG(this, _drvTables[ii], _drvTables[ii + 1],
				_drvTables[ii + 2],	_drvTables[ii + 3],	_drvTables[ii + 4], _drvTables[ii + 5]);
			_ssgChannels[i]->init();
		}
	}

	_mixer->playInputStream(Audio::Mixer::kMusicSoundType,
		&_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, false, true);

	_ready = true;
	return true;
}

int inline TownsPC98_OpnDriver::readBuffer(int16 *buffer, const int numSamples) {
	memset(buffer, 0, sizeof(int16) * numSamples);
	int32 samplesLeft = numSamples >> 1;
	while (samplesLeft) {
		if (!_samplesTillCallback) {
			callback();
			_samplesTillCallback = _samplesPerCallback;
			_samplesTillCallbackRemainder += _samplesPerCallbackRemainder;
			if (_samplesTillCallbackRemainder >= _tempo) {
				_samplesTillCallback++;
				_samplesTillCallbackRemainder -= _tempo;
			}
		}

		int32 render = MIN(samplesLeft, _samplesTillCallback);
		samplesLeft -= render;
		_samplesTillCallback -= render;

		nextTick(buffer, render);

		for (int i = 0; i < render; ++i) {
			buffer[i << 1] <<= 2;
			buffer[(i << 1) + 1] <<= 2;
		}

		buffer += (render << 1);
	}

	return numSamples;
}

void TownsPC98_OpnDriver::loadData(uint8 *data, bool loadPaused) {
	if (!_ready) {
		warning("TownsPC98_OpnDriver: Driver must be initialized before loading data");
		return;
	}

	if (!data) {
		warning("TownsPC98_OpnDriver: Invalid music file data");
		return;
	}

	lock();
	_trackData = data;

	reset();
	
	uint8 *src_a = data;

	for (uint8 i = 0; i < 3; i++) {
		_channels[i]->loadData(data + READ_LE_UINT16(src_a));
		src_a += 2;
	}

	for (int i = 0; i < _numSSG; i++) {
		_ssgChannels[i]->loadData(data + READ_LE_UINT16(src_a));
		src_a += 2;
	}

	for (uint8 i = 3; i < _numChan; i++) {
		_channels[i]->loadData(data + READ_LE_UINT16(src_a));
		src_a += 2;
	}

	if (_hasADPCM) {
		//_adpcmChannel->loadData(data + READ_LE_UINT16(src_a));
		src_a += 2;
	}

	_ssgFlag = 0;

	_patches = src_a + 4;
	_cbCounter = 4;
	_finishedChannelsFlag = 0;

	// AH 0x17
	unlock();
	_playing = (loadPaused ? false : true);
}

void TownsPC98_OpnDriver::reset() {
	for (int i = 0; i < (_numChan); i++)
		_channels[i]->reset();
	for (int i = 0; i < (_numSSG); i++)
		_ssgChannels[i]->reset();

	_playing = _fading = false;
	_looping = 0;
	_tickCounter = 0;
}

void TownsPC98_OpnDriver::fadeOut() {
	if (!_playing)
		return;

	_fading = true;

	for (int i = 0; i < 20; i++) {		
		lock();
		uint32 dTime = _tickCounter + 2;
		for (int j = 0; j < _numChan; j++) {
			if (_updateChannelsFlag & _channels[j]->_idFlag)
				_channels[j]->fadeStep();
		}
		for (int j = 0; j < _numSSG; j++)
			_ssgChannels[j]->fadeStep();

		unlock();

		while (_playing) {
			if (_tickCounter >= dTime)
				break;
		}
	}

	_fading = false;

	reset();
}

void TownsPC98_OpnDriver::callback() {
	if (!_playing || --_cbCounter)
		return;

	_cbCounter = 4;
	_tickCounter++;

	lock();

	for (int i = 0; i < _numChan; i++) {
		if (_updateChannelsFlag & _channels[i]->_idFlag) {
			_channels[i]->processEvents();
			_channels[i]->processFrequency();
		}
	}

	if (_numSSG) {
		for (int i = 0; i < _numSSG; i++) {
			_ssgChannels[i]->processEvents();
			_ssgChannels[i]->processFrequency();
		}
	}
	
	_ssgFlag = 0;

	unlock();

	if (_finishedChannelsFlag == _updateChannelsFlag)
		reset();
}

void TownsPC98_OpnDriver::nextTick(int16 *buffer, uint32 bufferSize) {
	if (!_playing)
		return;	

	for (int i = 0; i < _numChan ; i++) {
		if (_channels[i]->_updateEnvelopes) {
			_channels[i]->_updateEnvelopes = false;
			_channels[i]->updateEnv();
		}
		
		for (uint32 ii = 0; ii < bufferSize ; ii++)
			_channels[i]->generateOutput(buffer[ii * 2],
			buffer[ii * 2 + 1],	&_channels[i]->_feedbuf[2], _channels[i]->_feedbuf);
	}

	for (int i = 0; i < _numSSG ; i++) {
		if (_ssgChannels[i]->_updateEnvelopes) {
			_ssgChannels[i]->_updateEnvelopes = false;
			_ssgChannels[i]->updateEnv();
		}
		
		for (uint32 ii = 0; ii < bufferSize ; ii++)
			_ssgChannels[i]->generateOutput(buffer[ii * 2],
			buffer[ii * 2 + 1],	&_ssgChannels[i]->_feedbuf[2], _ssgChannels[i]->_feedbuf);
	}
}

void TownsPC98_OpnDriver::generateTables() {
	delete [] _oprRates;
	_oprRates = new uint8[128];
	memset(_oprRates, 0x90, 32);
	uint8 *dst = (uint8*) _oprRates + 32;
	for (int i = 0; i < 48; i += 4)
		WRITE_BE_UINT32(dst + i, 0x00081018);
	dst += 48;
	for (uint8 i = 0; i < 16; i ++) {
		uint8 v = (i < 12) ? i : 12;
		*dst++ = ((4 + v) << 3);
	}
	memset(dst, 0x80, 32);

	delete [] _oprRateshift;
	_oprRateshift = new uint8[128];
	memset(_oprRateshift, 0, 128);
	dst = (uint8*) _oprRateshift + 32;
	for (int i = 11; i; i--) {
		memset(dst, i, 4);
		dst += 4;
	}

	delete [] _oprFrq;
	_oprFrq = new uint32[0x1000];
	for (uint32 i = 0; i < 0x1000; i++)
		_oprFrq[i] = (uint32)(_baserate * (double)(i << 11));

	delete [] _oprAttackDecay;
	_oprAttackDecay = new uint8[152];
	memset(_oprAttackDecay, 0, 152);
	for (int i = 0; i < 36; i++)
		WRITE_BE_UINT32(_oprAttackDecay + (i << 2), _adtStat[i]);

	delete [] _oprSinTbl;
	_oprSinTbl = new uint32[1024];
	for (int i = 0; i < 1024; i++) {
		double val = sin((double) (((i << 1) + 1) * PI / 1024.0));
		double d_dcb = log(1.0 / (double)ABS(val)) / log(2.0) * 256.0;
		int32 i_dcb = (int32)(2.0 * d_dcb);
		i_dcb = (i_dcb & 1) ? (i_dcb >> 1) + 1 : (i_dcb >> 1);
		_oprSinTbl[i] = (i_dcb << 1) + (val >= 0.0 ? 0 : 1);
	}

	delete [] _oprLevelOut;
	_oprLevelOut = new int32[0x1a00];
	for (int i = 0; i < 256; i++) {
		double val = floor(65536.0 / pow(2.0, 0.00390625 * (double)(1 + i)));
		int32 val_int = ((int32) val) >> 4;
		_oprLevelOut[i << 1] = (val_int & 1) ? ((val_int >> 1) + 1) << 2 : (val_int >> 1) << 2;
		_oprLevelOut[(i << 1) + 1] = -_oprLevelOut[i << 1];
		for (int ii = 1; ii < 13; ii++) {
			_oprLevelOut[(i << 1) + (ii << 9)] =  _oprLevelOut[i << 1] >> ii;
			_oprLevelOut[(i << 1) + (ii << 9) + 1] = -_oprLevelOut[(i << 1) + (ii << 9)];
		}
	}

	uint8 *dtt = new uint8[128];
	memset(dtt, 0, 36);
	memset(dtt + 36, 1, 8);
	memcpy(dtt + 44, _drvTables + 144, 84);

	delete [] _oprDetune;
	_oprDetune = new int32[256];
	for (int i = 0; i < 128; i++) {
		_oprDetune[i] = (int32)	((double)dtt[i] * _baserate * 64.0);
		_oprDetune[i + 128] = -_oprDetune[i];
	}

	delete [] dtt;
}

void TownsPC98_OpnDriver::setTempo(uint8 tempo) {
	_tempo = tempo;
	_samplesPerCallback = getRate() / _tempo;
	_samplesPerCallbackRemainder = getRate() % _tempo;
}

const uint8 TownsPC98_OpnDriver::_drvTables[] = {
	//	channel presets
	0x00, 0x80, 0x00, 0x00, 0x00, 0x01,
	0x01, 0x80, 0x01, 0x01, 0x00, 0x02,
	0x02, 0x80, 0x02, 0x02, 0x00, 0x04,
	0x00, 0x80, 0x03, 0x04, 0x01, 0x08,
	0x01, 0x80, 0x04, 0x05, 0x01, 0x10,
	0x02, 0x80, 0x05, 0x06, 0x01, 0x20,

	//	control event size
	0x01, 0x01, 0x01, 0x01,	0x01, 0x01, 0x04, 0x05,
	0x02, 0x06, 0x02, 0x00, 0x00, 0x02, 0x00, 0x02,

	//	fmt level presets 
	0x54, 0x50,	0x4C, 0x48,	0x44, 0x40, 0x3C, 0x38,
	0x34, 0x30, 0x2C, 0x28, 0x24, 0x20, 0x1C, 0x18,
	0x14, 0x10, 0x0C, 0x08,	0x04, 0x90, 0x90, 0x90,
	
	//	carriers
	0x08, 0x08, 0x08, 0x08,	0x0C, 0x0E, 0x0E, 0x0F,

	//	frequencies
	0x6A, 0x02, 0x8F, 0x02, 0xB6, 0x02,	0xDF, 0x02,
	0x0B, 0x03, 0x39, 0x03, 0x6A, 0x03, 0x9E, 0x03,
	0xD5, 0x03,	0x10, 0x04, 0x4E, 0x04, 0x8F, 0x04,
	0x00, 0x00, 0x00, 0x00,

	//	unused
	0x01, 0x00,	0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
	0x02, 0x00,	0x00, 0x00,	0x05, 0x00, 0x00, 0x00,
	0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,

	//	detune
	0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x01, 0x01,	0x01, 0x01,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x03,	0x03, 0x03,
	0x04, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07,
	0x08, 0x08, 0x09, 0x0a,	0x0b, 0x0c, 0x0d, 0x0e,
	0x10, 0x10, 0x10, 0x10,	0x02, 0x02, 0x02, 0x02,
	0x02, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05,
	0x05, 0x06,	0x06, 0x07, 0x08, 0x08, 0x09, 0x0a,
	0x0b, 0x0c,	0x0d, 0x0e, 0x10, 0x11, 0x13, 0x14,
	0x16, 0x16, 0x16, 0x16,

	//	pc98 level presets 
	0x40, 0x3B, 0x38, 0x34, 0x30, 0x2A, 0x28, 0x25,
	0x22, 0x20, 0x1D, 0x1A, 0x18, 0x15, 0x12, 0x10,
	0x0D, 0x0A, 0x08, 0x05, 0x02, 0x90, 0x90, 0x90
};

const uint32 TownsPC98_OpnDriver::_adtStat[] = {
	0x00010001, 0x00010001,	0x00010001, 0x01010001,
	0x00010101, 0x00010101, 0x00010101, 0x01010101,
	0x01010101, 0x01010101, 0x01010102, 0x01010102,
	0x01020102, 0x01020102, 0x01020202, 0x01020202,
	0x02020202, 0x02020202, 0x02020204, 0x02020204,
	0x02040204, 0x02040204, 0x02040404, 0x02040404,
	0x04040404, 0x04040404, 0x04040408, 0x04040408,
	0x04080408, 0x04080408, 0x04080808, 0x04080808,
	0x08080808, 0x08080808, 0x10101010, 0x10101010
};

SoundTowns::SoundTowns(KyraEngine_v1 *vm, Audio::Mixer *mixer)
	: Sound(vm, mixer), _lastTrack(-1), _currentSFX(0), _sfxFileData(0),
	_sfxFileIndex((uint)-1), _sfxWDTable(0), _sfxBTTable(0), _parser(0) {

	_driver = new Towns_EuphonyDriver(_mixer);
	int ret = open();
	if (ret != MERR_ALREADY_OPEN && ret != 0)
		error("couldn't open midi driver");
}

SoundTowns::~SoundTowns() {
	AudioCD.stop();
	haltTrack();
	delete[] _sfxFileData;

	Common::StackLock lock(_mutex);
	_driver->setTimerCallback(0, 0);
	close();

	_driver = 0;
}

bool SoundTowns::init() {
	_vm->checkCD();
	int unused = 0;
	_sfxWDTable = _vm->staticres()->loadRawData(k1TownsSFXwdTable, unused);
	_sfxBTTable = _vm->staticres()->loadRawData(k1TownsSFXbtTable, unused);

	return loadInstruments();
}

void SoundTowns::process() {
	AudioCD.updateCD();
}

void SoundTowns::playTrack(uint8 track) {
	if (track < 2)
		return;
	track -= 2;

	const int32 *const tTable = (const int32 *const) cdaData();
	int tTableIndex = 3 * track;

	int trackNum = (int) READ_LE_UINT32(&tTable[tTableIndex + 2]);
	int32 loop = (int32) READ_LE_UINT32(&tTable[tTableIndex + 1]);

	if (track == _lastTrack && _musicEnabled)
		return;

	beginFadeOut();

	if (_musicEnabled == 2 && trackNum != -1) {
		AudioCD.play(trackNum+1, loop ? -1 : 1, 0, 0);
		AudioCD.updateCD();
	} else if (_musicEnabled) {
		playEuphonyTrack(READ_LE_UINT32(&tTable[tTableIndex]), loop);
	}

	_lastTrack = track;
}

void SoundTowns::haltTrack() {
	_lastTrack = -1;
	AudioCD.stop();
	AudioCD.updateCD();
	if (_parser) {
		Common::StackLock lock(_mutex);
		_parser->setTrack(0);
		_parser->jumpToTick(0);
		_parser->unloadMusic();
		delete _parser;
		_parser = 0;
	}
	_driver->queue()->release();
}

void SoundTowns::loadSoundFile(uint file) {
	if (_sfxFileIndex == file)
		return;
	_sfxFileIndex = file;
	delete[] _sfxFileData;
	_sfxFileData = _vm->resource()->fileData(fileListEntry(file), 0);
}

void SoundTowns::playSoundEffect(uint8 track) {
	if (!_sfxEnabled || !_sfxFileData)
		return;

	if (track == 0 || track == 10) {
		_mixer->stopHandle(_sfxHandle);
		return;
	} else if (track == 1) {
		// sfx fadeout
		_mixer->stopHandle(_sfxHandle);
		return;
	}

	uint8 note = 0x3c;
	if (_sfxFileIndex == 5) {
		if (track == 0x10) {
			note = 0x3e;
			track = 0x0f;
		} else if (track == 0x11) {
			note = 0x40;
			track = 0x0f;
		} else if (track == 0x12) {
			note = 0x41;
			track = 0x0f;
		}
	}

	uint8 *fileBody = _sfxFileData + 0x01b8;
	int32 offset = (int32)READ_LE_UINT32(_sfxFileData + (track - 0x0b) * 4);
	if (offset == -1)
		return;

	uint32 *sfxHeader = (uint32*)(fileBody + offset);

	uint32 sfxHeaderID = READ_LE_UINT32(sfxHeader);
	uint32 sfxHeaderInBufferSize = READ_LE_UINT32(&sfxHeader[1]);
	uint32 sfxHeaderOutBufferSize = READ_LE_UINT32(&sfxHeader[3]);
	uint32 sfxRootNoteOffs = READ_LE_UINT32(&sfxHeader[7]);
	uint32 sfxRate = READ_LE_UINT32(&sfxHeader[6]);

	uint32 playbackBufferSize = (sfxHeaderID == 1) ? sfxHeaderInBufferSize : sfxHeaderOutBufferSize;

	uint8 *sfxPlaybackBuffer = (uint8 *)malloc(playbackBufferSize);
	memset(sfxPlaybackBuffer, 0x80, playbackBufferSize);

	uint8 *sfxBody = ((uint8 *)sfxHeader) + 0x20;

	if (!sfxHeaderID) {
		memcpy(sfxPlaybackBuffer, sfxBody, playbackBufferSize);
	} else if (sfxHeaderID == 1) {
		Screen::decodeFrame4(sfxBody, sfxPlaybackBuffer, playbackBufferSize);
	} else if (_sfxWDTable) {
		uint8 *tgt = sfxPlaybackBuffer;
		uint32 sfx_BtTable_Offset = 0;
		uint32 sfx_WdTable_Offset = 0;
		uint32 sfx_WdTable_Number = 5;

		for (uint32 i = 0; i < sfxHeaderInBufferSize; i++) {
			sfx_WdTable_Offset = (sfx_WdTable_Number * 3 << 9) + sfxBody[i] * 6;
			sfx_WdTable_Number = READ_LE_UINT16(_sfxWDTable + sfx_WdTable_Offset);

			sfx_BtTable_Offset += (int16)READ_LE_UINT16(_sfxWDTable + sfx_WdTable_Offset + 2);
			*tgt++ = _sfxBTTable[((sfx_BtTable_Offset >> 2) & 0xff)];

			sfx_BtTable_Offset += (int16)READ_LE_UINT16(_sfxWDTable + sfx_WdTable_Offset + 4);
			*tgt++ = _sfxBTTable[((sfx_BtTable_Offset >> 2) & 0xff)];
		}
	}

	for (uint32 i = 0; i < playbackBufferSize; i++) {
		if (sfxPlaybackBuffer[i] < 0x80)
			sfxPlaybackBuffer[i] = 0x80 - sfxPlaybackBuffer[i];
	}

	playbackBufferSize -= 0x20;
	uint32 outputRate = uint32(11025 * semitoneAndSampleRate_to_sampleStep(note, sfxRootNoteOffs, sfxRate, 11025, 0x2000));

	_currentSFX = Audio::makeLinearInputStream(sfxPlaybackBuffer, playbackBufferSize,
		outputRate, Audio::Mixer::FLAG_UNSIGNED | Audio::Mixer::FLAG_LITTLE_ENDIAN | Audio::Mixer::FLAG_AUTOFREE, 0, 0);
	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, _currentSFX);
}

void SoundTowns::beginFadeOut() {
	_lastTrack = -1;
	_driver->fading();

	// TODO: this should fade out too
	AudioCD.stop();
	AudioCD.updateCD();
}

int SoundTowns::open() {
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;

	_driver->setTimerCallback(this, &onTimer);
	return 0;
}

void SoundTowns::close() {
	if (_driver)
		_driver->close();
}

void SoundTowns::send(uint32 b) {
	_driver->send(b);
}

uint32 SoundTowns::getBaseTempo(void) {
	return _driver ? _driver->getBaseTempo() : 0;
}

bool SoundTowns::loadInstruments() {
	uint8 *twm = _vm->resource()->fileData("twmusic.pak", 0);
	if (!twm)
		return false;
	_driver->queue()->loadDataToCurrentPosition(twm, 0x8BF0);
	_driver->loadFmInstruments(_driver->queue()->trackData() + 8);

	_driver->queue()->loadDataToCurrentPosition(twm + 0x0CA0, 0xC58A);
	_driver->loadWaveInstruments(_driver->queue()->trackData() + 8);
	delete[] twm;
	_driver->queue()->release();

	return true;
}

void SoundTowns::playEuphonyTrack(uint32 offset, int loop) {
	uint8 *twm = _vm->resource()->fileData("twmusic.pak", 0);
	Common::StackLock lock(_mutex);

	if (!_parser) {
		_parser = new Towns_EuphonyParser(_driver->queue());
		_parser->setMidiDriver(this);
		_parser->setTimerRate(getBaseTempo());
	}

	_parser->property(MidiParser::mpAutoLoop, loop);
	_parser->loadMusic(twm + 0x4b70 + offset, 0xC58A);

	delete[] twm;
}

void SoundTowns::onTimer(void *data) {
	SoundTowns *music = (SoundTowns *)data;
	Common::StackLock lock(music->_mutex);
	if (music->_parser)
		music->_parser->onTimer();
}

float SoundTowns::semitoneAndSampleRate_to_sampleStep(int8 semiTone, int8 semiToneRootkey,
	uint32 sampleRate, uint32 outputRate, int32 pitchWheel) {
	if (semiTone < 0)
		semiTone = 0;
	if (semiTone > 119)
		semiTone = 119;
	if (semiTone < 0)
		semiTone = 0;
	if (semiTone > 119)
		semiTone = 119;

	static const float noteFrq[] = {
		0004.13f, 0004.40f, 0004.64f, 0004.95f, 0005.16f, 0005.50f, 0005.80f, 0006.19f, 0006.60f, 0006.86f,
		0007.43f, 0007.73f, 0008.25f, 0008.80f, 0009.28f, 0009.90f, 0010.31f, 0011.00f, 0011.60f, 0012.38f,
		0013.20f, 0013.75f, 0014.85f, 0015.47f,	0016.50f, 0017.60f, 0018.56f, 0019.80f, 0020.63f, 0022.00f,
		0023.21f, 0024.75f, 0026.40f, 0027.50f, 0029.70f, 0030.94f, 0033.00f, 0035.20f, 0037.16f, 0039.60f,
		0041.25f, 0044.00f, 0046.41f, 0049.50f, 0052.80f, 0055.00f, 0059.40f, 0061.88f, 0066.00f, 0070.40f,
		0074.25f, 0079.20f, 0082.50f, 0088.00f, 0092.83f, 0099.00f, 0105.60f, 0110.00f, 0118.80f, 0123.75f,
		0132.00f, 0140.80f, 0148.50f, 0158.40f, 0165.00f, 0176.00f, 0185.65f, 0198.00f, 0211.20f, 0220.00f,
		0237.60f, 0247.50f, 0264.00f, 0281.60f, 0297.00f, 0316.80f, 0330.00f, 0352.00f, 0371.30f, 0396.00f,
		0422.40f, 0440.00f, 0475.20f, 0495.00f,	0528.00f, 0563.20f, 0594.00f, 0633.60f, 0660.00f, 0704.00f,
		0742.60f, 0792.00f, 0844.80f, 0880.00f, 0950.40f, 0990.00f, 1056.00f, 1126.40f, 1188.00f, 1267.20f,
		1320.00f, 1408.00f, 1485.20f, 1584.00f, 1689.60f, 1760.00f, 1900.80f, 1980.00f, 2112.00f, 2252.80f,
		2376.00f, 2534.40f, 2640.00f, 2816.00f, 2970.40f, 3168.00f, 3379.20f, 3520.00f, 3801.60f, 3960.00f
	};

	float pwModifier = (pitchWheel - 0x2000) / 0x2000;
	int8 d = pwModifier ? (pwModifier < 0 ? -1 : 1) : 0;
	float rateshift = (noteFrq[semiTone] - ((noteFrq[semiTone] -
		noteFrq[semiTone + d]) * pwModifier * d)) / noteFrq[semiToneRootkey];

	return (float) sampleRate * 10.0f * rateshift / outputRate;
}

SoundPC98::SoundPC98(KyraEngine_v1 *vm, Audio::Mixer *mixer) :
	Sound(vm, mixer), _musicTrackData(0), _sfxTrackData(0), _lastTrack(-1), _driver(0) {
}

SoundPC98::~SoundPC98() {
	delete[] _musicTrackData;
	delete[] _sfxTrackData;
	delete _driver;
}

bool SoundPC98::init() {
	_driver = new TownsPC98_OpnDriver(_mixer, TownsPC98_OpnDriver::OD_TYPE26);
	_sfxTrackData = _vm->resource()->fileData("se.dat", 0);
	if (!_sfxTrackData)
		return false;
	return _driver->init();
}

void SoundPC98::playTrack(uint8 track) {
	if (--track >= 56)
		track -= 55;
 
	if (track == _lastTrack && _musicEnabled)
		return;

	haltTrack();

	char musicfile[13];
	sprintf(musicfile, fileListEntry(0), track);
	delete[] _musicTrackData;
	_musicTrackData = _vm->resource()->fileData(musicfile, 0);
	if (_musicEnabled)
		_driver->loadData(_musicTrackData);

	_lastTrack = track;
}

void SoundPC98::haltTrack() {
	_lastTrack = -1;
	AudioCD.stop();
	AudioCD.updateCD();
	_driver->reset();
}

void SoundPC98::beginFadeOut() {
	_driver->fadeOut();
	haltTrack();
}

void SoundPC98::playSoundEffect(uint8) {
	/// TODO ///
}


//	KYRA 2

SoundTownsPC98_v2::SoundTownsPC98_v2(KyraEngine_v1 *vm, Audio::Mixer *mixer) :
	Sound(vm, mixer), _currentSFX(0), _musicTrackData(0), _lastTrack(-1), _driver(0), _useFmSfx(false) {
}

SoundTownsPC98_v2::~SoundTownsPC98_v2() {
	delete[] _musicTrackData;
	delete _driver;
}

bool SoundTownsPC98_v2::init() {
	_driver = new TownsPC98_OpnDriver(_mixer, _vm->gameFlags().platform == Common::kPlatformPC98 ?
		TownsPC98_OpnDriver::OD_TYPE86 : TownsPC98_OpnDriver::OD_TOWNS);
	_useFmSfx = _vm->gameFlags().platform == Common::kPlatformPC98 ? true : false;
	_vm->checkCD();
	// FIXME: While checking for 'track1.XXX(X)' looks like
	// a good idea, we should definitely not be doing this
	// here. Basically our filenaming scheme could change
	// or we could add support for other audio formats. Also
	// this misses the possibility that we play the tracks
	// right off CD. So we should find another way to
	// check if we have access to CD audio.
	if (_musicEnabled &&
		(Common::File::exists("track1.mp3") || Common::File::exists("track1.ogg") ||
		 Common::File::exists("track1.flac") || Common::File::exists("track1.fla")))
			_musicEnabled = 2;
	return _driver->init();
}

void SoundTownsPC98_v2::process() {
	AudioCD.updateCD();
}

void SoundTownsPC98_v2::playTrack(uint8 track) {
	if (track == _lastTrack && _musicEnabled)
		return;

	const uint16 *const cdaTracks = (const uint16 *const) cdaData();

	int trackNum = -1;
	if (_vm->gameFlags().platform == Common::kPlatformFMTowns) {
		for (int i = 0; i < cdaTrackNum(); i++) {
			if (track == (uint8) READ_LE_UINT16(&cdaTracks[i * 2])) {
				trackNum = (int) READ_LE_UINT16(&cdaTracks[i * 2 + 1]) - 1;
				break;
			}
		}
	}

	beginFadeOut();

	char musicfile[13];
	sprintf(musicfile, fileListEntry(0), track);
	delete[] _musicTrackData;
	
	_musicTrackData = _vm->resource()->fileData(musicfile, 0);
	_driver->loadData(_musicTrackData, true);

	if (_musicEnabled == 2 && trackNum != -1) {
		AudioCD.play(trackNum+1, _driver->looping() ? -1 : 1, 0, 0);
		AudioCD.updateCD();
	} else if (_musicEnabled) {
		_driver->cont();
	}

	_lastTrack = track;
}

void SoundTownsPC98_v2::haltTrack() {
	_lastTrack = -1;
	AudioCD.stop();
	AudioCD.updateCD();
	_driver->reset();
}

void SoundTownsPC98_v2::beginFadeOut() {
	_driver->fadeOut();
	haltTrack();
}

int32 SoundTownsPC98_v2::voicePlay(const char *file, bool) {
	static const uint16 rates[] =	{ 0x10E1, 0x0CA9, 0x0870, 0x0654, 0x0438, 0x032A, 0x021C, 0x0194 };

	int h = 0;
	if (_currentSFX) {
		while (_mixer->isSoundHandleActive(_soundChannels[h].channelHandle) && h < kNumChannelHandles)
			h++;
		if (h >= kNumChannelHandles)
			return 0;
	}

	char filename[13];
	sprintf(filename, "%s.PCM", file);

	uint8 *data = _vm->resource()->fileData(filename, 0);
	uint8 *src = data;

	uint16 sfxRate = rates[READ_LE_UINT16(src)];
	src += 2;
	bool compressed = (READ_LE_UINT16(src) & 1) ? true : false;
	src += 2;
	uint32 outsize = READ_LE_UINT32(src);
	uint8 *sfx = (uint8*) malloc(outsize);
	uint8 *dst = sfx;
	src += 4;

	if (compressed) {
		for (uint32 i = outsize; i;) {
			uint8 cnt = *src++;
			if (cnt & 0x80) {
				cnt &= 0x7F;
				memset(dst, *src++, cnt);
			} else {
				memcpy(dst, src, cnt);
				src += cnt;
			}
			dst += cnt;
			i -= cnt;
		}
	} else {
		memcpy(dst, src, outsize);
	}

	for (uint32 i = 0; i < outsize; i++) {
		uint8 cmd = sfx[i];
		if (cmd & 0x80) {
			cmd = ~cmd;
		} else {
			cmd |= 0x80;
			if (cmd == 0xff)
				cmd--;
		}
		if (cmd < 0x80)
			cmd = 0x80 - cmd;
		sfx[i] = cmd;
	}

	uint32 outputRate = uint32(11025 * SoundTowns::semitoneAndSampleRate_to_sampleStep(0x3c, 0x3c, sfxRate, 11025, 0x2000));

	_currentSFX = Audio::makeLinearInputStream(sfx, outsize, outputRate,
		Audio::Mixer::FLAG_UNSIGNED | Audio::Mixer::FLAG_LITTLE_ENDIAN | Audio::Mixer::FLAG_AUTOFREE, 0, 0);
	_soundChannels[h].file = file;
	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_soundChannels[h].channelHandle, _currentSFX);

	delete[] data;
	return 1;
}

void SoundTownsPC98_v2::playSoundEffect(uint8 track) {
	if (!_useFmSfx)
		return;

	uint8 *sd = _vm->resource()->fileData("sound.dat", 0);


	//TODO

	delete [] sd;
}

} // end of namespace Kyra

#undef EUPHONY_FADEOUT_TICKS

