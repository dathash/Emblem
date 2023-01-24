// Author: Alex Hartford
// Program: Emblem
// File: Audio

#ifndef AUDIO_H
#define AUDIO_H

static ma_engine GlobalAudioEngine;
static ma_sound_group GlobalMusicGroup;
static ma_sound_group GlobalSfxGroup;

// Plays a song with miniaudio.
void
PlayMusic(const string &filename)
{
    //ma_sound_set_looping();
    ma_engine_play_sound(&GlobalAudioEngine, 
                         (MUSIC_PATH + filename).c_str(),
                         &(GlobalMusicGroup));
}

// Plays a sound effect with miniaudio.
void
PlaySfx(const string &filename)
{
    ma_engine_play_sound(&GlobalAudioEngine, 
                         (SFX_PATH + filename).c_str(),
                         &(GlobalSfxGroup));
}

void
SetMusicVolume(float volume)
{
    ma_sound_group_set_volume(&(GlobalMusicGroup), volume);
}
void
SetSfxVolume(float volume)
{
    ma_sound_group_set_volume(&(GlobalSfxGroup), volume);
}

struct AudioBank
{
    vector<ma_sound> sounds;
};
static AudioBank GlobalMusic;
static AudioBank GlobalSfx;

#endif
