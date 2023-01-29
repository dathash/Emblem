// Author: Alex Hartford
// Program: Emblem
// File: Audio

#ifndef AUDIO_H
#define AUDIO_H

static ma_engine GlobalAudioEngine;
static ma_sound_group GlobalMusicGroup;
static ma_sound_group GlobalSfxGroup;

enum AudioType
{
    MUSIC,
    SFX,
};

struct Sound
{
    ma_sound sound;
    string name;


    Sound(const string &name_in, AudioType type_in)
    : name(name_in)
    {
        switch(type_in)
        {
            case MUSIC:
            {
        ma_sound_init_from_file(&GlobalAudioEngine, 
                                (MUSIC_PATH + name_in).c_str(),
                                MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, 
                                &GlobalMusicGroup, NULL, &sound);
                ma_sound_set_looping(&sound, true);
            } break;
            case SFX:
            {
        ma_sound_init_from_file(&GlobalAudioEngine, 
                                (SFX_PATH + name_in).c_str(),
                                MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, 
                                &GlobalSfxGroup, NULL, &sound);
            } break;
            default:
            {
                SDL_assert(!"Shouldn't get here!\n");
            } break;
        }
    }

    ~Sound()
    {
        ma_sound_uninit(&sound);
    }

    void
    Pause()
    {
        ma_sound_stop(&sound);
    }

    void
    Stop()
    {
        ma_sound_stop(&sound);
        ma_sound_seek_to_pcm_frame(&sound, 0);
    }

    void
    Start()
    {
        ma_sound_start(&sound);
    }
};


///////////////////////////////////////////////////////////////////////////
struct AudioBank
{
    vector<Sound *> sounds;
};
static AudioBank GlobalMusic;
static AudioBank GlobalSfx;

Sound *
GetMusic(const string &name)
{
    for(Sound *sound : GlobalMusic.sounds)
        if(sound->name == name)
            return sound;
    SDL_assert(!"ERROR GetMusic: Song name doesn't exist\n");
    return nullptr;
}


void
PauseMusic(const string &name)
{
    for(Sound *sound : GlobalMusic.sounds)
        if(sound->name == name)
            sound->Pause();
}

void
StopMusic(const string &name)
{
    for(Sound *sound : GlobalMusic.sounds)
        if(sound->name == name)
            sound->Stop();
}

void
StartMusic(const string &name)
{
    for(Sound *sound : GlobalMusic.sounds)
        if(sound->name == name)
            sound->Start();
}

void
PlaySfx(const string &name)
{
    for(Sound *sound : GlobalSfx.sounds)
        if(sound->name == name)
        {
            sound->Stop();
            sound->Start();
        }
}


///////////////////////////////////////////////////////////////////////////
/*
    NOTE: THIS IS THE OLD INTERFACE.

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
*/

///////////////////////////////////////////////////////////////////////////

static Sound *GlobalSong;

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

#endif
