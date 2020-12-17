# TODO

### General

- Music
- Sounds
- Replace assets with free ones
- Cleaner variable/state handling. I don't like the Variables/GenVariables thing.
- Maybe cleaner "scene" handling. I don't like how you have to pass everything you want to use
	into the class, but it's okay for now.
- Remove all the debug keys:
	- K in game to win
	- E in game to lose
	- V in battle to win
	- G to toggle flying
	- B to toggle boat
	- D to toggle puzzle pieces
	- R to regen RNG tiles
- Add fading between battle, victory, loss, intro, etc.

### In-game

- What happens when you ask for an audience with the king when already max rank? I think maybe there is no option.
- Better collision?
- Add artifact effects
- Stop mobs from bunching up
- Alternative way of animating the tiles? Loading 10 full tilesets seems excessive.
- Implement "can_move" for mob and hero in terms of virtual functions instead of a function pointer.

### Battle

- Add archmage/druids shoot magic in battle
- Archers seem too tanky. Probably a bug there.
- Add HALF etc effect messages in battle
- Add AI
- Add out of control
