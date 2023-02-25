// Author: Alex Hartford
// Program: Emblem
// File: Mission

#ifndef MISSION_H
#define MISSION_H

// Mission_Armored_Train = Mission_Train:new{
//     Objectives = Objective("Mission_ArmoredTrain_Obj",2),
//     ObjectiveId = "Mission_ArmoredTrain_Obj",
//     TrainPawn = "Train_Armored",
//     TrainDamaged = "Train_Armored_Damaged",
//     SpawnMod = 1,
// }

// BONUS_ASSET = 1
// BONUS_KILL = 2
// BONUS_GRID = 3
// BONUS_MECHS = 4
// BONUS_BLOCK = 5
// BONUS_KILL_FIVE = 6
// BONUS_DEBRIS = 7
// BONUS_SELFDAMAGE = 8
// BONUS_PACIFIST = 9
// 
// function PowerObjective(text, value, potential)
// 	potential = potential or value
// 	local ret = Objective(text,value,potential)
// 	ret.category = REWARD_POWER
// 	return ret
// end
// 
// function PowerObjectiveExt(text, param1, param2, value, potential)
// 	potential = potential or value
// 	local ret = Objective(text,param1,param2,value,potential)
// 	ret.category = REWARD_POWER
// 	return ret
// end
// 
// Mission = 	{
// 	Name = "Mission",
// 	MapList = {},
// 	InfiniteSpawn = false,
// 	BlockCracks = false,
// 	Spawner = nil,
// 	Objectives = { },
// 	BonusObjs = { },
// 	BonusPool = { BONUS_KILL_FIVE, BONUS_GRID, BONUS_MECHS, BONUS_DEBRIS, BONUS_PACIFIST, BONUS_SELFDAMAGE },
// 	UseBonus = true,
// 	BossMission = false,
// 	RetreatEndingMessage = true,
// 	Environment = "Env_Null",
// 	LiveEnvironment = Env_Null,
// 	TurnLimit = 4,
// 	SpawnStart = 5,
// 	SpawnStart_Easy = {4,5},
// 	SpawnStart_Unfair = 5,
// 	GlobalSpawnMod = 0,--changes ALL spawning
// 	SpawnStartMod = 0,
// 	SpawnMod = 0,
// 	MapTags = "generic",
// 	MapVetoes = {},
// 	VoiceEvents = {},
// 	PowerStart = 0,
// 	AssetId = "",
// 	Ambience = "",
// 	AssetPassive = "",
// 	AssetLoc = nil,
// 	DebrisId = {},
// 	MaxEnemy = 6,   --JUSTIN
// 	MaxEnemy_Easy = 6,
// 	DiffMod = DIFF_MOD_NONE,
// 	BlockedSpawns = 0,
// 	KilledVek = 0,
// 	BlockEasy = false,
// 	--LastTurnkills = false,
// 	--PassiveTurn = false,
// 	CustomTile = "",
// 	NextPhase = "",
// 	PhaseCount = 1,
// 	BlockSecret = false,
// 	ID = "",
// }	

 


#endif
