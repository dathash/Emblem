// Author: Alex Hartford
// Program: Emblem
// File: State

#ifndef STATE_H
#define STATE_H

void
GoToAIPhase()
{
    GlobalPhase = PHASE_AI;
    GlobalInterfaceState = NO_OP;
    GlobalAIState = AI_FINDING_NEXT;

    EmitEvent(END_TURN_EVENT);
}

void
GoToPlayerPhase(Level *level, Cursor *cursor)
{
    GlobalPhase = PHASE_PLAYER;
    GlobalInterfaceState = NEUTRAL_UNIT;
    GlobalAIState = AI_NO_OP;

    cursor->selected = nullptr;
    cursor->PlaceAt(level->Leader());

    ++level->turn_count;

    for(auto const &unit : level->combatants)
        unit->Activate();

    EmitEvent(END_TURN_EVENT);
}

void
GameOver()
{
    GlobalPhase = PHASE_PLAYER;
    GlobalAIState = AI_NO_OP;
    GlobalInterfaceState = GAME_OVER;
}

#endif
