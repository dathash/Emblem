// Author: Alex Hartford
// Program: Emblem
// File: State

#ifndef STATE_H
#define STATE_H

void GoToAIPhase() {
    GlobalPhase = PHASE_AI;
    GlobalInterfaceState = NO_OP;
    GlobalAIState = AI_FINDING_NEXT;

    EmitEvent(END_TURN_EVENT);
}

void GoToPlayerPhase(Level *level, Cursor *cursor) {
    GlobalPhase = PHASE_PLAYER;
    GlobalInterfaceState = NEUTRAL_OVER_UNIT;
    GlobalAIState = AI_NO_OP;

    cursor->selected = nullptr;
    cursor->PlaceAt(level->Leader());

    for(auto const &unit : level->combatants)
        unit->Activate();

    EmitEvent(END_TURN_EVENT);
}

void GoToResolutionPhase() {
    GlobalPhase = PHASE_RESOLUTION;
    GlobalInterfaceState = NO_OP;
    GlobalAIState = AI_NO_OP;

    EmitEvent(END_TURN_EVENT);
}

void GoToSpawningPhase() {
    GlobalPhase = PHASE_SPAWNING;
    GlobalInterfaceState = NO_OP;
    GlobalAIState = AI_NO_OP;

    EmitEvent(END_TURN_EVENT);
}

void GameOver() {
    GlobalPhase = PHASE_PLAYER;
    GlobalAIState = AI_NO_OP;
    GlobalInterfaceState = GAME_OVER;
}
void Victory() {
    GlobalPhase = PHASE_PLAYER;
    GlobalAIState = AI_NO_OP;
    GlobalInterfaceState = VICTORY;
}

#endif
