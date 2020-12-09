#include "game/army-gen.hpp"

#include "bounty.hpp"

static constexpr int kVillainArmies[5][17] = {
    {
        Peasants,
        Nomads,
        Sprites,
        Wolves,
        Militias,
        Sprites,
        Militias,
        Elves,
        Orcs,
        Skeletons,
        Skeletons,
        Giants,
        Gnomes,
        Archers,
        Demons,
        Demons,
        Dragons,
    },
    {
        Wolves,
        Militias,
        Sprites,
        Orcs,
        Militias,
        Ghosts,
        Archers,
        Archmages,
        Ogres,
        Zombies,
        Zombies,
        Dragons,
        Barbarians,
        Pikemen,
        Vampires,
        Dragons,
        Dragons,
    },
    {
        Militias,
        Militias,
        Skeletons,
        Archers,
        Archers,
        Knights,
        Pikemen,
        Druids,
        Trolls,
        Ghosts,
        Ghosts,
        Ogres,
        Giants,
        Cavalries,
        Archmages,
        Cavalries,
        Dragons,
    },
    {
        Peasants,
        Peasants,
        Zombies,
        Trolls,
        Elves,
        Archmages,
        Cavalries,
        Pikemen,
        Giants,
        Vampires,
        Vampires,
        Orcs,
        Nomads,
        Knights,
        Gnomes,
        Knights,
        Demons,
    },
    {
        Peasants,
        Peasants,
        Ogres,
        Dwarves,
        Barbarians,
        Archmages,
        Knights,
        Sprites,
        Wolves,
        Demons,
        Demons,
        Gnomes,
        Peasants,
        Dragons,
        Peasants,
        Archmages,
        Vampires,
    },
};

static constexpr int kVillainArmyCounts[17][5] = {
    {
        50,
        10,
        70,
        30,
        50,
    },
    {
        250,
        100,
        30,
        150,
        500,
    },
    {
        600,
        30,
        300,
        35,
        30,
    },
    {
        50,
        100,
        20,
        30,
        50,
    },
    {
        20,
        50,
        10,
        20,
        30,
    },
    {
        20,
        100,
        200,
        5,
        40,    // A
    },
    {
        100,
        50,
        10,
        25,
        25,
    },
    {
        20,
        20,
        10,
        10,
        10,
    },
    {
        20,
        10,
        10,
        30,
        50,
    },
    {
        30,
        20,
        80,
        100,
        200,
    },
    {
        25,
        30,
        60,
        20,
        2,
    },
    {
        10,
        4,
        16,
        30,
        5,
    },
    {
        10,
        25,
        200,
        100,
        60,
    },
    {
        500,
        250,
        100,
        25,
        40,    // B between 2-40
    },
    {
        4,
        6,
        5,
        4,
        16,
    },
    {
        300,
        80,
        6,
        10,
        200,
    },
    {
        700,
        5,
        5000,
        60,
        100,
    },
};

/*
	D5: 0-17 Villain
	D6: 0-4 index
	D0: Scratch
	D1: Offset into IDs source
	D2: Offset into IDs destination
	-$5(A6): Castle ID

	int a = villain * 17
	int b = (castle * 4) + castle
	int *c = &castle_units
	int d = i + b
	c[castle][]

*/
void gen_villain_army(int villain, std::array<int, 5> &army, std::array<int, 5> &counts)
{
    for (int i = 0; i < 5; i++) {
        army[i] = kVillainArmies[i][villain];
        int count = kVillainArmyCounts[villain][i];
        count += (kVillainArmyCounts[villain][i] / 16) + (rand() % count);
        count += (kVillainArmyCounts[villain][i] / 32) + (rand() % count);
        count += 4 + (rand() % count);
        counts[i] = count;
    }
}
