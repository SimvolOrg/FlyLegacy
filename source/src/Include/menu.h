/*
 * menu.h
 *
 * Part of Fly! Legacy project
 *
 * Copyright 2003 Chris Wallace
 *
 * Fly! Legacy is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 * Fly! Legacy is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *   along with Fly! Legacy; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*! \file Menu.h
 *  \brief addin main menu bar in static fashion
 */


#ifndef MENU_H
#define MENU_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// the submenu max number is arbitrary fixed to 10
// must be consistent with the sdkmenu::CSDKMenu::AddMenuItem (...) counter (UI.cpp)
// maybe some dinamically manager can rid of this
// const int MAX_SDK_SUBMENU_NUM = 10; // ui.h

typedef struct SSDKEventNotice {
  
  unsigned long windowID;
  unsigned long componentID;
  unsigned long event;
  unsigned long subEvent;

} SSDKEventNotice;

SSDKEventNotice sdk_en[5][10] = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
                                 {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
                                 {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
                                 {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
                                 {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}};

void _0_0_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[0][0].windowID, sdk_en[0][0].componentID, sdk_en[0][0].event, sdk_en[0][0].subEvent);}
void _0_1_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[0][1].windowID, sdk_en[0][1].componentID, sdk_en[0][1].event, sdk_en[0][1].subEvent);}
void _0_2_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[0][2].windowID, sdk_en[0][2].componentID, sdk_en[0][2].event, sdk_en[0][2].subEvent);}
void _0_3_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[0][3].windowID, sdk_en[0][3].componentID, sdk_en[0][3].event, sdk_en[0][3].subEvent);}
void _0_4_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[0][4].windowID, sdk_en[0][4].componentID, sdk_en[0][4].event, sdk_en[0][4].subEvent);}
void _0_5_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[0][5].windowID, sdk_en[0][5].componentID, sdk_en[0][5].event, sdk_en[0][5].subEvent);}
void _0_6_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[0][6].windowID, sdk_en[0][6].componentID, sdk_en[0][6].event, sdk_en[0][6].subEvent);}
void _0_7_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[0][7].windowID, sdk_en[0][7].componentID, sdk_en[0][7].event, sdk_en[0][7].subEvent);}
void _0_8_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[0][8].windowID, sdk_en[0][8].componentID, sdk_en[0][8].event, sdk_en[0][8].subEvent);}
void _0_9_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[0][9].windowID, sdk_en[0][9].componentID, sdk_en[0][9].event, sdk_en[0][9].subEvent);}

void _1_0_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[1][0].windowID, sdk_en[1][0].componentID, sdk_en[1][0].event, sdk_en[1][0].subEvent);}
void _1_1_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[1][1].windowID, sdk_en[1][1].componentID, sdk_en[1][1].event, sdk_en[1][1].subEvent);}
void _1_2_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[1][2].windowID, sdk_en[1][2].componentID, sdk_en[1][2].event, sdk_en[1][2].subEvent);}
void _1_3_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[1][3].windowID, sdk_en[1][3].componentID, sdk_en[1][3].event, sdk_en[1][3].subEvent);}
void _1_4_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[1][4].windowID, sdk_en[1][4].componentID, sdk_en[1][4].event, sdk_en[1][4].subEvent);}
void _1_5_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[1][5].windowID, sdk_en[1][5].componentID, sdk_en[1][5].event, sdk_en[1][5].subEvent);}
void _1_6_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[1][6].windowID, sdk_en[1][6].componentID, sdk_en[1][6].event, sdk_en[1][6].subEvent);}
void _1_7_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[1][7].windowID, sdk_en[1][7].componentID, sdk_en[1][7].event, sdk_en[1][7].subEvent);}
void _1_8_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[1][8].windowID, sdk_en[1][8].componentID, sdk_en[1][8].event, sdk_en[1][8].subEvent);}
void _1_9_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[1][9].windowID, sdk_en[1][9].componentID, sdk_en[1][9].event, sdk_en[1][9].subEvent);}

void _2_0_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[2][0].windowID, sdk_en[2][0].componentID, sdk_en[2][0].event, sdk_en[2][0].subEvent);}
void _2_1_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[2][1].windowID, sdk_en[2][1].componentID, sdk_en[2][1].event, sdk_en[2][1].subEvent);}
void _2_2_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[2][2].windowID, sdk_en[2][2].componentID, sdk_en[2][2].event, sdk_en[2][2].subEvent);}
void _2_3_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[2][3].windowID, sdk_en[2][3].componentID, sdk_en[2][3].event, sdk_en[2][3].subEvent);}
void _2_4_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[2][4].windowID, sdk_en[2][4].componentID, sdk_en[2][4].event, sdk_en[2][4].subEvent);}
void _2_5_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[2][5].windowID, sdk_en[2][5].componentID, sdk_en[2][5].event, sdk_en[2][5].subEvent);}
void _2_6_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[2][6].windowID, sdk_en[2][6].componentID, sdk_en[2][6].event, sdk_en[2][6].subEvent);}
void _2_7_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[2][7].windowID, sdk_en[2][7].componentID, sdk_en[2][7].event, sdk_en[2][7].subEvent);}
void _2_8_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[2][8].windowID, sdk_en[2][8].componentID, sdk_en[2][8].event, sdk_en[2][8].subEvent);}
void _2_9_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[2][9].windowID, sdk_en[2][9].componentID, sdk_en[2][9].event, sdk_en[2][9].subEvent);}

void _3_0_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[3][0].windowID, sdk_en[3][0].componentID, sdk_en[3][0].event, sdk_en[3][0].subEvent);}
void _3_1_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[3][1].windowID, sdk_en[3][1].componentID, sdk_en[3][1].event, sdk_en[3][1].subEvent);}
void _3_2_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[3][2].windowID, sdk_en[3][2].componentID, sdk_en[3][2].event, sdk_en[3][2].subEvent);}
void _3_3_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[3][3].windowID, sdk_en[3][3].componentID, sdk_en[3][3].event, sdk_en[3][3].subEvent);}
void _3_4_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[3][4].windowID, sdk_en[3][4].componentID, sdk_en[3][4].event, sdk_en[3][4].subEvent);}
void _3_5_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[3][5].windowID, sdk_en[3][5].componentID, sdk_en[3][5].event, sdk_en[3][5].subEvent);}
void _3_6_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[3][6].windowID, sdk_en[3][6].componentID, sdk_en[3][6].event, sdk_en[3][6].subEvent);}
void _3_7_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[3][7].windowID, sdk_en[3][7].componentID, sdk_en[3][7].event, sdk_en[3][7].subEvent);}
void _3_8_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[3][8].windowID, sdk_en[3][8].componentID, sdk_en[3][8].event, sdk_en[3][8].subEvent);}
void _3_9_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[3][9].windowID, sdk_en[3][9].componentID, sdk_en[3][9].event, sdk_en[3][9].subEvent);}

void _4_0_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[4][0].windowID, sdk_en[4][0].componentID, sdk_en[4][0].event, sdk_en[4][0].subEvent);}
void _4_1_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[4][1].windowID, sdk_en[4][1].componentID, sdk_en[4][1].event, sdk_en[4][1].subEvent);}
void _4_2_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[4][2].windowID, sdk_en[4][2].componentID, sdk_en[4][2].event, sdk_en[4][2].subEvent);}
void _4_3_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[4][3].windowID, sdk_en[4][3].componentID, sdk_en[4][3].event, sdk_en[4][3].subEvent);}
void _4_4_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[4][4].windowID, sdk_en[4][4].componentID, sdk_en[4][4].event, sdk_en[4][4].subEvent);}
void _4_5_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[4][5].windowID, sdk_en[4][5].componentID, sdk_en[4][5].event, sdk_en[4][5].subEvent);}
void _4_6_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[4][6].windowID, sdk_en[4][6].componentID, sdk_en[4][6].event, sdk_en[4][6].subEvent);}
void _4_7_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[4][7].windowID, sdk_en[4][7].componentID, sdk_en[4][7].event, sdk_en[4][7].subEvent);}
void _4_8_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[4][8].windowID, sdk_en[4][8].componentID, sdk_en[4][8].event, sdk_en[4][8].subEvent);}
void _4_9_cb (puObject* obj)       {globals->plugins.On_EventNotice (NULL, sdk_en[4][9].windowID, sdk_en[4][9].componentID, sdk_en[4][9].event, sdk_en[4][9].subEvent);}


char *sdk_menu_legends[5][10] = {{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
                                 {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
                                 {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
                                 {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
                                 {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}};

puCallback sdk_menu_cb[5][10] = {{_0_0_cb, _0_1_cb, _0_2_cb, _0_3_cb, _0_4_cb, _0_5_cb, _0_6_cb, _0_7_cb, _0_8_cb, _0_9_cb},
                                 {_1_0_cb, _1_1_cb, _1_2_cb, _1_3_cb, _1_4_cb, _1_5_cb, _1_6_cb, _1_7_cb, _1_8_cb, _1_9_cb},
                                 {_2_0_cb, _2_1_cb, _2_2_cb, _2_3_cb, _2_4_cb, _2_5_cb, _2_6_cb, _2_7_cb, _2_8_cb, _2_9_cb},
                                 {_3_0_cb, _3_1_cb, _3_2_cb, _3_3_cb, _3_4_cb, _3_5_cb, _3_6_cb, _3_7_cb, _3_8_cb, _3_9_cb},
                                 {_4_0_cb, _4_1_cb, _4_2_cb, _4_3_cb, _4_4_cb, _4_5_cb, _4_6_cb, _4_7_cb, _4_8_cb, _4_9_cb}};




#endif // MENU_H