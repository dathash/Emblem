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

void GoToPlayerPhase(Level *level, 
                     Level *backup, 
                     const Resolution &res,
                     Resolution *res_backup, 
                     Cursor *cursor)
{
    GlobalPhase = PHASE_PLAYER;
    GlobalInterfaceState = NEUTRAL;
    GlobalAIState = AI_NO_OP;

    cursor->selected = nullptr;
    cursor->PlaceAt(level->FirstAlly());

    for(auto const &unit : level->combatants)
        unit->Activate();

    EmitEvent(END_TURN_EVENT);

    // set up backup for undo turn.
    *backup = Level(*level);

    res_backup->incidents = {};
    for(const Incident &incident : res.incidents)
    {
        Unit *old_ptr = incident.unit;
        Unit *new_ptr = backup->map.tiles[old_ptr->pos.col][old_ptr->pos.row].occupant;
        res_backup->incidents.push_back({new_ptr, incident.weapon, incident.offset, incident.type});
    }
    GlobalPlayer.backup_health = GlobalPlayer.health;
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
