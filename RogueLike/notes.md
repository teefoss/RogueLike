# TO DO

Actor animation during turn (walking)
Log slide right on disappear
Sword/Shield sprite

little indicator when mobs drop items (ooh!) - rectangular shrinking ring?
damage indicators
alert indicators

put row/col numbers in sprite sheets
line of sight doesn't expand on floors for mob LOS

Selected item indicator when inventory closed.

INVENTORY:
    Make inventory panel 'float' above level (a small margin)
    inventory is only as tall as items held?
    Move inventory counts over 1 scaled pixel
    Reduce space between cols?
- teleporter stone (key)

Food = strength


# BUGS

blob 'stutter'
* key got placed in exit room! / beyond exit room
particle lighting
* pillar button broken
* opening door does not cast sight!


# Keep an eye on...

- Pillar button never spawned?



# Fixed/Complete

- make exit pillars tiles, not actors (no, they are taller than a tile can be)
Mob spawns on exit tile



# DESIGN

## LEVEL

The Forest (one level )
- lakes and forests, dark - find well (need bucket to descend)
The well (levels ?-?)
- first: water surrounded by dirt
- followed by single rooms with stairs down
The Tunnel (one level)
- One long hallway with snakes
The Flooded Grotto (2 levels, perlin, water)
The Catacombs (10? levels)
The Gateway (1 level, non-proc, just a set up for the labyrinth)
The Labyrinth (1 level - huge)

Overall end options:
 - Set number of levels?
 Create a character, tracks no. runs, loot obtained on successful runs
 Goal: Final treasure, a set no. of items (The Jade Amulet of Destiny, etc.) Game 'complete'
 when found all and discovered all other varieties of gems
 
 Treasures go in order: started with more mundane sounding items, the final has the rainbow effect.
 
 Level types:
 - (1) forest (perlin trees and rivers)
 - dungeon of various sizes
 - cave ()
 
 
 - Option to exit early, but miss better loot on higher levels?

## ITEMS

Leveled items:
 - basic: found in vases
 - better: chests

Treasures that give super effects, e.g. permanent increase in turns
but you sacrifice counting it as loot

Different levels of loot:
 - Basic, most common (Loot)
 - Medium, treasure (Generic 'expensive' items - gems)
 - Artifacts, high tier named items


# STORY

+gpt Create a story for a rogue-like computer game where you traverse deeper and deeper levels of dungeon.
In the land of Eryndor, there was a legend of a cursed dungeon that was said to be filled with untold treasures and unspeakable horrors. Many brave adventurers had entered the dungeon, but none had ever returned. You play as a rogue-like character who has decided to risk it all and venture into the cursed dungeon in search of riches and glory.

As you descend deeper and deeper into the dungeon, you encounter all sorts of monsters and traps. You must use your wits and skills to overcome these challenges and continue your descent. Each level of the dungeon is procedurally generated, making every playthrough unique.

As you progress, you find powerful weapons and armor that will aid you in your journey. You also discover magical artifacts that grant you special abilities, such as the ability to teleport or turn invisible.

But the deeper you go, the more dangerous the dungeon becomes. The monsters become stronger and the traps more deadly. You must constantly be on your guard and make split-second decisions to survive.

At the very bottom of the dungeon lies the ultimate treasure: a magical artifact that is said to grant unlimited power to whoever possesses it. But to reach it, you must face the most powerful monster of all, the dungeon's guardian.

Will you be able to defeat the guardian and claim the ultimate treasure? Or will you become just another adventurer lost in the cursed dungeon? The choice is yours in this thrilling rogue-like adventure.


# Resources

- https://www.redblobgames.com/maps/terrain-from-noise/



# Other

next game: 10 "worlds"
- dark forest, two moons transit at different speed, occasionally lining up for
extra brightness and total darkness, same gen as this rogue-like (clearings and lakes)
- blizzard world
- underground
