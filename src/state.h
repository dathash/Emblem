// Author: Alex Hartford
// Program: Emblem
// File: State

#ifndef STATE_H
#define STATE_H

// This file contains functions which pertain to overall game state.
// This includes
// * whose turn it is
// * Whether we just started a turn or not
// * What animations, if any, are playing.
// * What to do if we're editing the stage.

// Function called by commands to end the player's turn.
void
EndPlayerTurn()
{
    GlobalInterfaceState = NO_OP;
    GlobalPlayerTurn = false;
    GlobalTurnStart = true;
}

void
EndAITurn()
{
    GlobalAIState = PLAYER_TURN;
    GlobalPlayerTurn = true;
    GlobalTurnStart = true;
}

void
NextLevel()
{
    GlobalNextLevel = true;
    GlobalTurnStart = true;
}

void
RestartLevel()
{
    GlobalRestart = true;
}

void
ReloadLevel()
{
    GlobalLevelTimer = Timer(LEVEL_TIME);
    GlobalAIState = PLAYER_TURN;
    GlobalPlayerTurn = true;
    GlobalTurnStart = true;
}


#endif
