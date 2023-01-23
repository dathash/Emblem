// Author: Alex Hartford
// Program: Emblem
// File: Audio

#ifndef AUDIO_H
#define AUDIO_H

struct AudioBank
{
    vector<ma_sound> sounds;
    ma_sound_group group;
};

static ma_engine GlobalAudioEngine;
static AudioBank GlobalMusic;
static AudioBank GlobalSfx;

// TODO: Keep sounds in a bank.
// This currently calls new and delete every time we play a sound.
// Check the Miniaudio docs.
// We will want some kind of lookup table for the sound vectors.

// Plays a song with miniaudio.
void
PlayMusic(const string &filename)
{
    //ma_sound_set_looping();
    ma_engine_play_sound(&GlobalAudioEngine, 
                         (MUSIC_PATH + filename).c_str(),
                         &(GlobalMusic.group));
}

// Plays a sound effect with miniaudio.
void
PlaySfx(const string &filename)
{
    ma_engine_play_sound(&GlobalAudioEngine, 
                         (SFX_PATH + filename).c_str(),
                         &(GlobalSfx.group));
}

void
SetMusicVolume(float volume)
{
    ma_sound_group_set_volume(&(GlobalMusic.group), volume);
}
void
SetSfxVolume(float volume)
{
    ma_sound_group_set_volume(&(GlobalSfx.group), volume);
}

#endif
