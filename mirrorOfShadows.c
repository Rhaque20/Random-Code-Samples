#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#define percent 37

//Debuff IDs Frosbite ID: 5 Frozen ID: 10 Electrified ID: 6 Paralyze ID: 10
//array.debuff[i][0] = debuff;// Debuff ID
//array.debuff[i][1]++;// Debuff Stack
//array.debuff[i][2] = 30;//Duration

//Bitmasks for conditions
// 0 = neutral
// 0x0001 = Immobilized
// 0x0002 = Staggered
// 0x0004 = Airborne
// 0x0008 = Super Armor
// 0x0016 = Hyper Armor
// 0x0032 = Overdrive
// 0x0064 = Break
// 0x0128 = 

//Basic ATK structure
//basicatk[i][0] Light Attack Power
//basicatk[i][1] Light Attack time
//basicatk[i][2] Light Attack Type
//basicatk[i][3] Heavy Attack Power
//basicatk[i][4] Heavy Attack time
//basicatk[i][5] Heavy Attack Type

//Statmods

/**   0     1     2     3     4     5     6     7     8     9    10
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 * |  HP | ATK | DEF | MAG | SPR | DEX | AGI | CRI |CurHP|     |MVSPD|
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
 **/
typedef struct mechanic
{
    int id;
    int charges;
    int *elementclip;
    int boreal_energy;
}mechanic;


typedef struct status
{
    int debuff;
    int buff;
    int stack;
    int id;
    int type; // Is it a 1 = buff or 2 = debuff or 3 - Heal
    float time;
    float currtime;
    float effect;
    float chance;
    int apply; // 0 = No one 1 = Self 2 = Entire Party 3 = Enemy 4 = Entire Enemy Party
}status;

typedef struct elemancy
{
    float power;
    int element;
    int hitcount;
    int type;
    float time;
    int chargeable;
    int weight;
    float *mods;// Used if it's a multi hit type. Ex. 
}elemancy;

typedef struct combo
{
    float power;
    int element;
    int hitcount;
    int type;// 0 = No damage 1 = Physical 2 = Magical
    float time;
    int chargeable;
    float spgain;
    int weight; //0 = No hitstun 1 = Light 2 = Heavy
    char *name;
    int chain;
    
}combo;

typedef struct skill
{
    char *skillname;
    float skillmod;
    int charge;
    int element;// 0 = Normal 1= Fire 2 = Water 3 = Wind 4 = Lightning 5 = Ice 6 = Earth 7 = Light 8 = Dark;
    float *sp;//0 = current cooldown 1 = max cooldown
    status *effects;
    int type; //0 = Undodgeable 1 = Evadable //Player 0 = Buff 1 = Skill
    int hitcount;
    int dmgtype;// 0 = non type 1 = Physical Melee 2 = Magical Melee 3 = Physical Projectile 4 = Magical Projectile
    int special; // Checks to see if attack type will change midway through attack.
    float *multi_skillmod;
    float time;
    int cost;
    int numskills;
    int antiair;
    int chain;// Chains into another skill
}skill;

typedef struct extra
{
    float range;
    float *statmods;//% increase[0] HP [1] ATK [2] DEF [3] MAG [4] SPR [5] DEX [6] AGI [7] CRIT [8] MP [9] CRITDMG
    int *upgrades;// 9 stat boosts via flat boosts like equipment
    float *bane;// 0 Hyoman 1 = Beast 2 = Bug 3 = Plant
    float *element; //0 = Normal 1= Fire 2 = Water 3 = Wind 4 = Lightning 5 = Ice 6 = Earth 7 = Light 8 = Dark;
    float *elementres;
    float *dmgtype;//[0] Physical DMG Up [1] Physical DMG Red [2] Magical DMG Up [3] Magical DMG Red

}extra;

typedef struct player
{
    int count; // How many are there
    char *name;// just free
    float mvspd;
    int level;
    float *stats;// 10 length;
    combo *light;
    combo *heavy;
    float *statchange;// just free
    int low;
    int high;
    int x,y;// Represents position
    float atkspd;
    mechanic *ability;
    skill *skills;
    status *effects;//
    int accessory;// Uses bitmasking to toggle specific accessories on or off.
    int awakening;// There are a total of 10 unique effects via bitmasking
    float origHP;
    extra *boosts;
}player;

typedef struct enemy // name,spd,cr,crgain,maxHP,currentHP,level,ATK,DEF,MAG
{
    int count; // How many are there.
    char *name;//just free
    float cr;
    float crgain;
    int boss;
    float mvspd;
    float *stats;// 10 length;
    status *effects;//12 length, 3 height
    float *statchange;// just free
    int armor;
    float remaintime,airtime;
    float mode;
    float *modebar;
    unsigned int condition;
    int level;
    int ready;
    int pattern;
    int ai;
    int size,target;
    int x,y;// Represents position
    int pos;
    int id;
    extra *boosts;
    skill *slots;
}enemy;

void printelement(int element)
{
    switch(element)
    {
        case 1:
            printf("Fire ");
            break;
        case 2:
            printf("Water ");
            break;
        case 3:
            printf("Wind ");
            break;
        case 4:
            printf("Electric ");
            break;
        case 5:
            printf("Ice ");
            break;
        case 6:
            printf("Earth ");
            break;
        case 7:
            printf("Light ");
            break;
        case 8:
            printf("Dark ");
            break;
    }
}

void printdmgtype (int type)
{
    if (type == 1)
    {
        printf("Physical ");
        return;
    }

    if (type == 2)
    {
        printf("Magical ");
        return;
    }

}

int printeffective(int element,float eleres, float dmgres)
{
    int check = 0;
    if (dmgres >= 0.3)
    {
        printf("RESIST! ");
        check++;
    }
    if (dmgres < 0)
    {
        printf("WEAK!! ");
        check++;
    }

    if (element != 0)
    {
        if (eleres >= 0.3)
        {
            printf("INEFFECTIVE! ");
            check++;
        }
        if (eleres < 0)
        {
            printf("EFFECTIVE! ");
            check++;
        }
    }

    if (check > 0)
        return 1;
    
    return 0;
}

float damagecalc(enemy foe,player ally,float atk_mod,int flag,int power,int type, int hitcount)
{
    int boosts,input,critfoe = 0,i,format = 0;
    float critdmg = 1,chance,damage = 0.0,atkbuff,defbuff,atktype,deftype,dmg_type,bane = 0;
    float elementpower,elementres;
    float totaldamage = 0;

    if(foe.stats[2] == 0 || foe.stats[4] == 0)
        return 0;

    if(flag == 1)
    {
        atktype = (ally.stats[1] + ally.boosts->upgrades[1]) * (1 + ally.boosts->statmods[1]);
        deftype = foe.stats[2];
        atkbuff = ally.boosts->statmods[1];
        defbuff = foe.statchange[2];
        critfoe = 1;
        dmg_type = ally.boosts->dmgtype[0];
        if (hitcount == 1)
            format = printeffective(power,foe.boosts->elementres[power],foe.boosts->dmgtype[1]);
        elementpower = ally.boosts->element[power];
        elementres = foe.boosts->elementres[power];
    }

    if(flag == 2)
    {
        atktype = (ally.stats[3] + ally.boosts->upgrades[3]) * (1 + ally.boosts->statmods[3]);
        deftype = foe.stats[4];
        atkbuff = ally.statchange[3];
        defbuff = foe.statchange[4];
        critfoe = 1;
        dmg_type = ally.boosts->dmgtype[2];
        if (hitcount == 1)
            format = printeffective(power,foe.boosts->elementres[power],foe.boosts->dmgtype[3]);
        elementpower = ally.boosts->element[power];
        elementres = foe.boosts->elementres[power];
    }

    if(flag == 3)
    {
        printf("The enemy is attacking!\n");
        atktype = foe.stats[1];
        deftype = ally.stats[2];
        atkbuff = foe.statchange[1];
        defbuff = ally.statchange[2];
        critfoe = 0;
        dmg_type = foe.boosts->dmgtype[0];
        elementpower = foe.boosts->element[power];
        elementres = ally.boosts->elementres[power];
    }

    if(flag == 4)
    {
        printf("The enemy is attacking!\n");
        atktype = foe.stats[3];
        deftype = ally.stats[4];
        atkbuff = foe.statchange[3];
        defbuff = ally.statchange[4];
        critfoe = 0;
        dmg_type = foe.boosts->dmgtype[2];
        elementpower = foe.boosts->element[power];
        elementres = ally.boosts->elementres[power];
    }

    if((flag == 1 || flag == 2) && type == 1)
    {
        chance = rand()%100 + 1;
        ally.boosts->range = rand()%(ally.high - ally.low + 1) + ally.low;
        ally.boosts->range /= 100;
    }

    if((flag == 1 || flag == 2) && type == 0)
    {
        chance = 10;
        ally.boosts->range = rand()%(ally.high - ally.low + 1) + ally.low;
        ally.boosts->range /= 100;
    }

    else
    {
        ally.boosts->range = 1;
    }

    if(ally.stats[7] >= chance && critfoe == 1)
    {
        printf(">>CRITCAL HIT!<<");
        format++;
        critdmg = 1.5;
    }

    if(format > 0)
    {
        printf("\n");
    }

    damage = (ally.level+10) * (atktype * (1 + atkbuff))*1.5/deftype*(1+defbuff)*2;
    damage = damage * atk_mod * (1 + bane) * (1 + elementpower - elementres) * (1 + dmg_type);

    if (hitcount == 1)
    {
        damage = damage * critdmg * ally.boosts->range;
    }

    if (hitcount > 1)
    {
        for (i = 0; i < hitcount; i++)
        {
            chance = rand()%100 + 1;
            ally.boosts->range = rand()%(ally.high - ally.low + 1) + ally.low;
            ally.boosts->range /= 100;

            if(ally.stats[7] >= chance && critfoe == 1)
            {
                printf(">>CRITCAL HIT!<<\n");
                critdmg = 1.5;
            }
            else
            {
                critdmg = 1;
            }
            format = printeffective(power,foe.boosts->elementres[power],foe.boosts->dmgtype[1]);
            if (format > 0)
            {
                printf("\n");
            }
            damage = damage * critdmg * ally.boosts->range;
            printf("%s inflicted %.0f ",ally.name,damage);
            printelement(power);
            printdmgtype(flag);
            printf("Damage.\n");
            totaldamage += damage;
        }

        return totaldamage;
    }

    if(flag == 3 || flag == 4)
    {
        printf("Enemy %s dealt %.0f damage to %s\n",foe.name,damage,ally.name);
        printf("\n");
        return damage;
    }

    printf("%s inflicted %.0f ",ally.name,damage);
    printelement(power);
    printdmgtype(flag);
    printf("Damage.\n");

    return damage;

}

skill *destroyskills(skill *slots)
{
    int i;

    for (i = 0; i < slots->numskills; i++)
    {
        printf("Freeing up %s.\n",slots[i].skillname);
        free(slots[i].skillname);

        if (slots[i].effects != NULL)
        {
            free(slots[i].effects);
        }

        free(slots[i].sp);

        if (slots[i].special > 0)
            free(slots[i].multi_skillmod);
    }

    free(slots);

    return NULL;
}

extra *freeextra(extra *moar)
{
    if (moar == NULL)
    {
        printf("Nothing was intialized.\n");
        return NULL;
    }

        free(moar->element);
        free(moar->elementres);
        free(moar->statmods);
        free(moar->upgrades);
        free(moar->dmgtype);
        free(moar->bane);

    free(moar);
    
    return NULL;
}

void freeability(mechanic *ability)
{
    free(ability);
}

void freehero(player party)
{
    int i,j;

    printf("Freeing up the hero %s.\n",party.name);
    free(party.name);
    free(party.statchange);
    free(party.stats);
    free(party.effects);
    free(party.light);
    for (i = 0; i < 5; i++)
    {
        free(party.heavy[i].name);
    }
    free(party.heavy);
    party.skills = destroyskills(party.skills);
    party.boosts = freeextra(party.boosts);
    freeability(party.ability);
}

extra *createbonuses(extra *boosts)
{
    boosts = malloc(sizeof(extra));

    boosts->statmods = calloc(11,sizeof(float));
    boosts->upgrades = calloc(9,sizeof(int));
    boosts->element = calloc(9,sizeof(float));
    boosts->dmgtype = calloc(4,sizeof(float));
    boosts->bane = calloc(5,sizeof(float));
    boosts->elementres = calloc(9,sizeof(float));

    boosts->upgrades[0] = 100;
    boosts->statmods[0] = 0.3;

    return boosts;
}

float calcatkspd(player hero)
{
    float atkspd;
    atkspd = hero.stats[5] + hero.boosts->upgrades[5];
    atkspd *= (1 + hero.boosts->statmods[5]);
    atkspd = sqrt(atkspd/2) * 0.01;

    return atkspd;
}

combo setupcombo(combo sequence,float power, int hitcount, int element, int type, float time,int chargeable,char *str)
{
    sequence.power = power;
    sequence.hitcount = hitcount;
    sequence.element = element;
    sequence.type = type;
    sequence.time = time;
    sequence.chargeable = chargeable;
    sequence.name = malloc(sizeof(char) * 20 + 1);
    strcpy(sequence.name,str);

    return sequence;
}

void freeenemy(enemy npc)
{
    printf("Freeing %s\n",npc.name);
    free(npc.name);
    free(npc.statchange);
    free(npc.effects);
    free(npc.stats);
    printf("Freeing enemy skills.\n");
    npc.slots = destroyskills(npc.slots);  
    npc.boosts = freeextra(npc.boosts);
}

float *createstats(float HP, float ATK, float DEF, float MAG, float SPR, float DEX, float AGI, float CRI)
{
    float *stats = malloc(sizeof(float) * 9);
    stats[0] = HP;
    stats[1] = ATK;
    stats[2] = DEF;
    stats[3] = MAG;
    stats[4] = SPR;
    stats[5] = DEX;
    stats[6] = AGI;
    stats[7] = CRI;
    stats[8] = stats[0];

    return stats;
}

status *createeffect(status *effects, int id)
{
    switch(id)
    {
        case 1:
            effects = calloc(1,sizeof(status));
            effects->id = 17;
            effects->type = 1;
            effects->time = 30;
            effects->currtime = 30;
            effects->effect = 52;
            effects->chance = 100;
            effects->stack = 1;
            effects->apply = 1;
            break;
        case 2:
            effects = calloc(1,sizeof(status));
            effects->id = 3;
            effects->type = 2;
            effects->time = 30;
            effects->currtime = 30;
            effects->effect = -0.2;
            effects->chance = 50;
            effects->stack = 1;
            effects->apply = 2;
            break;
        case 3:
            effects = calloc(1,sizeof(status));
            effects->id = 6;
            effects->type = 2;
            effects->time = 30;
            effects->currtime = 30;
            effects->effect = 0.50;
            effects->chance = 80;
            effects->stack = 1;
            break;
        case 4:
            effects = calloc(1,sizeof(status));
            effects->id = 10;
            effects->type = 2;
            effects->time = 30;
            effects->currtime = 30;
            effects->effect = -0.2;
            effects->chance = 100;
            effects->stack = 1;
    }
}

float *specialskills(float *multi, int id, float atkmod)
{

    return multi;
}

skill initializeskill(skill slots, char *name,float mod,int element,float cooldown,int dmgtype, int special,float atktime, int hits, int cost)
{
    slots.skillname = calloc(20,sizeof(char));
    strcpy(slots.skillname,name);
    slots.sp = calloc(2,sizeof(float));
    slots.sp[1] = cooldown;
    slots.element = element;
    slots.time = atktime;
    slots.hitcount = hits;
    slots.cost = cost;
    slots.skillmod = mod;
    slots.dmgtype = dmgtype;
    slots.antiair = 0;

    if (special >= 1)
    {
        slots.multi_skillmod = calloc(hits,sizeof(float));
        slots.multi_skillmod = specialskills(slots.multi_skillmod,special, mod);
    }

    if (special == 0)
    {
        slots.skillmod = mod;
    }
    return slots;
}
skill *enemyskills(skill *slots, int id,int boss)
{
    if (boss == 0)
    {
        slots = calloc(3,sizeof(skill));
    }
    if (boss == 1)
    {
        slots = calloc(4,sizeof(skill));
    }
    if (boss == 2)
    {
        slots = calloc(5,sizeof(skill));
        //bossskills(slots);
    }

    switch(id)
    {
        case 1:
            slots[0] = initializeskill(slots[0],"Howl",0,0,10,0,0,1.15,1,0);
            slots[0].effects = createeffect(slots[0].effects,4);
            slots[0].type = 0;
            slots[1] = initializeskill(slots[1],"Scratch",1.1,0,0,1,0,1.5,1,0);
            slots[1].effects = NULL;
            slots[1].type = 1;
            slots[2] = initializeskill(slots[2],"Charge",2.1,0,0,1,0,3.00,1,0);
            slots[2].effects = NULL;
            slots[2].type = 1;
            slots[2].antiair = 1;
            break;
        case 2:
            slots[0] = initializeskill(slots[0],"Pierce",1.15,0,0,1,0,1.15,1,0);
            slots[0].effects = NULL;
            slots[0].type = 1;
            slots[1] = initializeskill(slots[1],"Flame Pillar",1.30,1,0,1,0,1.20,1,0);
            slots[1].effects = NULL;
            slots[1].type = 1;
            slots[1].antiair = 1;
            slots[2] = initializeskill(slots[2],"Flame Spiral",1.34,1,0,1,0,1.15,1,0);
            slots[2].effects = NULL;
            slots[2].type = 1;
            slots[3] = initializeskill(slots[3],"Dragon Descent",2.1,1,0,1,0,3.00,1,0);
            slots[3].effects = NULL;
            slots[3].type = 1;
            slots[3].antiair = 1;
            break;
        case 3:
            slots[0] = initializeskill(slots[0],"Vine-Grab",1.15,0,0,1,0,0.75,1,0);
            slots[0].antiair = 1;
            slots[0].effects = NULL;
            slots[1] = initializeskill(slots[1],"Vine-Spin",1.30,1,0,1,0,2.20,1,0);
            slots[1].effects = NULL;
            slots[1].type = 1;
            slots[2] = initializeskill(slots[2],"Fury",0,0,0,1,0,3.00,1,0);
            slots[2].effects = NULL;
            slots[2].type = 1;
            break;
    }

    return slots;
}
skill *createskills(skill *slots,int id)
{
    slots = calloc(3,sizeof(skill));
    switch(id)
    {
        case 1:
            slots[0] = initializeskill(slots[0],"Imbue Sword",0,2,2500,0,0,1.15,0,0);
            slots[0].effects = createeffect(slots[0].effects,1);
            slots[0].type = 0;
            slots[1] = initializeskill(slots[1],"Visxe",1.2,2,500,2,0,0.5,1,0);
            slots[1].effects = NULL;
            slots[1].type = 1;
            slots[2] = initializeskill(slots[2],"Dio",1.2,5,500,2,0,0.5,1,0);
            slots[2].type = 1;
            slots[2].effects = NULL;
            break;
        case 2:
            slots[0] = initializeskill(slots[0],"Storm Cutter",2.35,4,2000,2,0,1.2,1,0);
            slots[0].effects = createeffect(slots[0].effects,6);
            slots[0].type = 0;
            slots[1] = initializeskill(slots[1],"Visxe",1.2,2,500,2,0,0.5,1,0);
            slots[1].effects = NULL;
            slots[1].type = 1;
            slots[2] = initializeskill(slots[2],"Dio",1.2,5,500,2,0,0.5,1,0);
            slots[2].type = 1;
            slots[2].effects = NULL;
            break;

    }
    return slots;
}

extra *setupboosts(extra *boosts,int id)
{
    switch (id)
    {
        case 2:
            boosts->elementres[1] = 0.5;
            boosts->elementres[2] = -0.5;
            boosts->elementres[3] = 0.5;
            boosts->elementres[5] = -1;
            boosts->dmgtype[1] = 0.5;
            boosts->dmgtype[3] = -0.5;
            break;
    }

    return boosts;
}

player herotime(player hero, char *name, int level, int flag)
{
    int lvscal = 0,i, val = 1;
    float light;
    hero.level = level;
    lvscal = hero.level - 1;

    hero.name = malloc(sizeof(char) * 10 + 1);
    hero.statchange = calloc(9,sizeof(float));
    hero.light = calloc(5,sizeof(combo));
    hero.heavy = calloc(5,sizeof(combo));
    hero.stats = malloc(sizeof(float) * 9);
    hero.boosts = createbonuses(hero.boosts);
    hero.effects = calloc(17,sizeof(status));

    hero.effects->debuff = 0;
    hero.effects->buff = 0;

    for (i = 0; i < 17; i++)
    {
        hero.effects[i].id = val;
        val++;
    }
    hero.ability = calloc(1,sizeof(mechanic));


    strcpy(hero.name,name);

    switch(flag)
    {
        case 1:
        hero.skills = createskills(hero.skills,1);
        hero.stats[0] = 145 + ((1073.0/99.0)*lvscal);//index 0
        hero.origHP = hero.stats[0];
        hero.stats[0] =(hero.stats[0] + hero.boosts->upgrades[0])*(1 + hero.boosts->statmods[0]);
        hero.stats[8] = hero.stats[0];
        hero.stats[1] = 45 + ((480.0/99.0)*(lvscal));// index 1
        hero.stats[2] = 36 + ((322.0/99.0)*lvscal);// index 2
        hero.stats[3] = 41 +((182.0/99.0)*lvscal);// index 3
        hero.stats[4] = 33 + ((145.0/99.0)*lvscal);// index 4
        hero.stats[5] = 50 + ((310.0/99.0)*lvscal);// index 5
        hero.stats[6] = 63 + ((129.0/99.0)*lvscal);// index 6
        hero.low = 90;
        hero.high = 110;
        hero.atkspd = calcatkspd(hero);
        light = 0.7;
        hero.ability->charges = 0;

        for(i = 0; i < 5; i++)
        {
            hero.light[i] = setupcombo(hero.light[i],light,1,0,1,0.8,0,"");
            free(hero.light[i].name);
            light += 0.15;
            hero.light[i].spgain = 130;
            hero.heavy[i].spgain = 200;
        }

        hero.light[4].time =1.1;
        hero.heavy[0] = setupcombo(hero.heavy[0],1.50,3,2,2,2.2,1,"Whirling Splash");// Neutral
        hero.heavy[1] = setupcombo(hero.heavy[1],1.10,5,2,2,1.3,0,"Bubbling Punch");// Forward
        hero.heavy[2] = setupcombo(hero.heavy[1],1.15,1,2,2,1.1,0,"Aqua Wave");// Back
        hero.heavy[3] = setupcombo(hero.heavy[3],1.10,1,5,2,1.1,1,"Ice Claw");// Left
        hero.heavy[4] = setupcombo(hero.heavy[4],1.30,5,5,2,1.1,0,"Frost Crash");// Right
        
        break;

        case 2:
        hero.skills = createskills(hero.skills,1);
        hero.stats[0] = 180 + ((1333.0/99.0)*lvscal);
        hero.origHP = hero.stats[0];
        hero.stats[0] =(hero.stats[0] + hero.boosts->upgrades[0])*(1 + hero.boosts->statmods[0]);
        hero.stats[8] = hero.stats[0];
        hero.stats[1] = 53 + ((500.0/99.0)*lvscal);
        hero.stats[2] = 48 + ((345.0/99.0)*lvscal);
        hero.stats[3] = 58 + ((465.0/99.0)*lvscal);
        hero.stats[4] = 42 + ((219.0/99.0)*lvscal);
        hero.stats[5] = 48 + ((292.0/99.0)*lvscal);
        hero.stats[6] = 58 + ((122.0/99.0)*lvscal);

        hero.low = 90;
        hero.high = 110;
        hero.atkspd = calcatkspd(hero);

        light = 0.8;
        hero.ability->id = 1;
        hero.ability->boreal_energy = 0;

        for(i = 0; i < 5; i++)
        {
            hero.light[i] = setupcombo(hero.light[i],light,1,0,1,0.6,0,"");
            free(hero.light[i].name);
            light += 0.10;
            hero.light[i].spgain = 150;
            hero.heavy[i].spgain = 200;
        }

        hero.light[2].spgain = 220;
        hero.light[2].hitcount = 2;
        hero.light[3].hitcount = 3;
        hero.light[3].spgain = 360;
        hero.light[4].spgain = 900;
        hero.light[4].time = 1.1;

        hero.heavy[0] = setupcombo(hero.heavy[0],1.65,1,4,1,1.5,1,"Flash");// Neutral
        hero.heavy[1] = setupcombo(hero.heavy[1],1.35,1,4,1,1.3,0,"Backpedal");// Forward
        hero.heavy[2] = setupcombo(hero.heavy[1],1.15,1,4,1,1.1,1,"Storm Lashes");// Back
        hero.heavy[3] = setupcombo(hero.heavy[3],1.10,1,4,1,1.1,0,"Shock Blast");// Left
        hero.heavy[4] = setupcombo(hero.heavy[4],0.50,5,4,1,1.1,0,"Sparks");// Right
        hero.ability->id = 2;
        hero.ability->charges = 0;
        break;
    }

    hero.stats[7] = 5;
    hero.mvspd = hero.stats[6] * 0.02;
    hero.skills->numskills = 3;

    return hero;
}

enemy stat(enemy npc, char *str, float *inputs,int level,int boss,int type)
{
    int length,i,val = 1;
    if(type == 0)
    {
        return npc;
    }

    length = strlen(str);
    npc.name = malloc((sizeof(char) * length)+ 1);
    npc.ready = 0;
    npc.pos = 1;
    npc.airtime = 0;
    npc.condition = 0;
    strcpy(npc.name, str);
    npc.stats = malloc(sizeof(float)* 11);
    npc.effects = calloc(13,sizeof(status));
    npc.target = 0;

    npc.effects->debuff = 0;
    npc.effects->buff = 0;
    
    for(i = 0 ; i < 11; i++)
    {
        npc.stats[i] = inputs[i];
    }

    for (i = 0; i < 13; i++)
    {
        npc.effects[i].id = i+1;
    }
    npc.mvspd = 0.02 * npc.stats[6];
    npc.cr = 50.0;
    npc.crgain = (npc.stats[6] / 10) * 2;
    npc.remaintime = 100/npc.crgain;
    npc.statchange = calloc(9,sizeof(float));
    npc.level = level;
    npc.boss = boss;
    npc.boosts = createbonuses(npc.boosts);
    npc.boosts = setupboosts(npc.boosts,2);
    npc.boosts->elementres[2] = 0.5;
    npc.boosts->elementres[5] = -0.3;

    if(npc.boss == 2)
    {
        npc.mode = 0.20 * npc.stats[0];
        npc.modebar = 0;
    }
    else
    {
        npc.mode = 0;
        npc.mode = 0;
    }

    printf("Created enemy %s\n",npc.name);
    
    return npc;
}

enemy createenemy(enemy foe, int choice)
{
    float *stats;

    switch(choice)
    {
        case 1:
            stats = createstats(1000,45,30,25,30,50,90,0);
            foe.id = 1;
            foe = stat(foe,"Dire Wolf",stats,5,0,1);
            foe.slots = enemyskills(foe.slots,choice,0);
            foe.slots->numskills = 3;
            foe.pattern =32322;
            foe.armor = 0;
            foe.ai = 1;
            break;
        case 2:
            stats = createstats(336,154,85,68,45,49,100,5);
            foe.id = 2;
            foe = stat(foe,"Achebe",stats,10,0,1);
            foe.slots = enemyskills(foe.slots,choice,1);
            foe.slots->numskills = 4;
            foe.pattern = 43211;
            foe.armor = 1000;
            foe.ai = 2;
            break;
        case 3:
            // Elijah, Steve, Cyrus, Cassandra, and Alicia
            stats = createstats(3500,60,60,68,45,49,100,5);
            foe.id = 3;
            foe = stat(foe,"Corpse Vine",stats,10,0,1);
            foe.slots = enemyskills(foe.slots,choice,0);
            foe.slots->numskills = 3;
            foe.pattern = 22223;
            foe.armor = 0;
            break;
    }

    free(stats);

    return foe;
}

status cleareffect(status buff, float **statchange)
{
    float *statalter = *statchange;
    int index;

    if (buff.id >= 9 && buff.id < 16)
    {
        index = buff.id - 9;

        if (buff.effect < 0)
        {
            statalter[index] += -(buff.effect);
            buff.debuff--;
        }
        if (buff.effect > 0)
        {
            statalter[index] -= buff.effect;
            buff.buff--;
        }
        
    }
    buff.currtime = 0;
    buff.time = 0;
    buff.effect = 0;
    buff.type = 0;
    buff.stack = 0;
    
    return buff;
}

void afflictions(status **effects,float **statchange, status afflicted)
{
    status *debuff = *effects;
    float *statalter = *statchange;
    int index, check = 0;

    debuff[afflicted.id - 1].type = afflicted.type;
    debuff[afflicted.id - 1].time = afflicted.time;
    debuff[afflicted.id - 1].currtime = afflicted.time;
    debuff[afflicted.id - 1].stack += afflicted.stack;
    debuff[afflicted.id - 1].effect += afflicted.effect;

    if (afflicted.id >= 9 && afflicted.id < 16)
    {
        index = afflicted.id - 9;
        statalter[index] += afflicted.effect;
        
        if (debuff[afflicted.id - 1].effect > 0)
            check = 1;
        if (debuff[afflicted.id - 1].effect < 0)
            check = 2;

        debuff[afflicted.id - 1].effect += afflicted.effect;

        if (debuff[afflicted.id - 1].effect < 0)
        {
            (debuff->debuff)++;

            if (check == 1)
                (debuff->buff)--;
        }
        if (debuff[afflicted.id - 1].effect > 0)
        {
            (debuff->buff)++;

            if (check == 2)
                (debuff->debuff)--;
        }
        return;

    }

    if (afflicted.type == 1)
    {
        (debuff->buff)++;
    }

    if (afflicted.type == 2)
    {
        (debuff->debuff)++;
    }
}
void printdebuffnames (status effects)
{
    if (effects.stack == 0)
        return;
    
    switch(effects.id)
    {
        case 1:
            if (effects.stack < 5)
                printf("BRN (x%d) ",effects.stack);
            if (effects.stack >= 5)
                printf("BLZ ");
            break;
        case 2:
        if (effects.stack < 5)
            printf("Bog x%d ",effects.stack);
        if (effects.stack == 5)
            printf("Swamp ");
        break;

        case 3:
        if (effects.stack < 5)
            printf("Frost x%d ",effects.stack);
        if (effects.stack == 5)
            printf("FRZN ");
        break;

        case 6:
        if (effects.stack < 5)
            printf("PARA x%d ",effects.stack);
        if (effects.stack == 5)
            printf("Elec ");
        break;
    }
}
void printdebuffs(int id, int stack)
{
    switch(id)
    {
        case 1:
        if (stack < 5)
            printf("Burn x%d",stack);
        if (stack == 5)
            printf("Immolant");
        break;

        case 2:
        if (stack < 5)
            printf("Bogged x%d",stack);
        if (stack == 5)
            printf("Swamped");
        break;

        case 3:
        if (stack < 5)
            printf("Frostbite x%d",stack);
        if (stack == 5)
            printf("Frozen!");
        break;

        case 6:
        if (stack < 5)
            printf("Paralyzed x%d",stack);
        if (stack == 5)
            printf("Electrified");
    }
    
}

void elementimbue (player *character, int choice, status imbue, int type)
{
    player hero = *character;
    int i,element,mod = imbue.effect;

    if (type == 1)
    {
        if (choice == 1)
            mod /= 10;
        
        element = mod % 10;
        printf("Imbued with the power of ");
        printelement(element);
        printf("\n");
        for (i = 0; i < 5; i++)
        {
            hero.light[i].element = element;
        }
        afflictions(&(character->effects),&(character->statchange),imbue);
    }

    if (type == 0)
    {
        for (i = 0; i < 5; i++)
        {
            hero.light[i].element = 0;
        }
        printf("Removed elemental effect.\n");
        hero.effects[16] = cleareffect(character->effects[16],&(character->statchange));
    }
}

void effectcheck(player *party,float time)
{
    int i;
    player hero = *party;
    status imbue;

    if (strcmp("Ayaz",hero.name) == 0)
    {
        imbue = hero.skills[0].effects[0];
    }
    
    for (i = 0; i < 17; i++)
    {
        hero.effects[i].time -= time;
        if (hero.effects[i].time <= 0 && hero.effects[i].id != 17)
        {
            hero.effects[i] = cleareffect(hero.effects[i],&(hero.statchange));
        }
        if (hero.effects[i].time <= 0 && hero.effects[i].id != 17)
        {
            elementimbue(party,0,imbue,0);
        }
    }
}

void skillcharge(skill **set, float sp)
{
    skill *slot = *set;
    int i;

    
    for (i = 0; i < 3; i++)
    {
        if (slot[i].charge != 1)
            slot[i].sp[0] += sp;

        if (slot[i].sp[0] >= slot[i].sp[1] && slot[i].charge != 1)
        {
            slot[i].charge = 1;
            printf("%s is ready!\n",slot[i].skillname);
        }
    }
}

float globaltime (player **heroes, enemy **foes, float time)
{
    int i,j;
    player *party = *heroes;
    enemy *threats = *foes;
    float leftover = 0;
    
    for (i = 0; i < threats[0].size; i++)
    {
        if ((threats[i].condition) & 1 || threats[i].ready)
            continue;
        
        if (threats[i].remaintime < time)
        {
            leftover = time - threats[i].remaintime;
            threats[i].cr = 0;
            threats[i].remaintime = 100/threats[i].crgain;
            threats[i].ready = 1;

            continue;
        }

        printf("%s can move %.2f spaces\n",threats[i].name,threats[i].mvspd * time);
        
        threats[i].cr += (threats[i].crgain * 0.5) * time;
        threats[i].remaintime = (100 - threats[i].cr)/threats[i].crgain;
    }

    return leftover;
}

float enemyvsplayer(player **party, enemy **horde,int cur,int condition, float time)
{
    int choice = 0,i,j,attack,fail = 0,light,flag = 0,flag2 = 0;
    float damage,cast = 0,risk = 0,atkspd;
    player *team = *party;
    enemy *foe = *horde;

    atkspd = (1 - team[cur].atkspd);

    light = team[cur].light->chain;

    for (i = 0; i < foe[0].size; i++)
    {
        if (foe[i].ready == 1)
        {
            if (foe[i].pos > 10000)
                    foe[i].pos = 1;
            
            attack = foe[i].pattern/foe[i].pos;
            attack = attack % 10;
            foe[i].pos *= 10;
            
            if (time >= foe[i].slots[attack - 1].time)
            {
                cast = time - foe[i].slots[attack - 1].time;
                flag = 0;
                printf("Too slow!\n");
            }
            if (time < foe[i].slots[attack - 1].time)
            {
                cast = foe[i].slots[attack - 1].time - time;
                flag = 1;
            }


            if (condition != 0 && flag)
            {
                printf("E%d %s is about to use %s in %.2f seconds\n",i + 1,foe[i].name,foe[i].slots[attack-1].skillname,cast);
                printf("Did the attack connect? 1) Connect 2) Perfect Dodge 3) Attack\n");
                scanf("%d",&choice);
            }

            else
            {
                choice = 1;
            }
            
            if (choice == 3)
            {
                printf("What attack was used?\n");
                printf("1) Light Attack (Chain x%d) 2) Elemancy Attack \n",team[cur].light->chain);
                scanf("%d",&choice);

                if (choice == 1)
                {
                    if (team[cur].light[0].time * atkspd <= cast)
                    {
                        if (foe[i].armor <= 0 || team[cur].light[0].power > foe[i].armor)
                        {
                            printf("%s interrupted E%d %s's attack! (+20%c Damage Boost!)\n",team[cur].name,i + 1,foe[i].name,percent);
                            risk = 0.2;
                        }
                        if (foe[i].armor > 0)
                        {
                            printf("%s receives additional damage for his folly!\n",team[cur].name);
                            damage = damagecalc(foe[i],team[cur],foe[i].slots[attack - 1].skillmod + 0.15,3,foe[i].slots[attack - 1].element,1,1);
                            team[cur].stats[8] -= damage;
                        }
                        damage = damagecalc(foe[i],team[cur],team[cur].light[0].power + risk,1,0,1,team[cur].light[0].hitcount);

                        foe[i].stats[8] -= damage;

                    }

                    if (team[cur].light[0].time * atkspd > cast)
                    {
                        printf(">> E%d %s interrupted %s's attack! (+20%c Damage Boost!)\n",i + 1,foe[i].name,team[cur].name,percent);
                        damage = damagecalc(foe[i],team[cur],foe[i].slots[attack - 1].skillmod + 0.20,3,foe[i].slots[attack - 1].element,1,1);
                        team[cur].stats[8] -= damage;
                    }
                    choice = 4;
                }

                if (choice == 2)
                {
                    if (team[cur].heavy[0].time * atkspd< cast)
                    {
                        if (foe[i].armor <= 0)
                        {
                            printf("%s interrupted E%d %s's attack! (+50%c Damage Boost!)\n",team[cur].name,i + 1,foe[i].name,percent);
                            risk = 0.5;
                        }
                        if (foe[i].armor > 0)
                        {
                            printf("%s receives additional damage for his folly!\n",team[cur].name);
                            damage = damagecalc(foe[i],team[cur],foe[i].slots[attack - 1].skillmod + 0.2,3,foe[i].slots[attack - 1].element,1,1);
                            team[cur].stats[8] -= damage;
                        }
                        damage = damagecalc(foe[i],team[cur],team[cur].heavy[0].power + risk,1,1,1,team[cur].heavy[0].hitcount);
                        foe[i].stats[8] -= damage;

                    }

                    if (team[cur].heavy[0].time * atkspd > cast)
                    {
                        printf(">> E%d %s interrupted %s's attack! (+30%c Damage Boost!)\n",i + 1,foe[i].name,team[cur].name,percent);
                        damage = damagecalc(foe[i],team[cur],foe[i].slots[attack - 1].skillmod + 0.30,3,foe[i].slots[attack - 1].element,1,1);
                        team[cur].stats[8] -= damage;
                    }
                    choice = 4;
                }

                choice = 4;
            }

            if (choice == 1)
            {
                damage = damagecalc(foe[i],team[cur],foe[i].slots[attack - 1].skillmod,3,foe[i].slots[attack - 1].element,1,1);
                
                team[cur].stats[8] -= damage;
                if (foe[i].id == 2)
                {
                    if (foe[i].pos > 10000)
                    foe[i].pos = 1;
            
                    attack = foe[i].pattern/foe[i].pos;
                    attack = attack % 10;
                    foe[i].pos *= 10;
                    printf("E%d ",i + 1);
                    damage = damagecalc(foe[i],team[cur],foe[i].slots[attack - 1].skillmod,3,foe[i].slots[attack - 1].element,1,1);
                    party[cur]->stats[8] -= damage;
                }
            }
            if (choice != 1)
            {
                foe[i].cr -= 10;
                foe[i].remaintime = (100 - foe[i].cr)/foe[i].crgain;
                printf("Attack failed.\n");
            }

            globaltime(party,horde,foe[i].slots[attack - 1].time);
            foe[i].ready = 0;
        }
    }
    return 0;
}

void checkstatus (player *party, enemy *horde)
{
    int i,j;

    for (i = 0; i < 2; i++)
    {
        printf("%s HP: %.0f/%.0f\n",party[i].name,party[i].stats[8],party[i].stats[0]);
        for (j = 0; j < 3; j++)
        {
            printf("S%d ",j + 1);
            if (party[i].skills[j].sp[0] == party[i].skills[j].sp[1])
                printf("(READY!) ");
            else
                printf("(%.0f%c) ",100*(party[i].skills[j].sp[0]/party[i].skills[j].sp[1]),percent);
        }
        printf("\n");
        printf("Status Effects:\n");

        if (party[i].effects->buff != 0 || party[i].effects->debuff != 0)
        {
            for (j = 0; j < 17; j++)
            {
                printdebuffnames(party[i].effects[j]);
            }
        }
        printf("===========================================\n");
    }

}

float move(player **party,enemy **horde,int cur)
{
    player *hero = *party;
    enemy *foe = *horde;
    int i,choice = 0,choice2 = 0,x,y,newx,newy, flag = 0, distance;
    float time = 0,currtime,leftover = 0;

    hero[cur].mvspd = (hero[cur].stats[6] * 0.02) * (1 + hero[cur].boosts->statmods[10]);

    while (choice != 3)
    {
        printf("---------------------------------------\n");
        for(i = 0; i < foe[0].size; i++)
        {
            printf("%s Time: %.2f seconds remain\n",foe[i].name,foe[i].remaintime);
        }
        printf("---------------------------------------\n");

        printf("\n");
        printf("Current position (%d,%d)\n",hero[cur].x,hero[cur].y);
        printf("How will you move?\n");
        printf("1) Triangular 2) End Move\n");
        scanf("%d",&choice);

        if (choice == 2)
        {
            printf("%s current position is (%d,%d)\n",hero[cur].name,hero[cur].x,hero[cur].y);
            return time;
        }
        while (choice2 != 1)
        {
            switch(choice)
            {
                case 1:
                    printf("What position will %s move to?\n",hero[cur].name);
                    printf("Current Position: (%d,%d)\n",hero[cur].x,hero[cur].y);
                    scanf("%d %d",&x,&y);

                    while (x > 10)
                    {
                        printf("Invalid x position.\n");
                        scanf("%d",&x);
                    }

                    while (y > 14)
                    {
                        printf("Invalid y position.\n");
                        scanf("%d",&y);
                    }
                    
                    newx = hero[cur].x - x;
                    if (newx < 0)
                        newx *= -1;

                    newy = hero[cur].y - y;
                    if (newy < 0)
                        newy *= -1;

                    currtime = sqrt((newx*newx) + (newy*newy))/hero[cur].mvspd;

                    printf("This move will take %.2f seconds. Confirm?\n",currtime);
                    printf("1 = Yes 2 = No\n");
                    scanf("%d",&choice2);

                    if (choice2 == 1)
                    {
                        leftover = globaltime(party,horde,currtime);

                        if (leftover == 0)
                        {
                            hero[cur].x = x;
                            hero[cur].y = y;
                            time += leftover;
                            printf("%s new position is (%d,%d)\n",hero[cur].name,hero[cur].x,hero[cur].y);
                        }

                        if (leftover != 0)
                        {
                            printf("Intercepted after moving %.0f spaces\n",leftover*hero[cur].mvspd);
                            printf("%s new position is: \n",hero[cur].name);
                            scanf("%d %d",&x,&y);

                            while (x > 10)
                            {
                                printf("Invalid x position.\n");
                                scanf("%d",&x);
                            }

                            while (y > 14)
                            {
                                printf("Invalid y position.\n");
                                scanf("%d",&y);
                            }

                            hero[cur].x = x;
                            hero[cur].y = y;
                            printf(" Pos: (%d,%d)\n",hero[cur].x,hero[cur].y);

                            enemyvsplayer(party,horde,cur,1,leftover);
                        }
                        time += leftover;
                        leftover = 0;
                    }
                    choice = 0;
                    return time;

                    case 2:
                        return time;
                   
            }
            printf("===========================================\n");
            printf("\n");
        }
        choice2 = 0;
    }

    return time;
}

int targetting(enemy **horde,int action)
{
    enemy *foe = *horde;
    int choice,targets = 0,i,size = foe[0].size;

    if (action == 0)
    {
        for (i = 0; i < size; i++)
            foe[0].target = 0;
        
        return 0;
    }

    for (i = 0; i < size; i++)
    {
        if (foe[i].stats[8] <= 0)
            printf("%d) %s (DEAD!)\n",i+1,foe[0].name);
        if(foe[i].stats[8] > 0)
            printf("%d) %s\n",i+1,foe[0].name);
    }

    printf("How many targets? Enemy Size: %d\n",size);
    
    scanf("%d",&choice);

    if (choice == 0)
        return 0;
    
    if (choice >= size)
    {
        choice = size;
        for (i = 0; i < size; i++)
            foe[i].target = 1;
        return choice;
    }

    printf("Which targets do you select?\n");

    for(i = 0; i < choice; i++)
    {
        scanf("%d",&targets);

        if (targets > size)
            targets = size;

        if (targets < 1)
            targets = 1;
        
        foe[targets - 1].target = 1;
        printf("%s has been targeted.\n",foe[targets - 1].name);
    }

    printf("Enemy Horde Size is %d.\n",foe[0].size);
    
    return choice;
}

int comboattack(player **hero, enemy **horde, int curr)
{
    int choice, i,chargeable,hitcount,element,dash = 0,type,light = 0,heavy,hits = 0;
    int player_air = 0,knockdown = 0, targets = 1;
    float damage,total_damage = 0,power,time,total,leftover,airtime = 0,atkspd;
    float resisttimer = 0;
    player *heroes = *hero;
    enemy *foe = *horde;
    int size = foe[0].size;

    foe[0].target = 1;

    while (choice != 8)
    {
        atkspd = (1 - heroes[curr].atkspd);
        for (i = 0; i < size; i++)
        {
            if (foe[i].condition)
                printf("%s is in the air.\n",foe[i].name);
            if (foe[i].airtime < 0.7 && foe[i].condition)
                printf("%s is about to hit the ground.\n",foe[i].name);

            printf("E%d %s Remaining Time: %.2f Seconds \n",i+1,foe[i].name,foe[i].remaintime);
        }


        if (targets == 0)
            printf("No targets selected\n");
        
        if (player_air)
            printf("%s is airborne!\n",heroes[curr].name);

        printf("===========================================\n");
        printf("Current Actions:\n");
        printf("1) Light Attack 2) Elemancy 3) Skills\n");
        printf("4) Dash 5) Jump 6) Switch 7) Target\n");
        scanf("%d",&choice);
        printf("===========================================\n");

        if (choice == 8)
            return curr;
        
        switch(choice)
        {
            case 1:
                if (targets == 0)
                {
                    printf("No targets selected.\n");
                    break;
                }

                power = heroes[curr].light[light].power;
                type = heroes[curr].light[light].type;
                element = heroes[curr].light[light].element;
                hitcount = heroes[curr].light[light].hitcount;
                time = heroes[curr].light[light].time * atkspd;

                leftover = globaltime(hero,horde,time);

                if (foe[0].ready != 0)
                {
                    printf("%s strikes!\n",foe[0].name);
                    enemyvsplayer(hero,horde,curr,2,leftover);
                    return curr;
                }
                if ((foe[0].condition & 1) && player_air == 0 && time < airtime)
                {
                    printf("Swing missed\n");
                    airtime -= time;
                    total += time;
                    break;
                }

                for (i = 0 ; i < foe[0].size ; i++)
                {
                    if (foe[i].target == 1)
                    {
                        damage = damagecalc(foe[i],heroes[curr],power,type,element,1,hitcount);
                        total_damage += damage;
                        foe[i].stats[8] -= damage;
                    }
                }
                
                skillcharge(&(heroes[curr].skills),heroes[curr].light[light].spgain);
                printf("%.3f seconds.\n",time);
                hits += hitcount;
                printf("%d Hit!\n",hits);
                total += time;

                // This is the person used the launch attack
                if (dash == 1 && light == 0 && knockdown == 0)
                {
                    for (i = 0; i < foe[0].size; i++)
                    {
                        if (foe[i].target)
                            printf("E%d %s has been launched.\n",i+1,foe[i].name);
                            foe[i].condition ^= 1;
                            foe[i].airtime += 2;
                    }
                    dash = 0;
                }
                if (knockdown == 1)
                    printf("Target can't be launched.\n");
                
                for (i = 0; i < foe[0].size; i++)
                {
                    if ((foe[i].condition) & 1)
                        foe[i].airtime += time + 0.3;
                }
                
                if (light == 4 && player_air == 1)
                {
                    leftover = globaltime(hero,horde,2);
                    if (foe[0].ready != 0)
                    {
                        printf("%s is being intercepted in the air!\n",heroes[curr].name);
                        printf("%s strikes!\n",foe[0].name);
                        enemyvsplayer(hero,horde,curr,0,leftover);
                        return curr;
                    }
                    printf("%s has touched the ground.\n",heroes[curr].name);
                    player_air = 0;
                    light = 0;
                    heroes[curr].light->chain = 0;
                }
                if (resisttimer > 0)
                    resisttimer -= time;

                light++;
                heroes[curr].light->chain++;
                
                break;

                case 2:
                    printf("Which elemancy skill\n");
                    for (i = 0; i < 5; i++)
                    {
                        printf("%d) %s\n",i + 1,heroes[curr].heavy[i].name);
                    }
                    scanf("%d",&heavy);

                    while (heavy > 5 || heavy < 1)
                    {
                        printf("Which elemancy skill\n");
                        scanf("%d",&heavy);
                    }
                    heavy--;
                    power = heroes[curr].heavy[heavy].power;
                    type = heroes[curr].heavy[heavy].type;
                    element = heroes[curr].heavy[heavy].element;
                    hitcount = heroes[curr].heavy[heavy].hitcount;
                    chargeable = heroes[curr].heavy[heavy].chargeable;
                    time = heroes[curr].heavy[heavy].time * atkspd;

                    if (chargeable == 1)
                    {
                        printf("Would you like to charge it (0 No) (1 Yes)? (Takes a little longer, but hits harder)\n");
                        scanf("%d",&choice);

                        if (choice == 1)
                        {
                            power += 0.8;
                            time += 1;
                            hitcount += 2;
                        }

                        if (foe[0].condition & 1)
                            airtime -= time;
                    }

                    if ((foe[0].condition & 1) && player_air == 0 && time < airtime)
                    {
                        printf("Attack missed\n");
                        airtime -= time;
                        total += time;
                        break;
                    }

                    leftover = globaltime(hero,horde,time);

                    if (foe[0].ready != 0)
                    {
                        printf("%s strikes!\n",foe[0].name);
                        enemyvsplayer(hero,horde,curr,3,leftover);
                        return curr;
                    }

                    skillcharge(&(heroes[curr].skills),heroes[curr].heavy[heavy].spgain);

                    for (i = 0 ; i < foe[0].size ; i++)
                    {
                        if (foe[i].target == 1)
                        {
                            damage = damagecalc(foe[i],heroes[curr],power,type,element,1,hitcount);
                            total_damage += damage;
                            foe[i].stats[8] -= damage;
                        }
                    }

                    printf("%.3f seconds.\n",time);
                    hits += hitcount;
                    printf("%d Hit!\n",hits);
                    total += heroes[curr].heavy[heavy].time;
                    light = 0;
                    heroes[curr].light->chain = 0;

                    if (player_air == 1)
                    {
                        leftover = globaltime(hero,horde,2);
                        if (foe[0].ready != 0)
                        {
                            printf("%s is being intercepted in the air!\n",heroes[curr].name);
                            printf("%s strikes!\n",foe[0].name);
                            enemyvsplayer(hero,horde,curr,0,leftover);
                            return curr;
                        }
                        airtime += time + 0.5;
                        player_air = 0;
                    }

                    if (resisttimer > 0)
                        resisttimer -= time;

                    break;
                    
                    case 4:
                        printf("You are dashing\n");
                        dash = 1;
                        break;
                    case 5:
                        if (player_air == 1)
                        {
                            leftover = globaltime(hero,horde,time);
                            printf("%s force lands.\n",heroes[curr].name);
                            player_air = 0;
                            if (foe[0].ready != 0)
                            {
                                printf("%s is being intercepted in the air!\n",heroes[curr].name);
                                printf("%s strikes!\n",foe[0].name);
                                enemyvsplayer(hero,horde,curr,0,leftover);
                                return curr;
                            }
                            break;
                        }
                        printf("%s is airborne.\n",heroes[curr].name);
                        light = 0;
                        heroes[curr].light->chain = 0;
                        player_air = 1;
                        break;
                    case 6:
                        curr = !curr;
                        printf("Switched to %s\n",heroes[curr].name);
                        break;
                    case 7:
                        targets = targetting(horde,1);
                        break;
        }

        printf("There are %d targets\n",targets);
        if (light >= 5)
        {
            light = 0;
            heroes[curr].light->chain = 0;
        }
        
        if ((foe[0].condition & 1) && airtime <= 0)
        {
            printf("%s has hit the ground.\n",foe[0].name);
            knockdown = 1;
            airtime = 0;
            resisttimer = 10;
            foe[0].condition ^= 1;
        }
        if (foe[0].stats[8] <= 0 || foe[0].stats[8] <= 0)
        {
            printf("%s dealt a total of %.0f damage.\n",heroes[0].name,total_damage);
            return curr;
        }
        printf("===========================================\n");
    }

    return curr;
}


void battle (player **hero,enemy **horde)
{
    int i = 0,j,id,choice = 0,choice2 = 0,light = 0, heavy = 0,curr = 0, hits = 0,x = 0, y = 0,confirm = 0;
    float damage = 0,total_damage = 0,time,total = 0,chance, charge;
    float power, type, element,hitcount,chargeable,leftover,atkspd;
    int target = 0,numtarget = 1, capture = 0;
    player *heroes = *hero;
    skill chosenskill;
    enemy *foe = *horde;
    
    printf("Remaining Time: %.2f seconds\n",foe[0].remaintime);
    printf("What will you do?\n");
    for(i = 0; i < 5; i++)
    {
        atkspd = (1 - heroes[curr].atkspd);
        printf("===========================================\n");
        printf("%s HP: %.0f/%.0f\n",heroes[curr].name,heroes[curr].stats[8],heroes[curr].stats[0]);
        for (j = 0; j < 3; j++)
        {
            printf("S%d ",j + 1);
            if (heroes[curr].skills[j].sp[0] == heroes[curr].skills[j].sp[1])
                printf("(READY!) ");
            else
                printf("(%.0f%c) ",100*(heroes[curr].skills[j].sp[0]/heroes[curr].skills[j].sp[1]),percent);
        }
        printf("\n");
        for(j = 0; j < foe[0].size; j++)
            printf("%s Remaining Time: %.2f\n",foe[j].name,foe[j].remaintime);

        printf("\n");
        printf("Current Position (%d,%d)\n",heroes[curr].x,heroes[curr].y);

        printf("1 = Attack\n");
        printf("3 = Switch 4 = Use Skill 5 = Move 6 = Run\n");
        printf("7) Current Status\n");
        scanf("%d",&choice);

        if (choice == 6)
        {
            printf("You ran away.\n");
            return;
        }

        while (choice > 7 || choice < 1)
        {
            printf("1 = Attack 2 = Soul Skills\n");
            printf("3 = Switch 4 = Use Skill 5 = Move 6 = Run\n");
            printf("7 Enemy HP 8 = Switch Targets\n");
            scanf("%d",&choice);
        }
        printf("===========================================\n");

        switch (choice)
        {
            case 1:
                curr = comboattack(hero,horde,curr);
                break;
            
            case 3:
                if (curr == 0)
                {
                    curr = 1;
                    printf("Switched to %s.\n",heroes[curr].name);
                    break;
                }
                if (curr == 1)
                {
                    curr = 0;
                    printf("Switched to %s.\n",heroes[curr].name);
                    break;
                }
            case 4:
                printf("Which skill will you use?\n");
                for (j = 0; j < 3; j++)
                {
                    printf("S%d) %s",j+1,heroes[curr].skills[j].skillname);
                    if (heroes[curr].skills[j].charge == 1)
                    {
                        printf(" READY!!\n");
                    }
                    if (heroes[curr].skills[j].charge == 0)
                    {
                        charge = heroes[curr].skills[j].sp[0]/heroes[curr].skills[j].sp[1];
                        charge *= 100;
                        printf("(%.1f %c)\n",charge,percent);
                    }
                    
                }
                scanf("%d",&choice);
                if (choice > 3 || choice < 1)
                {
                    break;
                }

                chosenskill = heroes[curr].skills[choice - 1];
                printf("Ayaz used %s.\n",chosenskill.skillname);
                printf("===========================================\n");
                chosenskill.sp[0] = 0;
                chosenskill.charge = 0;
                time = chosenskill.time;

                leftover = globaltime(hero,horde,time);
                if (foe[0].ready != 0)
                {
                    printf("%s strikes!\n",foe[0].name);
                    enemyvsplayer(hero,horde,curr,1,leftover);
                    break;
                }

                if (chosenskill.type == 0)
                {
                    if (chosenskill.element != 0)
                    {
                        printf("Will you charge it up for 1 Boreal Energy? 1) Yes 0) No\n");
                        scanf("%d",&choice);

                        while (choice > 1 || choice < 0)
                        {
                            printf("Will you charge it up for 1 Boreal Energy?\n");
                            scanf("%d",&choice);
                        }
                        
                        elementimbue(&(heroes[curr]),choice,chosenskill.effects[0], 1);
                    }
                    break;
                }
                
                if (heroes[curr].skills[choice - 1].type == 1)
                {
                    type = heroes[curr].skills[choice - 1].dmgtype;
                    power = heroes[curr].skills[choice - 1].skillmod;
                    element = heroes[curr].skills[choice - 1].element;
                    hitcount = heroes[curr].skills[choice - 1].hitcount;
                    damage = damagecalc(foe[0],heroes[curr],power,type,element,0,hitcount);
                }

                chance = rand()%100 + 1;

                if (heroes[curr].skills[choice - 1].effects != NULL)
                {
                    if (heroes[curr].skills[choice - 1].effects[0].chance <= chance)
                    {
                        printf("%s has been afflicted with ",foe[0].name);
                        afflictions(&(foe[0].effects),&(foe[0].statchange),chosenskill.effects[0]);
                        printdebuffs(chosenskill.effects[0].id,chosenskill.effects[0].stack);
                        printf("\n");
                    }

                    id = chosenskill.effects[0].id;

                    if (foe[0].effects[id - 1].stack >= 5)
                    {
                        printf("The target is ");
                        printdebuffs(foe[0].effects[id - 1].id,foe[0].effects[id - 1].stack);
                        printf("\n");
                    }

                }

                break;

                case 5:
                    time = move(hero,horde,curr);
                    break;
                
                case 6:
                    printf("You ran away\n");
                    return;
                case 7:
                    for (j = 0; j < 2; j++)
                        printf("%s HP: %.0f/ %.0f\n",foe[j].name,foe[j].stats[8],foe[j].stats[0]);
                    //checkstatus(heroes,foe);
                    break;


            printf("===========================================\n");
        }


        if (foe[0].stats[8] <= 0 || foe[1].stats[8] <= 0)
        {
            printf("%s has been defeated.\n",foe[0].name);
            printf("===========================================\n");
            printf("This entire sequence took %.3f seconds.\n",total);
            printf("%s dealt a total of %.0f damage.\n",heroes[0].name,total_damage);
            printf("===========================================\n");
            return;
        }
    }
    printf("===========================================\n");
    printf("This entire sequence took %.3f seconds.\n",total);
    printf("%s dealt a total of %.0f damage.\n",heroes[0].name,total_damage);
    printf("===========================================\n");
}

int main(void)
{
    int i;
    player *team = malloc(sizeof(player) * 2);
    enemy *horde = malloc(sizeof(enemy) * 2);
    srand(time(0));

    horde[0] = createenemy(horde[0],1);
    horde[1] = createenemy(horde[1],1);

    horde[0].size = 2;

    team[0] = herotime(team[0],"Ayaz",10,1);
    printf("Ayaz has %.0f/%.0f HP.\n",team[0].stats[8],team[0].stats[0]);
    team[1] = herotime(team[1],"Cecile",10,2);
    printf("Cecile has %.0f/%.0f HP.\n",team[1].stats[8],team[1].stats[0]);
    
    for (i = 0; i < 2; i++)
        printf("%s attack speed is %.2f%c\n",team[i].name,100*(1-team[i].atkspd),percent);
    
    printf("What's the current position of the hero?\n");
    scanf("%d %d",&(team[0].x),&(team[0].y));

    team[1].x = team[0].x;
    team[1].y = team[0].y;

    battle(&team,&horde);

    printf("Ayaz has %.0f/%.0f HP.\n",team[0].stats[8],team[0].stats[0]);

    for(i = 0; i < 2; i++)
    {
        freehero(team[i]);
        freeenemy(horde[i]);
    }

    free(horde);
    free(team);
    printf("All good.\n");
    return 0;
}