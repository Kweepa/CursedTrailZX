#include <stdio.h>
#include <stropts.h>    // ioctl
#include <input.h>
#include <arch/zx.h>
#include <rect.h>
#include <z80.h>
#include <string.h>
#include <stdlib.h>
#include <graphics.h>

char day = 1;
int miles = 0;
int oldMiles = 0;
char damage = 0;
char post = 0;
char fixAvailable = 0;
char scout = 1;
char speed = 60;
char tweak = 0;
char lastEvent = 99;

#define MAX_DAMAGE 4

int judges = 5;
int bikes = 3;
int rations = 80;
int ammo = 70;
int fix = 5;
int meds = 5;
int gold = 100;
int vaccine = 1;

int rationsCost = 3;
int ammoCost = 2;
int medsCost = 8;
int fixCost = 38;

char buf[128];

struct item
{
	int *val;
	char *name;
	char *desc;
};

struct item items[8] =
{
	{ &judges,"Judges","These are you and your squad. Having a full team makes defence and hunting easier, and you need judges to ride the Lawmasters." },
	{ &bikes,"Lawmasters","These are the brand new MkII ""Quasar"" Lawmasters. The judge is out on their reliability. You use these to scout out the best route to take each day." },
	{ &rations,"Rations","Standard Justice-Rations (including anti-rad pills) or the food you have scavenged or bought along the way. You need rations to keep your squad alive and healthy." },
	{ &ammo,"Ammunition boxes","You need ammo for the Land Raider's defence systems, to fight off mutants and animals, and to hunt for food." },
	{ &fix,"Fix kits","Fix kits are used to repair the Land Raider. If you run out, you will have to limp onward, but slowly." },
	{ &meds,"Med packs","Standard Justice Department issue medical packs. Each is essentially a single use doc in a box." },
	{ &gold,"Gold","To trade in the radlands you need physical currency---gold. It's heavy so you can't take much." },
	{ &vaccine,"Vaccine","The vaccine to the 2T(FRU)T virus that you absolutely, positively must get to Mega-City Two." }
};

void putcc(char *s)
{
	for (char i = 0; s[i] != 0; ++i)
	{
		putc(s[i],stdout);
	}
}

void ink(int c)
{
	char s[3];
	s[0] = 16;
	s[1] = 48+c;
	s[2] = 0;
	putcc(s);
}

void paper(int c)
{
	char s[3];
	s[0] = 17;
	s[1] = 48+c;
	s[2] = 0;
	putcc(s);
}

void bright(int c)
{
	char s[3];
	s[0] = 19;
	s[1] = c;
	s[2] = 0;
	putcc(s);
}

void cursorTo(int x, int y)
{
	char s[4];
	s[0] = 22;
	s[1] = x;
	s[2] = y;
	s[3] = 0;
	putcc(s);	
}

void print_at(int x, int y, char *t)
{
	cursorTo(x, y);
	putcc(t);
}

void print_ww(char const *t)
{
	int i = 0;
	int e = strlen(t);
	do
	{
		int d = i + 32;
		if (d >= e)
		{
			putcc(t + i);
			if (strlen(t + i) < 32)
			{
				puts("");
			}
		}
		else
		{
			while (t[d] != ' ')
			{
				--d;
			}
			for (int k = i; k < d; ++k)
			{
				putc(t[k], stdout);
			}
			if (d - i < 32)
			{
				puts("");
			}
		}
		i = d + 1;
	}
	while (i < e);
}

void print_c(char *t)
{
	int e = strlen(t);
	for (int i = 0; i < 15 - e / 2; ++i)
	{
		putc(' ', stdout);
	}
	puts(t);
}

void cr()
{
	puts("");
}

void cls()
{
	zx_cls(INK_WHITE | PAPER_BLACK);
	paper(INK_BLACK);
	ink(INK_WHITE);
	cursorTo(1,1);
}

void pressSpace()
{
	paper(INK_BLUE);
	ink(INK_YELLOW);
	print_at(14,24,"  Press space...  ");

	while (!in_key_pressed(IN_KEY_SCANCODE_SPACE)) rand();
	while (in_key_pressed(IN_KEY_SCANCODE_SPACE));

	cls();
}

void intro()
{
	zx_cls(INK_WHITE | PAPER_BLACK);
	paper(INK_BLACK);
	bright(1);
	zx_border(INK_CYAN);
	ink(INK_GREEN);
	print_c("MEGA-CITY EDUCATIONAL");
	print_c("COMPUTING CONSORTIUM");
	ink(INK_WHITE);
	print_c("January 22, 2123");
	cr();
	print_c("Presents");
	cr();
	ink(INK_YELLOW);
	print_c("The CURSED TRAIL");
	cr();
	ink(INK_CYAN);
	print_c("by Kweepa");
	print_c("for the 2023 CSSCGC");

	cr();
	ink(INK_WHITE);
	print_ww("This program simulates a trip across the CURSED EARTH from Mega-City One to Mega-City Two in 2100.");
	cr();
	print_ww("Your team of five judges will cross the two thousand mile CURSED EARTH in three to four weeks---if you make it alive.");
	
	draw(0,0,255,191);
	
	pressSpace();

	cr();
	print_ww("Mega-City Two has been struck by the 2T(FRU)T virus, which scrambles the mind and induces a hunger for the \"forbidden fruit\"---human flesh!");
	cr();
	print_ww("You lead a team of Mega-City One judges on a mission to deliver a vaccine before the City succumbs entirely to chaos and cannibalism.");
	cr();
	print_ww("The infected control the spaceport, so your only option is over land---land devastated by the atomic wars.");
	
	pressSpace();
	
	cr();
	print_ww("At the vehicle testing ground, Justice HQ...");
	cr();
	print_ww("Judge McGruder: \"There's your vehicle... the new K2001 Land Raider! It's got four wheel drive, thermonuclear engines, flame thrower, machine guns and a special compartment for carrying the anti-plague vaccine.");
	cr();
	print_ww("\"Hold it---you've only seen half the Land Raider! I think you'll like... the Killdozer! The Killdozer is equipped with nemesis rockets, cannon, laser guns, Quasar bikes, and is nuclear blast-proof.\"");
	
	pressSpace();
}

int getSelectionKey(char *keys, char extra)
{
	int k = 0;
	while (k == 0)
	{
		int c = in_inkey();
		if (c > 0 && (c == extra || c == extra + 32))
		{
			k = c;
		}
		int e = strlen(keys);
		for (int i = 0; i < e; ++i)
		{
			if (c == keys[i] || c == keys[i] + 32)
			{
				k = c;
				break;
			}
		}
	}
	while (in_inkey() > 0) ;

	return k & ~32;
}

int getSpecificKeyAndReturnTime(char key)
{
	char c = 0;
	int t = 0;
	while (1)
	{
		if (++c == 91)
		{
			c = 0;
			++t;
			if (t == 500)
			{
				return t;
			}
		}
		int c = in_inkey();
		if (c == key || c == key + 32)
		{
			return t;
		}
	}
}

void header()
{
	ink(INK_CYAN);
	cursorTo(2,2);
	printf("Day %d", day);

	ink(INK_MAGENTA);
	cursorTo(18,2);
	printf("Odometer %d", miles);
	
	cr();
	cr();
	ink(INK_WHITE);
}

int max(int a, int b)
{
	return a > b ? a : b;
}

int min(int a, int b)
{
	return a < b ? a : b;
}

void stock()
{
	header();

	char const *damages[] = { "", " lightly damaged", " damaged", " heavily damaged" };
	sprintf(buf, "Your%s Land Raider is stocked with the following items:", damages[damage]);
	print_ww(buf);
	
	for (int i = 0; i < 8; ++i)
	{
		*items[i].val = max(0, *items[i].val);
	}
	
	for (int i = 0; i < 8; ++i)
	{
		ink(INK_GREEN);
		cursorTo(4,8+i);
		printf("% 4d", *items[i].val);
		putc(' ',stdout);
		ink(INK_YELLOW);
		paper(INK_RED);
		putc(items[i].name[0],stdout);
		paper(INK_BLACK);
		puts(items[i].name+1);
	}
	
	ink(INK_WHITE);
	print_at(1,17,"You can ");
}

char tryStockDesc(int c)
{
	char r = 0;
	for (int i = 0; i < 8; ++i)
	{
		if (c == items[i].name[0])
		{
			cursorTo(1,7);
			paper(INK_RED);
			ink(INK_YELLOW);
			printf(" %s ", items[i].name);
			paper(INK_BLACK);
			ink(INK_WHITE);
			cr();
			cr();
			print_ww(items[i].desc);
			r = 1;
			break;
		}
	}
	return r;
}

void changeStock()
{
	bikes = 2 + (rand() % 2);
	rations = 50 + (rand() % 51);
	ammo = 50 + (rand() % 51);
	fix = 3 + (rand() % 3);
	meds = 4 + (rand() % 3);
	gold = 120 + (rand() % 81);
}

void getLoadout()
{
	day = 1;
	miles = 0;
	oldMiles = 0;
	damage = 0;
	post = 0;
	fixAvailable = 0;
	scout = 1;
	speed = 60;
	tweak = 0;
	lastEvent = 99;

	judges = 5;
	
	changeStock();

	char d = 0;
	while (!d)
	{
		stock();
		ink(INK_YELLOW); paper(INK_RED); putc('C',stdout); paper(INK_BLACK); puts("hange the loadout");
		ink(INK_WHITE);print_at(1,18,"     or ");
		ink(INK_YELLOW); paper(INK_RED); putc('S',stdout); paper(INK_BLACK); puts("tart the journey.");	
		
		int c = getSelectionKey("JLRAFMGVCS", 0);
		
		cls();

		switch (c)
		{
		case 'C':
			changeStock();
			break;
		case 'S':
			d = 1;
			break;
		}
		
		if (tryStockDesc(c))
		{
			pressSpace();
		}
	}
}

char const *creatures[6] = { "mutant buffalo", "styracosaurus", "giant rat", "mutant", "rider", "robot" };

struct fightresult
{
	char shots;
	char rations;
};

void gunfight(char creature, struct fightresult *fr)
{
	cr();
	print_ww("You must type BLAM, BAM or BDAM when prompted as quickly as possible. Your speed will determine the outcome of the skirmish.");
	pressSpace();
	cr();
	cr();
	char s = 0;
	char r = 0;
	for (int i = 0; i < 3; ++i)
	{
		int t = 50 + (rand() % 50);
		for (int j = 0; j < t; ++j)
		{
			// this loop takes about 1/50th second
			for (int k = 0; k < 1000; ++k) ;
		}

		int b = rand() % 3;
		char const *bangs[3] = { "BLAM", "BAM", "BDAM" };
		printf("Type %s ", bangs[b]);

		t = 0;
		for (int j = 0; j < strlen(bangs[b]); ++j)
		{
			// get approx 1/50ths
			t += getSpecificKeyAndReturnTime(bangs[b][j]);
			putc(bangs[b][j], stdout);
		}
		t /= 20;
		t = max(1, min(t, 5));

		cr();
		char const *speeds[5] = { "Lightning!!","Fast!","Ok.","Slow.","Snail..." };
		puts(speeds[t-1]);

		s += 1 + (rand() % (1 + t));
		b = max(0, 7 - t);
		r += 1 + (rand() % (2 * b));
	}
	cr();
	if (s > 9)
	{
		judges = judges - 1;
		sprintf(buf, "A %s gets one of your party!", creatures[creature]);
		print_ww(buf);
		cr();
	}
	ammo -= s;

	fr->shots = s;
	fr->rations = r;
}

void hunt()
{
	if (ammo < 10)
	{
		cursorTo(1,7);
		print_ww("Drokk! You need more ammunition to go hunting.");
		return;
	}
	cr();
	cr();
	char c = rand() % 3;
	sprintf(buf, "Your hunting party sneaks up on a herd of %s.", creatures[c]);
	print_ww(buf);
	struct fightresult fr;
	gunfight(c, &fr);
	if (judges > 0)
	{
		sprintf(buf, "You expended %d ammo to get %d rations.", fr.shots, fr.rations);
		print_ww(buf);
	}
	rations += fr.rations;
	++day;
}

void displayTradingItem(char y, char cost, int num, char *name)
{
	cursorTo(2,y); ink(INK_RED); printf("% 4d", cost); ink(INK_GREEN); printf("% 4d ", num);
	if (name[0])
	{
		ink(INK_YELLOW);
		paper(INK_RED);
		putc(name[0], stdout);
		paper(INK_BLACK);
		putcc(name+1);
	}
}

void displayTradingGold()
{
	cursorTo(6, 15+fixAvailable);ink(INK_GREEN); printf("% 4d", gold); ink(INK_YELLOW); putcc(" Gold");
}

void tryBuy(char y, char try, char cost, int *num)
{
	if (try && gold >= cost)
	{
		gold -= cost;
		displayTradingGold();
		*num += 1;
		displayTradingItem(y, cost, *num, "");
	}
}

void trade()
{
	header();
	print_ww("You stop at a small mutant outpost. There isn't much to trade but they are willing to take gold from judges for what they have:");
	ink(INK_RED); print_at(2,10,"Cost"); ink(INK_GREEN); putcc(" Inventory");
	displayTradingItem(12, rationsCost, rations, "Rations");
	displayTradingItem(13, ammoCost, ammo, "Ammunition boxes");
	displayTradingItem(14, medsCost, meds, "Med packs");
	if (fixAvailable)
	{
		displayTradingItem(15, fixCost, fix, "Fix kits");
	}
	displayTradingGold();
	cr();
	cr();
	ink(INK_WHITE);
	print_ww("Type the first letter of an item to buy that item.");
	cr();
	putcc("Or you can "); paper(INK_RED); ink(INK_YELLOW); putc('Q', stdout); paper(INK_BLACK); putcc("uit trading.");
	
	while (1)
	{
		int c = getSelectionKey("QRAM", fixAvailable ? 'F' : 0);
		
		tryBuy(12, c == 'R', rationsCost, &rations);
		tryBuy(13, c == 'A', ammoCost, &ammo);
		tryBuy(14, c == 'M', medsCost, &meds);
		tryBuy(15, c == 'F', fixCost, &fix);
		
		if (c == 'Q')
		{
			break;
		}
	}
	++day;
	rations = rations - judges;
	
	if (fix > 0 && damage > 0)
	{
		cls();
		cursorTo(1,7);
		print_ww("You make repairs to the Land Raider with the purchased Fix kits.");
		char used = min(fix, damage);
		fix -= used;
		damage -= used;
	}
}

void travel()
{
	cr();
	if (judges > 0 && damage < MAX_DAMAGE)
	{
		char scouts = max(0, min(bikes, judges - 1));
		if (!scout) scouts = 0;
		int m = speed / (1 + damage);
		if (scouts > 0)
		{
			m += (rand() % (speed * scouts));
			sprintf(buf, "Scouting with %d Lawmaster%s, you advance %d miles.", scouts, scouts > 1 ? "s" : "", m);
		}
		else
		{
			sprintf(buf, "Without the ability to scout ahead, you only travel %d miles.", m);
		}
		print_ww(buf);
		speed = 60;
		oldMiles = miles;
		miles += m;
		++day;
		post = miles > 0 && ((rand() % 6) < min(1 + scouts, 3));
		rationsCost = 4 + (rand() % 4);
		ammoCost = 3 + (rand() % 3);
		medsCost = 9 + (rand() % 5);
		fixAvailable = (miles > 500 && (rand() & 1) == 0);
		fixCost = 39 + (rand() % 17);
		rations -= judges;
		scout = 1;
	}
}

void doDamage()
{
	cr();
	if (fix > 0)
	{
		print_ww("You have to stop to make repairs.");
		--fix;
	}
	else
	{
		print_ww("Since you can't repair it, you will have to limp onward.");
		++damage;
	}
	speed >>= 1;
}

void breakdown()
{
	print_ww("Dust storm! The air vents clog up and damage the Land Raider's engines.");
	doDamage();
}

void permlosebike()
{
	if (bikes > 0 && judges > 1 && scout)
	{
		print_ww("While scouting ahead, one of the Lawmasters falls into a rad pit. The judge just manages to escape alive.");
		--bikes;
	}
}

void injury()
{
	char const *injuries[] = { "a leg", "an arm", "a rib" };
	sprintf(buf, "Stomm! %s %s. ", judges == 1 ? "You break " : "A judge breaks", injuries[rand() % 3]);
	print_ww(buf);
	cr();
	if (meds > 0)
	{
		print_ww("You have to stop for an hour or two to use a med pack.");
		--meds;
	}
	else
	{
		sprintf(buf, "Without any med packs, the wound goes septic and %s!", judges == 1 ? "you die" : "the judge dies");
		print_ww(buf);
		--judges;
	}
	speed >>= 1;
}

void templosebike()
{
	if (bikes > 0 && judges > 1 && scout)
	{
		print_ww("High radiation interferes with a Lawmaster's navigation system and you spend half a day tracking it down.");
		speed >>= 1;
	}
}

void losejudge()
{
	if (bikes > 0 && judges > 1 && scout)
	{
		print_ww("A judge gets lost on a recce and you lose travel time looking for them.");
		speed >>= 1;
	}
}

void unsafewater()
{
	sprintf(buf, "%s pre-treated irradiated water.", judges > 1 ? "A judge accidentally drinks" : "You accidentally drink");
	print_ww(buf);
	cr();
	if (meds > 0)
	{
		print_ww("It takes a few hours for the med pack to flush the radiation from the bloodstream.");
		--meds;
	}
	else
	{
		char const *who = judges > 1 ? "they" : "you";
		sprintf(buf, "Since %s have no med packs, %s fall ill and die!", who, who);
		print_ww(buf);
		--judges;
	}
	speed >>= 1;
}

void rain()
{
	// rain or snow depending on the mileage
	if (miles > 950)
	{
		print_ww("The Land Raider throws a track in heavy snow.");
	}
	else
	{
		print_ww("Heavy rains flood the Land Raider's engines.");
	}
	doDamage();
}

void mutantsattack()
{
	print_ww("Mutants attack!");
	if (ammo < 10)
	{
		print_ww("You don't have enough ammo to defend yourselves, and the mutants kill a judge and make off with some gold.");
		--judges;
		gold -= 20 + (rand() % 50);
	}
	else
	{
		struct fightresult fr;
		gunfight(3, &fr);
		if (judges > 0)
		{
			print_ww("You drive the mutants away.");
		}
	}
}

void fire()
{
	print_ww("There was a fire in the Land Raider! You extinguished it quickly, but some of your supplies were damaged.");
	rations -= rand() % 8;
	ammo -= rand() % 7;
	doDamage();
	meds -= rand() % 5;
}

void fog()
{
	print_ww("Your journey is slowed by a heavy radiation fog that damages the Land Raider. You can't take the Lawmasters out to scout ahead, and you can barely see.");
	doDamage();
	scout = 0;
}

void snake()
{
	sprintf(buf, "%s bitten by a giant radioactive spider.", judges > 1 ? "A judge is" : "You are");
	print_ww(buf);
	cr();
	if (meds > 0)
	{
		--meds;
		print_ww("With the help of a med pack the wound is healed, but you lose some time.");
	}
	else
	{
		--judges;
		if (judges > 0)
		{
			print_ww("You have to stop to bury the body.");
		}
	}
	speed >>= 1;
}

void river()
{
	print_ww("The Land Raider is stopped in its tracks by a river of fire.");
		cr();
	putcc("Will you "); paper(INK_RED); ink(INK_YELLOW); putc('F', stdout); paper(INK_BLACK); puts("ord it,");
	ink(INK_WHITE); putcc("      or "); paper(INK_RED); ink(INK_YELLOW); putc('S', stdout); paper(INK_BLACK); puts("earch for a safe path?");
	
	int c = getSelectionKey("FS", 0);
	ink(INK_WHITE);
	cr();
	
	if (c == 'F')
	{
		if (rand() & 1)
		{
			print_ww("The Landraider is burned fording the river. You also lose some rations.");
			rations -= 1 + (rand() % 10);
			doDamage();
		}
		else
		{
			print_ww("You successfully ford the river and make good time.");
		}
	}
	else
	{
		print_ww("You lose time searching for a cooler part of the river.");
		speed /= 3;
	}
}

void stampede()
{
	print_ww("Stampede!");
	cr();
	sprintf(buf, "The Land Raider is surprised by a stampede of %s.", creatures[rand() % 3]);
	print_ww(buf);
	if (ammo < 10)
	{
		cr();
		print_ww("You don't have enough ammo to fend off the animals. The Land Raider is damaged in the stampede and needs repairs.");
		doDamage();
	}
	else
	{
		struct fightresult fr;
		gunfight(rand() % 3, &fr);
		if (judges > 0)
		{
			print_ww("You drive the animals away.");
			if (fr.shots < 7)
			{
				cr();
				print_ww("You manage to kill some and add the meat to your rations.");
				rations += fr.rations;
			}
		}
	}
}

void hail()
{
	print_ww("The Land Raider drives into a shower of irradiated rocks thrown down from the death belt.");
	cr();
	cr();
	
	putcc("Will you "); paper(INK_RED); ink(INK_YELLOW); putc('F', stdout); paper(INK_BLACK); puts("ind shelter,");
	ink(INK_WHITE); putcc("      or "); paper(INK_RED); ink(INK_YELLOW); putc('D', stdout); paper(INK_BLACK); puts("rive through it?");
	
	int c = getSelectionKey("FD", 0);
	ink(INK_WHITE);
	cr();
	
	if (c == 'F')
	{
		if (rand() % 5 == 1)
		{
			print_ww("You search in an area you thought would provide shelter, but the Land Raider gets pummelled anyway.");
			doDamage();
		}
		else
		{
			print_ww("You find a large rock overhang and wait out the storm.");
			speed /= 3;
		}
	}
	else
	{
		if (rand() % 5 < 2)
		{
			print_ww("The Land Raider drives on. It gets pummelled by the storm.");
			doDamage();
			if (bikes > 0)
			{
				cr();
				print_ww("A Lawmaster is also crushed by a heavy rock.");
				--bikes;
			}
		}
		else
		{
			print_ww("The Land Raider drives on. Fortune favours the bold! You avoid the worst of the storm.");
		}
	}
}

void illness()
{
	char const *source[] =
	{
		"You get too close to a rad pit!",
		"You drive through a rad cloud!",
		"Your squad eats irradiated rations!"
	};
	print_ww(source[rand() % 3]);
	cr();
	
	if (judges == 1)
	{
		print_ww("You get radiation burns and need medical treatment.");
		if (meds < 1)
		{
			cr();
			print_ww("You don't have any med packs and succumb to the radiation!");
			judges = 0;
		}
		else
		{
			--meds;
		}
	}
	else
	{
		int num = min(judges, rand() % 3);
		if (num > 1)
		{
			print_ww("Some judges get radiation burns and need medical treatment.");
		}
		else
		{
			print_ww("A judge gets radiation burns and needs medical treatment.");
		}
		if (meds < num)
		{
			cr();
			sprintf(buf, "You don't have enough med packs and %s succumb to the radiation.", num > 1 ? "some judges" : "they");
			print_ww(buf);
			judges -= num - meds;
			meds -= num;
		}
		else
		{
			meds -= num;
		}
	}
	speed >>= 1;
}

void mutantshelp()
{
	print_ww("A band of mutants gives you food to leave them in peace. You try to explain to them that it isn't necessary but they won't hear it.");
	rations += 4 + (rand() % 7);
}

void events()
{
	char const chance[] = { 6,11,13,15,17,22,32,35,37,42,44,54,64,69,95,100 };
	char e = lastEvent;
	while (e == lastEvent)
	{
		char r = 1 + (rand() % 100);
		e = 0; while (r > chance[e]) ++e;
	}
	lastEvent = e;

	cr();
	cr();
	switch (e)
	{
	case 0: breakdown(); break;
	case 1: permlosebike(); break;
	case 2: injury(); break;
	case 3: templosebike(); break;
	case 4: losejudge(); break;
	case 5: unsafewater(); break;
	case 6: rain(); break;
	case 7: mutantsattack(); break;
	case 8: fire(); break;
	case 9: fog(); break;
	case 10: snake(); break;
	case 11: river(); break;
	case 12: stampede(); break;
	case 13: hail(); break;
	case 14: illness(); break;
	case 15: mutantshelp(); break;
	}
}

void knox()
{
	pressSpace();
	cr();
	cr();
	if (judges < 2)
	{
		print_ww("You are taken in the night by a coven of robot vampires. The blood they drain is used to keep former President Booth alive.");
		judges = 0;
	}
	else
	{
		print_ww("One of the judges is taken in the night by a coven of robot vampires! You track them back to a basement in the ruins of Fort Knox.");
		struct fightresult fr;
		gunfight(5, &fr);
		if (judges == 1)
		{
			judges = 0;
		}
		else
		{
			print_ww("You defeat the vampires, and discover they were keeping former President Booth alive by feeding him scavenged blood. Amongst the ruins you also uncover a small cache of gold that miraculously survived looting.");
			gold += 10 + (rand() % 20);
		}
	}
}

void zoo()
{
	pressSpace();
	cr();
	print_ww("You spot a strange furry creature being chased by riders. From the creature's movement you sense an innate intelligence.");
	cr();
	print_ww("The pursuit is heading in your direction");
	cr();
	putcc("Will you "); paper(INK_RED); ink(INK_YELLOW); putc('I', stdout); paper(INK_BLACK); puts("ntercede,");
	ink(INK_WHITE); putcc("      or "); paper(INK_RED); ink(INK_YELLOW); putc('L', stdout); paper(INK_BLACK); puts("et things play out?");
	
	int c = getSelectionKey("IL", 0);
	ink(INK_WHITE);
	cr();
	
	if (c == 'L')
	{
		print_ww("You let the riders recapture the creature, but you can't help but feel that you made the wrong decision. As if to confirm your feeling, the Land Raider breaks down and it takes two days to repair.");
		day += 2;
		rations -= 2 * judges;
	}
	else
	{
		print_ww("You stop the riders, who claim the creature is an escaped alien slave, and insist on taking it back.");
		struct fightresult fr;
		gunfight(4, &fr);
		if (judges > 0)
		{
			print_ww("You drive off the riders and the alien joins your party.");
			cr();
			print_ww("He senses he can trust you. He explains that he must keep his intelligence secret or humans would take an interest in his world, then discover and plunder its riches. You have to agree.");			
			tweak = 1;
		}
	}
}

void satanus()
{
	pressSpace();
	cr();
	if (judges == 1)
	{
		print_ww("You are ambushed by mutants.");
		struct fightresult fr;
		gunfight(3, &fr);
		if (judges > 0)
		{
			if (fr.shots < 8)
			{
				print_ww("You subdue the mutants, and scavenge some rations and ammunition from the town.");
				rations += 1 + (rand() % 20);
				ammo += 1 + (rand() % 20);
			}
			else
			{
				print_ww("You are too slow! The mutants take you captive and tie you to a twenty foot stake at the edge of their settlement. From the wastes you hear a sniffing and a low roar. Then a huge battle-scarred tyrannosaur lopes towards you!");
				print_ww("The tyrannosaur makes short work of you.");
				judges = 0;
			}
		}
	}
	else
	{
		print_ww("Your squad is ambushed by mutants. They take a judge captive and tie them to a twenty foot stake at the edge of their settlement. From the wastes comes a sniffing and a low roar. Then a huge battle-scarred tyrannosaur lopes toward the stake!");
		struct fightresult fr;
		gunfight(3, &fr);
		if (judges > 1)
		{
			if (fr.shots > 7)
			{
				print_ww("You are too slow! The tyrannosaur makes short work of the tied up judge, to the delight of the mutants.");
			}
			else
			{
				print_ww("You fight off the tyrannosaur and subdue the mutants, then untie the judge. You scavenge some rations and ammunition from the town.");
				rations += 1 + (rand() % 20);
				ammo += 1 + (rand() % 20);
			}
		}
	}
}

void deathValley()
{
	pressSpace();
	cr();
	print_ww("The heat and radiation make travel across this stretch of desert slow going. You think it's a mirage at first, but war robots, left over from the atomic wars, start to crawl out of the sand and approach.");
	struct fightresult fr;
	gunfight(5, &fr);
	if (judges > 0)
	{
		if (tweak)
		{
			print_ww("You manage to fight off the nearby robots. The alien Tweak distracts the others, allowing you to escape.");
		}			
		else if (bikes > 0)
		{
			print_ww("You manage to fight off the nearby robots and escape the rest by rigging a bike to ride into them and explode.");
			--bikes;
		}
		else if (judges > 1)
		{
			print_ww("You manage to fight off the nearby robots. One of your squad sacrifices themselves to allow you to escape.");
			--judges;
		}
		else
		{
			print_ww("Despite your best efforts, the robots overcome you.");
			judges = 0;
		}
	}
}

void encounters()
{
	if (judges > 0 && damage < MAX_DAMAGE)
	{
		if (oldMiles < 500 && miles >= 500) knox();
		if (oldMiles < 1000 && miles >= 1000) zoo();
		if (oldMiles < 1500 && miles >= 1500) satanus();
		if (oldMiles < 2000 && miles >= 2000) deathValley();
		oldMiles = miles;
	}
}

void deadEnd()
{
	cr();
	print_ww("The city is overrun by the virus and has to be nuked from orbit to prevent a pandemic and the end of civilization. That didn't play out historically accurately!");
	cr();
}

void outro()
{
	cls();
	
	if (judges <= 0)
	{
		cursorTo(1,7);
		print_ww("You perish without reaching Mega-City Two.");
		deadEnd();
	}
	else if (damage >= MAX_DAMAGE)
	{
		cursorTo(1,7);
		print_ww("The Land Raider won't move any further and you perish without reaching Mega-City Two.");
		deadEnd();
	}
	else if (rations <= 0)
	{
		cursorTo(1,7);
		print_ww("You run out of rations and perish without reaching Mega-City Two.");
		deadEnd();
	}
	else
	{
		cr();
		print_ww("The Land Raider breaks down within sight of the walls of Mega-City Two, and you stagger exhausted the rest of the way.");
		print_ww("You reach a small door where a group of hold-outs from the virus usher you in and take the vaccine for replication and aerosolization.");
		cr();
		print_ww("Days later you awake in a med chamber. The city is saved.");
		print_ww("Chief Judge Clarence Goodman sends you his heartiest congratulations and wishes you a speedy recovery.");
		print_ww("You take the StratBat to Mega-City One. Things should be back to normal.");
		cr();
		print_ww("TO BE CONTINUED...");
	}
}

int main()
{
	while (1)
	{
		intro();
		getLoadout();
		
		while (miles < 2048 && judges > 0 && damage < 4 && rations > 0)
		{
			stock();
			ink(INK_YELLOW); paper(INK_RED); putc('H',stdout); paper(INK_BLACK); puts("unt");
			if (post)
			{
				ink(INK_YELLOW); paper(INK_RED); cursorTo(9,18); putc('S',stdout); paper(INK_BLACK); puts("top at a trading post");
			}
			ink(INK_WHITE); print_at(1,18+post,"     or ");
			ink(INK_YELLOW); paper(INK_RED); putc('T',stdout); paper(INK_BLACK); puts("ravel.");	
			
			int c = getSelectionKey("JLRAFMGVHT", post ? 'S' : 0);

			cls();
			
			switch (c)
			{
			case 'H':
				hunt();
				break;
			case 'S':
				trade();
				cls();
				break;
			case 'T':
				events();
				travel();
				encounters();
				break;
			}
			
			tryStockDesc(c);
			
			if (c != 'S')
			{
				pressSpace();
			}
		}
		
		outro();
		pressSpace();
	}
}