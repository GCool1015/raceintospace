/*
    Copyright (C) 2005 Michael K. McCarty & Fritz Bronner

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/** \file endgame.c End Game Routines
 */

#include "display/graphics.h"
#include "display/surface.h"
#include "display/image.h"

#include "endgame.h"
#include "Buzz_inc.h"
#include "aipur.h"
#include "draw.h"
#include "future.h"
#include "game_main.h"
#include "place.h"
#include "replay.h"
#include "newmis.h"
#include "start.h"
#include "mis_c.h"
#include "sdlhelper.h"
#include "gr.h"
#include "pace.h"
#include "endianness.h"
#include "filesystem.h"

#include <boost/shared_ptr.hpp>

#define NUM_LIGHTS 100
#define FLY_TIME 20
#define GRAVITY 0.6
#define FRICTION 0.3
#define PI 3.1415926
#define MAXINITSPEED 270
#define PutPixel(x,y,col) grPutPixel(x,y,col)

char month, firstOnMoon, capName[30];
char PF[29][40] = {
    "ORBITAL SATELLITE", "LUNAR FLYBY", "MERCURY FLYBY", "VENUS FLYBY",
    "MARS FLYBY", "JUPITER FLYBY", "SATURN FLYBY", "LUNAR PROBE LANDING",
    "DURATION LEVEL F", "DURATION LEVEL E", "DURATION LEVEL D",
    "DURATION LEVEL C", "DURATION LEVEL B", "ONE-PERSON CRAFT",
    "TWO-PERSON CRAFT", "THREE-PERSON CRAFT", "MINISHUTTLE", "FOUR-PERSON CRAFT",
    "MANNED ORBITAL", "MANNED LUNAR PASS", "MANNED LUNAR ORBIT",
    "MANNED RESCUE ATTEMPT", "MANNED LUNAR LANDING", "ORBITING LAB",
    "MANNED DOCKING", "WOMAN IN SPACE", "SPACEWALK", "MANNED SPACE MISSION"
};


char Burst(char win);
void EndGame(char win, char pad);
void Load_LenFlag(char win);
void Draw_NewEnd(char win);
void FakeHistory(char plr, char Fyear);
void HistFile(char *buf, unsigned char bud);
void PrintHist(char *buf);
void PrintOne(char *buf, char tken);
void AltHistory(char plr);
void EndPict(int x, int y, char poff, unsigned char coff);
void LoserPict(char poff, unsigned char coff);

char
Burst(char win)
{
    float Spsn[2];
    char R_value = 0;
    struct PROJECTILE {
        char clr;
        float vel[2];
        float psn[2];
        int16_t per;
    } Bomb[NUM_LIGHTS];
    int lp1, lp2, Region, xx, yy;
    float Ang, Spd, InitSpd;
    char clr = 1;

    key = 0;
    helpText = "i144";
    keyHelpText = "k044";
    vhptr->copyFrom(display::graphics.legacyScreen(), 0, 0, 319, 199);

    while (1) {
        Region = brandom(100);

        if (Region < 60) {
            Spsn[0] = 132 + brandom(187);
            Spsn[1] = 5 + brandom(39);
        } else {
            Spsn[0] = 178 + brandom(66);
            Spsn[1] = 11 + brandom(33);
        }

        InitSpd = brandom(MAXINITSPEED);

        for (lp1 = 0; lp1 < NUM_LIGHTS; lp1++) {
            Ang = brandom(2 * PI);
            Spd = brandom(InitSpd);
            Bomb[lp1].psn[0] = Spsn[0];
            Bomb[lp1].psn[1] = Spsn[1];
            Bomb[lp1].vel[0] = Spd * cos(Ang);
            Bomb[lp1].vel[1] = Spd * sin(Ang);
            Bomb[lp1].clr = clr;
            Bomb[lp1].per = brandom(FLY_TIME);
        }

        for (lp1 = 0; lp1 < FLY_TIME; lp1++) {
            for (lp2 = 0; lp2 < NUM_LIGHTS; lp2++) {
                xx = Bomb[lp2].psn[0];
                yy = Bomb[lp2].psn[1];

                /* This is overkill for pixels, but let's see... */
                if (xx >= 0 && xx < 320 && yy >= 0 && yy <= 172) {
                    display::graphics.legacyScreen()->setPixel(xx, yy, vhptr->getPixel(xx, yy));
                }

                key = 0;

                /* We can't wait 30 ms on default timer */
                GetMouse();

                if (key > 0 || mousebuttons > 0) {
                    if ((x >= 14 && y >= 182 && x <= 65 && y <= 190
                         && mousebuttons > 0) || key == 'H') {
                        R_value = 1;
                    }

                    if ((x >= 74 && y >= 182 && x <= 125 && y <= 190
                         && mousebuttons > 0) || key == 'S') {
                        R_value = 2;
                    }

                    if ((x >= 134 && y >= 182 && x <= 185 && y <= 190
                         && mousebuttons > 0) || key == 'P') {
                        R_value = 3;
                    }

                    if ((x >= 194 && y >= 182 && x <= 245 && y <= 190
                         && mousebuttons > 0) || key == 'M') {
                        R_value = 4;
                    }

                    if ((x >= 254 && y >= 182 && x <= 305 && y <= 190
                         && mousebuttons > 0) || key == K_ENTER) {
                        R_value = 5;
                    }

                    if (R_value > 0) {

                        vhptr->copyTo(display::graphics.legacyScreen(), 0, 0);
                        helpText = "i144";
                        keyHelpText = "k044";

                        return (R_value);
                    }
                }

                Bomb[lp2].vel[1] = Bomb[lp2].vel[1] + GRAVITY;
                Bomb[lp2].vel[0] = Bomb[lp2].vel[0] * FRICTION;
                Bomb[lp2].vel[1] = Bomb[lp2].vel[1] * FRICTION;

                Bomb[lp2].psn[0] =
                    Bomb[lp2].psn[0] + Bomb[lp2].vel[0];
                Bomb[lp2].psn[1] =
                    Bomb[lp2].psn[1] + Bomb[lp2].vel[1];
                xx = Bomb[lp2].psn[0];
                yy = Bomb[lp2].psn[1];

                if (win == 0) {
                    if (clr == 1) {
                        clr = 6;
                    } else if (clr == 6) {
                        clr = 9;
                    } else if (clr == 9) {
                        clr = 1;
                    }
                } else {
                    if (clr == 1) {
                        clr = 9;
                    } else if (clr == 9) {
                        clr = 11;
                    } else if (clr == 11) {
                        clr = 9;
                    }
                }

                if (lp1 < Bomb[lp2].per && (xx >= 0 && xx < 320 && yy >= 0
                                            && yy <= 172)) {
                    display::graphics.legacyScreen()->setPixel(xx, yy, clr);
                }
            }
        }

        for (lp2 = 0; lp2 < NUM_LIGHTS; lp2++) {
            xx = Bomb[lp2].psn[0];
            yy = Bomb[lp2].psn[1];

            if (xx >= 0 && xx < 320 && yy >= 0 && yy <= 172) {
                display::graphics.legacyScreen()->setPixel(xx, yy, vhptr->getPixel(xx, yy));
            }
        }
    }                              // end while

}

void EndGame(char win, char pad)
{
    int i = 0, r, gork;
    char miss, prog, man1, man2, man3, man4, bud;


    FadeOut(2, 10, 0, 0);
    helpText = "i000";
    keyHelpText = "k000";
    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    InBox(3, 3, 30, 19);
    IOBox(242, 3, 315, 19);
    ShBox(0, 24, 319, 199);
    fill_rectangle(5, 28, 314, 195, 0);
    fill_rectangle(5, 105, 239, 110, 3);
    ShBox(101, 102, 218, 113);
    display::graphics.setForegroundColor(6);
    draw_string(112, 110, "ALTERNATE HISTORY");

    if (win == 0) {
        draw_heading(34, 5, "US WINS", 1, -1);
    } else {
        draw_heading(34, 5, "USSR WINS", 1, -1);
    }

    draw_small_flag(win, 4, 4);
    display::graphics.setForegroundColor(1);
    draw_string(256, 13, "CONTINUE");

    if (Option == -1 && MAIL == -1) {
        miss = Data->P[win].Mission[pad].MissionCode;
    } else {
        miss = Data->P[win].History[Data->Prestige[Prestige_MannedLunarLanding].Index].MissionCode;
    }

    display::graphics.setForegroundColor(6);
    draw_string(10, 50, "MISSION TYPE: ");
    display::graphics.setForegroundColor(8);

    if (miss == 55  || miss == 56 || miss == 57) {
        i = 1;
    } else {
        i = 0;
    }

    MissionName(miss, 80, 50, 24);

    if (Option == -1 && MAIL == -1) {
        strcpy(capName , Data->P[win].Mission[pad].Name);
        month   = Data->P[win].Mission[pad].Month;
    } else {
        month   = Data->Prestige[Prestige_MannedLunarLanding].Month;

        if (MAIL != -1 || Option == win) {
            strcpy(capName , Data->P[win].History[Data->Prestige[Prestige_MannedLunarLanding].Index].MissionName[0]);
        } else {
            prog = Data->P[win].History[Data->Prestige[Prestige_MannedLunarLanding].Index].Hard[i][0] + 1;
            strcpy(capName , &Data->P[win].Manned[prog - 1].Name[0]);
            strcat(capName , " ");
            strcat(capName , Nums[Data->P[win].Manned[prog - 1].Used]);
        }
    }

    display::graphics.setForegroundColor(6);
    draw_string(10, 40, "MISSION: ");
    display::graphics.setForegroundColor(8);
    draw_string(0, 0, capName);
    display::graphics.setForegroundColor(6);
    draw_string(0, 0, "  -  ");
    display::graphics.setForegroundColor(8);
    draw_number(0, 0, dayOnMoon);
    draw_string(0, 0, " ");
    draw_string(0, 0, Month[month]);
    draw_string(0, 0, "19");
    draw_number(0, 0, Data->Year);

// correct mission pictures

    if (Option == -1 && MAIL == -1) {
        gork = Data->P[win].PastMissionCount - 1;
    } else {
        gork = Data->Prestige[Prestige_MannedLunarLanding].Index;
    }

    if (win == 1 && Data->P[win].History[gork].Hard[i][0] >= 3) {
        bud = 5;
    } else if (win == 0 && Data->P[win].History[gork].Hard[i][0] == 4) {
        bud = 2;
    } else {
        bud = ((Data->P[win].History[gork].Hard[i][2] - 5) + (win * 3));
    }

    if (bud < 0 || bud > 5) {
        bud = 0 + win;
    }

    InBox(241, 67, 313, 112);
    EndPict(242, 68, bud, 128);
    PatchMe(win, 270, 34, Data->P[win].History[gork].Hard[i][0], Data->P[win].History[gork].Patch[win], 32);
    man1 = Data->P[win].History[gork].Man[i][0];
    man2 = Data->P[win].History[gork].Man[i][1];
    man3 = Data->P[win].History[gork].Man[i][2];
    man4 = Data->P[win].History[gork].Man[i][3];
// no astronaut klugge
    r = Data->P[win].AstroCount;

    if (man1 <= -1) {
        man1 = brandom(r);
    }

    if (man2 <= -1) {
        man2 = brandom(r);
    }

    if (man3 <= -1) {
        man3 = brandom(r);
    }

    if (man4 <= -1) {
        man4 = brandom(r);
    }

    if (!(Option == -1 || Option == win)) {
        Data->P[win].History[gork].Man[i][0] = man1;
        Data->P[win].History[gork].Man[i][1] = man2;
        Data->P[win].History[gork].Man[i][2] = man3;
        Data->P[win].History[gork].Man[i][3] = man4;
    }

    prog = Data->P[win].History[gork].Hard[i][0] + 1;

    for (i = 1; i < 5; i++) {
        display::graphics.setForegroundColor(6);

        switch (i) {
        case 1:
            if (prog == 1) {
                draw_string(10, 70, "CAPSULE PILOT - EVA: ");
            } else if (prog == 2) {
                draw_string(10, 70, "CAPSULE PILOT - DOCKING: ");
            } else if (prog >= 3) {
                draw_string(10, 70, "COMMAND PILOT: ");
            }

            display::graphics.setForegroundColor(8);

            if (man1 != -1) {
                draw_string(0, 0, &Data->P[win].Pool[man1].Name[0]);
            }

            break;

        case 2:
            if (prog > 1 && prog < 5) {
                draw_string(10, 79, "LM PILOT - EVA: ");
            } else if (prog == 5) {
                draw_string(10, 79, "LUNAR PILOT: ");
            }

            display::graphics.setForegroundColor(8);

            if (man2 != -1) {
                draw_string(0, 0, &Data->P[win].Pool[man2].Name[0]);
            }

            break;

        case 3:
            if (prog > 2 && prog < 5) {
                draw_string(10, 88, "DOCKING SPECIALIST: ");
            } else if (prog == 5) {
                draw_string(10, 88, "EVA SPECIALIST: ");
            }

            display::graphics.setForegroundColor(8);

            if (man3 != -1 && prog > 2) {
                draw_string(0, 0, &Data->P[win].Pool[man3].Name[0]);
            }

            break;

        case 4:
            if (prog == 5) {
                draw_string(10, 97, "EVA SPECIALIST: ");
                display::graphics.setForegroundColor(8);

                if (man4 != -1) {
                    draw_string(0, 0, &Data->P[win].Pool[man4].Name[0]);
                }
            }

            break;

        default:
            break;
        }
    }

//Print the first in the moon
    firstOnMoon = (manOnMoon == 1 ? man1 : manOnMoon == 2 ? man2 : manOnMoon == 3 ? man3 : manOnMoon == 4 ? man4 : man2);
    display::graphics.setForegroundColor(11);
    draw_string(10, 60, "FIRST ON THE MOON: ");
    display::graphics.setForegroundColor(14);
    draw_string(0, 0, &Data->P[win].Pool[firstOnMoon].Name[0]);

    display::graphics.setForegroundColor(6);
    AltHistory(win);
    FadeIn(2, 10, 0, 0);

    WaitForMouseUp();
    i = 0;
    key = 0;

    while (i == 0) {
        key = 0;
        GetMouse();

        if ((x >= 244 && y >= 5 && x <= 313 && y <= 17 && mousebuttons > 0) || key == K_ENTER) {
            InBox(244, 5, 313, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            i = 1;
            key = 0;
            OutBox(244, 5, 313, 17);
        };
    }

    return;
}

void Load_LenFlag(char win)
{
    PatchHdr P;
    unsigned int coff;
    int j, Off_X, Off_Y;
    char poff;
    FILE *in;

    if (win == 1) {
        in = sOpen("LENIN.BUT", "rb", 0);
        Off_X = 224;
        Off_Y = 26;
    } else {
        in = sOpen("FLAGGER.BUT", "rb", 0);
        Off_X = 195;
        Off_Y = 0;
    }

    poff = 0;
    coff = 128;
    {
        display::AutoPal p(display::graphics.legacyScreen());
        fread(&p.pal[coff * 3], 384, 1, in);
    }
    fseek(in, (poff) * (sizeof P), SEEK_CUR);
    fread(&P, sizeof P, 1, in);
    SwapPatchHdr(&P);

    if (win != 1) {
        P.w++;    /* BUGFIX as everywhere */
    }

    fseek(in, P.offset, SEEK_SET);
    display::LegacySurface local(P.w, P.h);
    display::LegacySurface local2(P.w, P.h);
    local.clear(0);
    local2.copyFrom(display::graphics.legacyScreen(), Off_X, Off_Y, Off_X + P.w - 1, Off_Y + P.h - 1);
    fread(local.pixels(), P.size, 1, in);
    fclose(in);

    for (j = 0; j < P.size; j++)

        /* now fix the strip */
        if (win == 1 || ((j + 1) % P.w != 0)) {
            local2.pixels()[j] = local.pixels()[j] + coff;
        }

    local2.copyTo(display::graphics.legacyScreen(), Off_X, Off_Y);
}

void Draw_NewEnd(char win)
{
    music_start(M_VICTORY);

    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();

    boost::shared_ptr<display::PalettizedSurface> winner(Filesystem::readImage("images/winner.but.0.png"));
    winner->exportPalette(0, 128);
    display::graphics.screen()->draw(winner, 0, 0);

    ShBox(0, 173, 319, 199);
    InBox(5, 178, 314, 194);
    IOBox(12, 180, 67, 192);
    IOBox(72, 180, 127, 192);
    IOBox(132, 180, 187, 192);
    IOBox(192, 180, 247, 192);
    IOBox(252, 180, 307, 192);
    display::graphics.setForegroundColor(1);
    draw_string(21, 188, "HISTORY");
    draw_string(85, 188, "STATS");
    draw_string(142, 188, "PARADE");
    draw_string(198, 188, "MOON EVA");
    draw_string(269, 188, "EXIT");
    FadeIn(0, 10, 128, 0);
    Load_LenFlag(win);
    FadeIn(1, 40, 128, 1);
}

void NewEnd(char win, char loc)
{
    int i, Re_Draw = 0;
    char R_V = 0;

    music_start(M_VICTORY);
    EndGame(win, loc);
    Draw_NewEnd(win);
    R_V = Burst(win);
    WaitForMouseUp();
    i = 0;
    key = 0;
    display::LegacySurface local(162, 92);
    local.clear(0);

    while (i == 0) {
        key = 0;
        GetMouse();
        helpText = "i144";
        keyHelpText = "k044";

        music_start(M_VICTORY);

        if (((key == 'P' || key == 'M' || key == 'H' || key == 'S') || mousebuttons > 0) || R_V == 0)
            if (Re_Draw == 1) {
                if ((x >= 14 && y >= 182 && x <= 65 && y <= 190 && mousebuttons > 0) || key == 'H') {
                    R_V = 1;
                }

                if ((x >= 74 && y >= 182 && x <= 125 && y <= 190 && mousebuttons > 0) || key == 'S') {
                    R_V = 2;
                }

                if ((x >= 134 && y >= 182 && x <= 185 && y <= 190 && mousebuttons > 0) || key == 'P') {
                    R_V = 3;
                }

                if ((x >= 194 && y >= 182 && x <= 245 && y <= 190 && mousebuttons > 0) || key == 'M') {
                    R_V = 4;
                }

                if ((x >= 254 && y >= 182 && x <= 305 && y <= 190 && mousebuttons > 0) || key == K_ENTER) {
                    R_V = 5;
                }

                local.copyTo(display::graphics.legacyScreen(), 149, 9);
                {
                    display::AutoPal p(display::graphics.legacyScreen());
                    memset(&p.pal[384], 0, 384);
                }
                local.clear(0);
                Load_LenFlag(win);
                FadeIn(1, 40, 128, 1);

                if (R_V == 0 || R_V == -1) {
                    R_V = Burst(win);
                }

                Re_Draw = 0;
                helpText = "i144";
                keyHelpText = "k044";
            }

        if (((x >= 14 && y >= 182 && x <= 65 && y <= 190 && mousebuttons > 0) || key == 'H') || R_V == 1) {
            // History box
            InBox(14, 182, 65, 190);
            WaitForMouseUp();

            if (key > 0 || R_V > 0) {
                delay(150);
            }

            i = 0;
            key = 0;
            OutBox(14, 182, 65, 190);
            EndGame(win, loc);
            Draw_NewEnd(win);
            helpText = "i144";
            keyHelpText = "k044";
            R_V = Burst(win);
        }

        if (((x >= 74 && y >= 182 && x <= 125 && y <= 190 && mousebuttons > 0) || key == 'S') || R_V == 2) {
            // Stats box
            music_stop();
            InBox(74, 182, 125, 190);
            WaitForMouseUp();

            if (key > 0 || R_V > 0) {
                delay(150);
            }

            i = 0;
            key = 0;
            OutBox(74, 182, 125, 190);
            music_start(M_THEME);
            Stat(win);
            Draw_NewEnd(win);
            helpText = "i144";
            keyHelpText = "k044";
            R_V = Burst(win);
        }

        if (((x >= 134 && y >= 182 && x <= 185 && y <= 190 && mousebuttons > 0) || key == 'P') || R_V == 3) {
            // Parade
            music_stop();
            InBox(134, 182, 185, 190);
            WaitForMouseUp();

            if (key > 0 || R_V > 0) {
                delay(150);
            }

            if (R_V == 3) {
                R_V = -1;
            }

            i = 0;
            key = 0;
            Re_Draw = 1;
            OutBox(134, 182, 185, 190);
            FadeOut(1, 40, 128, 1);
            fill_rectangle(195, 0, 319, 172, 0);
            local.copyFrom(display::graphics.legacyScreen(), 149, 9, 309, 100);
            ShBox(149, 9, 309, 100);
            InBox(153, 13, 305, 96);
            music_start(M_PRGMTRG);
            Replay(win, 0, 154, 14, 149, 82, (win == 0) ? "UPAR" : "SPAR");
            music_stop();
            helpText = "i144";
            keyHelpText = "k044";
        }

        if (((x >= 194 && y >= 182 && x <= 245 && y <= 190 && mousebuttons > 0) || key == 'M') || R_V == 4) {
            // Moon EVA
            music_stop();
            InBox(194, 182, 245, 190);
            WaitForMouseUp();

            if (key > 0 || R_V > 0) {
                delay(150);
            }

            if (R_V == 4) {
                R_V = -1;
            }

            i = 0;
            key = 0;
            OutBox(194, 182, 245, 190);
            Re_Draw = 1;
            FadeOut(1, 40, 128, 1);
            fill_rectangle(195, 0, 319, 172, 0);
            local.copyFrom(display::graphics.legacyScreen(), 149, 9, 309, 100);
            ShBox(149, 9, 309, 100);
            InBox(153, 13, 305, 96);
            music_start(M_MISSPLAN);
            Replay(win, 0, 154, 14, 149, 82, (win == 0) ? "PUM3C6" : "PSM3C6");
            music_stop();
            helpText = "i144";
            keyHelpText = "k044";
        }

        if (((x >= 254 && y >= 182 && x <= 305 && y <= 190 && mousebuttons > 0) || key == K_ENTER) || R_V == 5) {
            music_stop();
            InBox(254, 182, 305, 190);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            i = 1;
            key = 0;
            OutBox(254, 182, 305, 190);
        };
    }
}

void FakeWin(char win)
{
    int i, r;
    char miss, prog, man1, man2, man3, man4, bud, yr, monthWin;
    monthWin = brandom(12);

    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 22);
    InBox(3, 3, 30, 19);
    IOBox(242, 3, 315, 19);
    ShBox(0, 24, 319, 199);
    fill_rectangle(5, 28, 314, 195, 0);
    fill_rectangle(5, 105, 239, 110, 3);
    ShBox(101, 102, 218, 113);
    display::graphics.setForegroundColor(6);
    draw_string(112, 110, "ALTERNATE HISTORY");

    if (win == 0) {
        draw_heading(36, 5, "US WINS", 1, -1);
    } else {
        draw_heading(36, 5, "USSR WINS", 1, -1);
    }

    draw_small_flag(win, 4, 4);
    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");
    r = brandom(100);

    if (r < 45) {
        miss = 53;
    } else if (r < 50) {
        miss = 54;
    } else if (r < 85) {
        miss = 55;
    } else {
        miss = 56;
    }

    display::graphics.setForegroundColor(6);
    draw_string(10, 50, "MISSION TYPE: ");
    display::graphics.setForegroundColor(8);

    MissionName(miss, 80, 50, 24);
    display::graphics.setForegroundColor(6);

    if (Data->Year <= 65) {
        r = 65 + brandom(5);
    } else if (Data->Year <= 70) {
        r = 70 + brandom(3);
    } else if (Data->Year <= 77) {
        r = Data->Year;
    }

    yr = r;
    r = brandom(100);

    if (miss == 54) {
        prog = 5;
    } else if (r < 20) {
        prog = 2;
    } else if (r < 60) {
        prog = 3;
    } else {
        prog = 4;
    }

    display::graphics.setForegroundColor(6);
    draw_string(10, 40, "MISSION: ");
    display::graphics.setForegroundColor(8);
    draw_string(0, 0, &Data->P[win].Manned[prog - 1].Name[0]);
    draw_string(0, 0, " ");
    draw_string(0, 0, &Nums[brandom(15) + 1][0]);
    display::graphics.setForegroundColor(6);
    draw_string(0, 0, "  -  ");
    display::graphics.setForegroundColor(8);;
    draw_number(0, 0, brandom(daysAMonth[monthWin]) + 1);
    draw_string(0, 0, " ");
    draw_string(0, 0, Month[monthWin]);
    draw_string(0, 0, "19");
    draw_number(0, 0, yr);
    r = brandom(100);

    if (win == 1 && prog == 5) {
        bud = 5;
    } else if (win == 0 && prog == 5) {
        bud = 2;
    } else {
        bud = (r < 50) ? 0 + (win * 3) : 1 + (win * 3);
    }

    if (bud < 0 || bud > 5) {
        bud = 0 + win;
    }

    InBox(241, 67, 313, 112);
    EndPict(242, 68, bud, 128);
    PatchMe(win, 270, 34, prog - 1, brandom(9), 32);
    r = Data->P[win].AstroCount;
    man1 = brandom(r);
    man2 = brandom(r);
    man3 = brandom(r);
    man4 = brandom(r);

    while (1) {
        if ((man1 != man2) && (man1 != man3) && (man2 != man4) &&
            (man2 != man3) && (man3 != man4) && (man1 != man4)) {
            break;
        }

        while (man1 == man2) {
            man2 = brandom(r);
        }

        while (man1 == man3) {
            man3 = brandom(r);
        }

        while (man2 == man4) {
            man2 = brandom(r);
        }

        while (man2 == man3) {
            man3 = brandom(r);
        }

        while (man3 == man4) {
            man4 = brandom(r);
        }

        while (man1 == man4) {
            man4 = brandom(r);
        }
    }

    for (i = 1; i < 5; i++) {
        display::graphics.setForegroundColor(6);

        switch (i) {
        case 1:
            if (prog >= 1 && prog <= 3) {
                draw_string(10, 70, "CAPSULE PILOT - EVA: ");
            }

            if (prog > 3) {
                draw_string(10, 70, "COMMAND PILOT: ");
            }

            display::graphics.setForegroundColor(8);

            if (man1 != -1) {
                draw_string(0, 0, &Data->P[win].Pool[man1].Name[0]);
            }

            break;

        case 2:
            if (prog > 1 && prog < 5) {
                draw_string(10, 79, "LM PILOT - EVA: ");
            } else if (prog == 5) {
                draw_string(10, 79, "LUNAR PILOT: ");
            }

            display::graphics.setForegroundColor(8);

            if (man2 != -1 && (prog > 1 && prog < 5)) {
                draw_string(0, 0, &Data->P[win].Pool[man2].Name[0]);
            }

            break;

        case 3:
            if (prog > 2 && prog < 5) {
                draw_string(10, 88, "DOCKING SPECIALIST: ");
            } else if (prog == 5) {
                draw_string(10, 88, "EVA SPECIALIST: ");
            }

            display::graphics.setForegroundColor(8);

            if (man3 != -1 && prog > 2) {
                draw_string(0, 0, &Data->P[win].Pool[man3].Name[0]);
            }

            break;

        case 4:
            if (prog == 5) {
                draw_string(10, 97, "EVA SPECIALIST: ");
                display::graphics.setForegroundColor(8);

                if (man4 != -1 && prog == 5) {
                    draw_string(0, 0, &Data->P[win].Pool[man4].Name[0]);
                }
            }

            break;

        default:
            break;
        }
    }

    manOnMoon = man2;

    if (prog == 3 || prog == 4) {
        manOnMoon = man1;
    }

    display::graphics.setForegroundColor(11);
    draw_string(10, 60, "FIRST ON THE MOON: ");
    display::graphics.setForegroundColor(14);
    draw_string(0, 0, &Data->P[win].Pool[manOnMoon].Name[0]);
    display::graphics.setForegroundColor(6);
    FakeHistory(win, yr);
    music_start(M_INTERLUD);
    FadeIn(2, 10, 0, 0);

    WaitForMouseUp();
    i = 0;
    key = 0;

    while (i == 0) {
        key = 0;
        GetMouse();

        if ((x >= 244 && y >= 5 && x <= 313 && y <= 17 && mousebuttons > 0) || key == K_ENTER) {
            InBox(244, 5, 313, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            i = 1;
            key = 0;
            OutBox(244, 5, 313, 17);
        };
    }

    music_stop();
    return;
}

void FakeHistory(char plr, char Fyear) // holds the winning player
{
    char bud;
    memset(buffer, 0, BUFFER_SIZE);

    if (Fyear <= 65) {
        bud = 0 + plr;
    } else if (Fyear <= 67) {
        bud = 2 + plr;
    } else if (Fyear <= 69) {
        bud = 4 + plr;
    } else if (Fyear <= 71) {
        bud = 6 + plr;
    } else if (Fyear >= 72) {
        bud = 8 + plr;
    } else {
        bud = 10 + plr;
    }

    HistFile(buffer + 1000, bud);
    PrintHist(buffer + 1000);
    return;
}

void HistFile(char *buf, unsigned char bud)
{
    FILE *fin;
    long i;
    i = bud * 600;
    fin = sOpen("ENDGAME.DAT", "rb", 0);
    fseek(fin, i, SEEK_SET);
    fread(buf, 600, 1, fin);
    fclose(fin);
}

void PrintHist(char *buf)
{
    int i, k;
    display::graphics.setForegroundColor(8);
    k = 121;
    grMoveTo(10, k);

    for (i = 0; i < (int)strlen(buf); i++) {
        if (buf[i] == '*') {
            k += 7;
            grMoveTo(10, k);
        } else {
            draw_character(buf[i]);
        }
    }
}

void PrintOne(char *buf, char tken)
{
    int i, k;
    display::graphics.setForegroundColor(7);

    if (tken == 0)  {
        k = 127;
    } else {
        k = 170;
    }

    grMoveTo(10, k);

    for (i = 0; i < (int)strlen(buf); i++) {
        if (buf[i] == '*') {
            k += 7;
            grMoveTo(10, k);
        } else {
            draw_character(buf[i]);
        }
    }
}

void AltHistory(char plr)  // holds the winning player
{
    char bud;
    memset(buffer, 0, BUFFER_SIZE);

    if (Data->Year <= 65) {
        bud = 0 + plr;
    } else if (Data->Year <= 67) {
        bud = 2 + plr;
    } else if (Data->Year <= 69) {
        bud = 4 + plr;
    } else if (Data->Year <= 71) {
        bud = 6 + plr;
    } else if (Data->Year >= 72) {
        bud = 8 + plr;
    } else {
        bud = 10 + plr;
    }

    HistFile(buffer + 1000, bud);
    PrintHist(buffer + 1000);
    return;
}

void SpecialEnd(void)
{
    char i;
    music_start(M_BADNEWS);

    display::graphics.screen()->clear();
    ShBox(0, 0, 319, 24);
    draw_heading(5, 5, "FAILED OBJECTIVE", 1, -1);
    ShBox(0, 26, 319, 199);
    fill_rectangle(1, 27, 318, 198, 7);
    InBox(5, 31, 314, 194);
    fill_rectangle(6, 32, 313, 193, 3);
    IOBox(242, 3, 315, 19);
    display::graphics.setForegroundColor(1);
    draw_string(258, 13, "CONTINUE");
    ShBox(6, 109, 313, 119);
    ShBox(6, 151, 313, 161);
    display::graphics.setForegroundColor(9);
    draw_string(130, 116, "UNITED STATES");
    draw_string(134, 158, "SOVIET UNION");
    fill_rectangle(6, 32, 313, 108, 0);
    InBox(178, 3, 205, 19);
    draw_small_flag(0, 179, 4);
    InBox(210, 3, 237, 19);
    draw_small_flag(1, 211, 4);
    LoserPict(0, 128); // load loser picture
    memset(buffer, 0x00, BUFFER_SIZE);
    HistFile(buffer + 1000, 10);
    PrintOne(buffer + 1000, 0);
    memset(buffer, 0x00, BUFFER_SIZE);
    HistFile(buffer + 1000, 11);
    PrintOne(buffer + 1000, 1);
    FadeIn(2, 10, 0, 0);

    WaitForMouseUp();
    i = 0;
    key = 0;

    while (i == 0) {
        key = 0;
        GetMouse();

        if ((x >= 244 && y >= 5 && x <= 313 && y <= 17 && mousebuttons > 0) || key == K_ENTER) {
            InBox(244, 5, 313, 17);
            WaitForMouseUp();

            if (key > 0) {
                delay(150);
            }

            i = 1;
            key = 0;
        };
    }

    music_stop();
    return;
}

void
EndPict(int x, int y, char poff, unsigned char coff)
{
    PatchHdrSmall P;
    unsigned int j;
    FILE *in;

    in = sOpen("ENDGAME.BUT", "rb", 0);
    {
        display::AutoPal p(display::graphics.legacyScreen());
        fread(&p.pal[coff * 3], 384, 1, in);
    }
    fseek(in, (poff) * (sizeof P), SEEK_CUR);
    fread(&P, sizeof P, 1, in);
    SwapPatchHdrSmall(&P);
    /*
     * off by one error in data file - again
     * P.w += 1 solves the problem, but then
     * we get a strip of garbage on the right hand side
     */
    P.w++;
    fseek(in, P.offset, SEEK_SET);
    display::LegacySurface local(P.w, P.h);
    display::LegacySurface local2(P.w, P.h);
    local2.copyFrom(display::graphics.legacyScreen(), x, y, x + P.w - 1, y + P.h - 1);
    fread(local.pixels(), P.size, 1, in);
    fclose(in);

    for (j = 0; j < P.size; j++)

        /* fix the strip */
        if (local.pixels()[j] != 0 && ((j + 1) % P.w != 0)) {
            local2.pixels()[j] = local.pixels()[j] + coff;
        }


    local2.copyTo(display::graphics.legacyScreen(), x, y);
}

void
LoserPict(char poff, unsigned char coff)
{
    /* This hasn't got an off-by-one...*/
    PatchHdr P;
    FILE *in;

    in = sOpen("LOSER.BUT", "rb", 0);
    {
        display::AutoPal p(display::graphics.legacyScreen());
        fread(&p.pal[coff * 3], 384, 1, in);
    }
    fseek(in, (poff) * (sizeof P), SEEK_CUR);
    fread(&P, sizeof P, 1, in);
    SwapPatchHdr(&P);
    fseek(in, P.offset, SEEK_SET);
    display::LegacySurface local(P.w, P.h);
    display::LegacySurface local2(P.w, P.h);
    local2.copyFrom(display::graphics.legacyScreen(), 6, 32, 6 + P.w - 1, 32 + P.h - 1);
    fread(local.pixels(), P.size, 1, in);
    fclose(in);

    local2.maskCopy(&local, 0, display::LegacySurface::SourceNotEqual, coff);

    local2.copyTo(display::graphics.legacyScreen(), 6, 32);
}


void PlayFirst(char plr, char first)
{
    char i, w = 0, index;
    int Check = 0;

    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
    music_start(M_LIFTOFF);
    ShBox(80, 18, 240, 39);
    draw_heading(92, 22, "PRESTIGE FIRST", 0, -1);
    ShBox(80, 41, 240, 132);
    InBox(84, 45, 236, 128);
    fill_rectangle(85, 46, 235, 127, 0);
    ShBox(80, 134, 240, 189); //77 first parameter
    display::graphics.setForegroundColor(1);
    draw_string(84, 141, "GOAL STEP COMPLETE: ");
    display::graphics.setForegroundColor(6);

//Modem Opponent => assure prestige first that mission
    Check = Data->Prestige[first].Index;
    index = plr;

    if (index == 0) {
        draw_string(0, 0, "U.S.A.");
    } else {
        draw_string(0, 0, "SOVIET");
    }

    for (i = first; i < 28; i++) {
        display::graphics.setForegroundColor(9);

        if (Data->Prestige[i].Place == index && Data->PD[index][i] == 0) {
            if (Option == -1 && MAIL == -1) {
                draw_string(84, 148 + w * 8, &PF[i][0]);
                ++w;
                Data->PD[index][i] = 1;
            } else {
                //Found prestige first same mission
                if (Data->Prestige[i].Index == Check) {
                    draw_string(84, 148 + w * 8, &PF[i][0]);
                    ++w;
                    Data->PD[index][i] = 1;
                }
            }
        }
    }

    display::graphics.setForegroundColor(7);
    FadeIn(2, 10, 0, 0);

    if (Option == -1 && MAIL == -1) {
        Replay(plr, Data->P[plr].PastMissionCount - 1, 85, 46, 151, 82, "OOOO");
    } else {
        Replay(index, Data->Prestige[first].Index, 85, 46, 151, 82, "OOOO");
    }

    PauseMouse();
    FadeOut(2, 10, 0, 0);
    display::graphics.screen()->clear();
    music_stop();
    return;
}

/* vim: set noet ts=4 sw=4 tw=77: */
