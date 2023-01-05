
// Author: Alex Hartford
// Program: Emblem
// File: Main
// Date: July 2022

#ifndef INPUT_H
#define INPUT_H

// Handles all SDL Events, including key presses.
void
HandleEvents(InputState *input)
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
        if(Event.type == SDL_QUIT || (Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_ESCAPE))
        {
            printf("Quit Received!\n");
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
                    case SDLK_LSHIFT:
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
                    case SDLK_LSHIFT:
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
            else if(Event.type == SDL_JOYBUTTONDOWN || Event.type == SDL_JOYBUTTONUP)
            {
                if(Event.jbutton.button == 0)
                {
                    if(Event.type == SDL_JOYBUTTONDOWN)
                    {
                        input->a = true;
                    }
                    else if(Event.type == SDL_JOYBUTTONUP)
                    {
                        input->a = false;
                    }
                }
                if(Event.jbutton.button == 1)
                {
                    if(Event.type == SDL_JOYBUTTONDOWN)
                    {
                        input->b = true;
                    }
                    else if(Event.type == SDL_JOYBUTTONUP)
                    {
                        input->b = false;
                    }
                }
            }
            else if(Event.type == SDL_JOYDEVICEADDED)
            {
                printf("Gamepad connected!\n");
                GlobalGamepadMode = true;
            }
            else if(Event.type == SDL_JOYDEVICEREMOVED)
            {
                printf("Gamepad removed!\n");
                GlobalGamepadMode = false;
            }
        }
    }
}

#endif
