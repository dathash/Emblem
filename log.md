
emblem log

# Design
## Questions
Should placing a unit on a tile change the tile to neutral again? This seems
reasonable from programming, but might not play as well.

Should paralysis be nullified by being pushed, damaged, etc? Should a unit be
able to break out of it just to move?

## Foundations
What is appealing to me about tactics games?
    Permadeath sacrifices
    Taking Risks
    Gaining new team members
    The beginning of Fire Emblem games are always the best.
    Middle of a level is best.
        Beginning: Setting up, being overly cautious. Not good.
        End: Ganging up on a boss. Pretty much a waste of time.
    Individual Characters
        Strong Characterization
        Developing Relationships
 
Ideas
* Social Psychology game
    Characterization is deeply ingrained into combat.
        Characters bark, speak their minds.
            Cowardly character doesn't like getting sent to the front lines.
                Performs worse.
                Over time becomes more comfortable, confident.
            Confident character tends to overextend.
                Enemies hit them more often.
        Conversations mid-battle.

* Deep exploration of mechanics
    Fire Emblem reaches a point where it's all sort of the same.
    What if we dug in to this?
        Made mechanics that lended themselves towards extremely specific playstyles?
        Made objectives that required modes of thinking?
        Dropped RPG elements to make way for the new puzzle-style mechanic?

Why are we fighting?
Fire Emblem maps are almost always about killing all the enemies
What about other objectives?
What about Save all the Innocents?
What about Collect all the Items?
What about Escape?
What about anything else?

What do RPG Mechanics really achieve?
* Feeling of commitment to the game.
* Numbers go up hehe
* The strategic decision of nurturing a weak unit until they grow.

Group mechanics (Units nearby eachother get bonuses)
Leader mechanics (Genealogy stars)
Fort with attack bonus instead of defense
Knight Moves?
Jumping tiles in general?
Boost teammate units over stuff?
Misty Step
Area of Effect
Grapple Mechanic, to hold units back so that Zarathustra can charge to the end.

Ability system, like that of D&D.

Really explicit die-rolls.
Simple statistics
I like the Two-handed fighter ability to reroll 1's.
I like the FTD mechanic where halberds can use intelligence for damage.

I like the idea of magic items which gain points as you use them for specific
things, like the sword that gets bonuses per-kill, or the armor that gets
stronger as it takes more hits.
I really like the dark souls Ring system, and the 5e attunement system.
Genealogy's items are pretty close too.

## Team design
Rome
* 5 mov
* 3 hp
* passive | swift
* Abilities
  * Tailwind | Move all units one over.
  * Buff | Applies Swift


Guy | The Lithomancer
* 2 movement
* 4 hp
* passive | stoneskin.
* Abilities
  * Sword | 1-2 range, solid damage.
  * Shield | Apply Stone to a unit and pull them to Guy.

Bata
* 3 mov
* 2 hp
* passive | immune to fire damage.
* Abilities
  * Fireball | diamond pattern, 2 dmg plus apply fire.
  * Flame Wall | Puts up a wall of flame, blocking attacks.

Suba
* 2 mov
* 3 hp
* passive | overdrive, double speed and dmg when on 1 hp.
* Abilities
  * Volt Tackle | ram attack
  * Jolt | line shot, no dmg with paralyze.

Fire 2
Quick, up in your face, paladin with two blades.
Glung

Wind 2
Grappler, can hold units in a whole line in place.
Rabbit?

Lightning 2
Lion
Paladin
Smite with paralysis.
Lightning Bolt.

Stone 2
Create blockages on the map, with stones.

## Strategy Layer
TODO

# TODO
BUG | Rogue Bata after you restart
BUG | Restart level doesn't work at all.

Visualization of attacks.
* Arrows for movement/push
* Trajectories
  * Push
  * Artillery
  * Line shot

Eventual architecture:
* In resolution, we keep a bunch of incidents.
* These incidents can be analyzed for their outcomes with a function call.
* This function is called when something changes, and we percolate the change
  through the whole array.
* We can display this outcome at any time, and animate it and resolve it.
* Therefore, an incident has three functions:
  * Re-calculate
  * Display
  * Animate + Resolve
This way, we don't have to calculate the outcome twice, and keep the two logics
in sync.

Sanding pass!!!

Finish warp mechanics (with unit storage overhaul)
Random enemy spawns at the start of level


Strat layer
* Level Menu
* Start level command
* End Level Command
* Selection


Four more units
Playtesting (for bugs and feel)

Ten levels
Ten Random Objectives

AI Attacks
* Avoid other ai attacks (dangerous)
* Avoid repeated behavior


Animations
CHECK OUT HOW MATTHEW DID IT
* Movement
* Pushing
* Destruction
* Death
* Attacks
* Surfacing

Strategy Layer
Experience points and pilot abilities
Polish

Switch Queue button to a global input thing that just sets a flag so we can hold
it.


# Later
Art
8 Portraits
8 Spritesheets
5 Tilemaps
Buildings
8 Enemies

Conversations
    Story nailed down
    50 Conversations
    20 Cutscenes

POLISH
    Z-Sorting

    Mini headshots displayed in unit blurb 

    BUG | Paths are drawn without regard for walkable squares. EX: When choosing
    a square to place a unit, its path is drawn through an enemy.
