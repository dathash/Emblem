// Author: Alex Hartford
// Program: Emblem
// File: Main

#ifndef INPUT_H
#define INPUT_H

// Handles all SDL Events, including key presses.
void
HandleEvents(InputState *input, SDL_Joystick *gamepad) 
{
    if(input->joystickCooldown)
    {
        --input->joystickCooldown;
    }
    SDL_Event Event;
    while(SDL_PollEvent(&Event))
    {
        if(Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_TAB)
        {
            GlobalEditorMode = !GlobalEditorMode;
            return;
        }
        if(Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_BACKQUOTE)
        {
            GlobalDebug = !GlobalDebug;
            return;
        }
        if(Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_1)
        {
            GlobalSpeedMod = 1;
            return;
        }
        if(Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_2)
        {
            GlobalSpeedMod = 2;
            return;
        }
        if(Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_3)
        {
            GlobalSpeedMod = 3;
            return;
        }
        if(Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_4)
        {
            GlobalSpeedMod = 4;
            return;
        }
        if(Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_5)
        {
            GlobalSpeedMod = 5;
            return;
        }
        if(Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_0)
        {
            GlobalPaused = !GlobalPaused;
            return;
        }
        if(Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_EQUALS)
        {
            if(GlobalPaused)
                GlobalStep = true;
            return;
        }
        if(Event.type == SDL_QUIT || (Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_ESCAPE))
        {
            GlobalRunning = false;
        }

        if(GlobalEditorMode)
        {
// ================================= editor inputs =============================
            ImGui_ImplSDL2_ProcessEvent(&Event);
            return;
        }
        else
        {
            ImGui_ImplSDL2_ProcessEvent(&Event);
// ================================= gameplay inputs ===========================
            if(Event.type == SDL_KEYDOWN)
            {
                switch(Event.key.keysym.sym)
                {
                    case SDLK_SPACE:
                        input->a = true;
                        break;
                    case SDLK_RSHIFT:
                        input->l = true;
                        break;
                    case SDLK_q:
                        input->b = true;
                        break;
                    case SDLK_e:
                        input->r = true;
                        break;
                    case SDLK_w:
                        input->up = true;
                        break;
                    case SDLK_s:
                        input->down = true;
                        break;
                    case SDLK_a:
                        input->left = true;
                        break;
                    case SDLK_d:
                        input->right = true;
                        break;
                }
            }
            else if(Event.type == SDL_KEYUP)
            {
                switch(Event.key.keysym.sym)
                {
                    case SDLK_SPACE:
                        input->a = false;
                        break;
                    case SDLK_RSHIFT:
                        input->l = false;
                        break;
                    case SDLK_q:
                        input->b = false;
                        break;
                    case SDLK_e:
                        input->r = false;
                        break;
                    case SDLK_w:
                        input->up = false;
                        break;
                    case SDLK_s:
                        input->down = false;
                        break;
                    case SDLK_a:
                        input->left = false;
                        break;
                    case SDLK_d:
                        input->right = false;
                        break;
                }
            }
// ======================================= gamepad =============================
            else if(Event.type == SDL_JOYAXISMOTION)
            {
                if(Event.jaxis.which == 0)
                {
                    if(Event.jaxis.axis == 0)
                    {
                        if(Event.jaxis.value < -JOYSTICK_DEAD_ZONE)
                        {
                            input->left = true;
                        }
                        else if(Event.jaxis.value > JOYSTICK_DEAD_ZONE)
                        {
                            input->right = true;
                        }
                        else
                        {
                            input->left = false;
                            input->right = false;
                        }
                    }

                    if(Event.jaxis.axis == 1)
                    {
                        if(Event.jaxis.value < -JOYSTICK_DEAD_ZONE)
                        {
                            input->up = true;
                        }
                        else if(Event.jaxis.value > JOYSTICK_DEAD_ZONE)
                        {
                            input->down = true;
                        }
                        else
                        {
                            input->up = false;
                            input->down = false;
                        }
                    }
                }
            }
            else if(Event.type == SDL_JOYBUTTONDOWN)
            {
                if(Event.jbutton.button == 0)
                    input->a = true;
                else if(Event.jbutton.button == 1)
                    input->b = true;
                else if(Event.jbutton.button == 11)
                    input->l = true;
                else if(Event.jbutton.button == 10)
                    input->r = true;
            }
            else if(Event.type == SDL_JOYBUTTONUP)
            {
                if(Event.jbutton.button == 0)
                    input->a = false;
                else if(Event.jbutton.button == 1)
                    input->b = false;
                else if(Event.jbutton.button == 9)
                    input->l = false;
                else if(Event.jbutton.button == 10)
                    input->r = false;
            }
            else if(Event.type == SDL_JOYDEVICEADDED)
            {
                printf("Gamepad connected!\n");
                gamepad = SDL_JoystickOpen(0);
                SDL_assert(gamepad);
            }
            else if(Event.type == SDL_JOYDEVICEREMOVED)
            {
                printf("Gamepad removed!\n");
                gamepad = NULL;
            }
        }
    }
}

#endif
