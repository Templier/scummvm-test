/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* 
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "sword25/fmv/yuvtorgba.h"

#ifdef ENABLE_THEORADEC

namespace Sword25 {

static const int PRECISION = 32768;
static const int COEFFS_Y[256] = {
	-593888, -555746, -517604, -479462, -441320, -403178, -365036, -326894, -288752, -250610, -212468, -174326, -136184,  -98042,  -59900,  -21758, 
	  16384,   54526,   92668,  130810,  168952,  207094,  245236,  283378,  321520,  359662,  397804,  435946,  474088,  512230,  550372,  588514, 
	 626656,  664798,  702940,  741082,  779224,  817366,  855508,  893650,  931792,  969934, 1008076, 1046218, 1084360, 1122502, 1160644, 1198786, 
	1236928, 1275070, 1313212, 1351354, 1389496, 1427638, 1465780, 1503922, 1542064, 1580206, 1618348, 1656490, 1694632, 1732774, 1770916, 1809058, 
	1847200, 1885342, 1923484, 1961626, 1999768, 2037910, 2076052, 2114194, 2152336, 2190478, 2228620, 2266762, 2304904, 2343046, 2381188, 2419330, 
	2457472, 2495614, 2533756, 2571898, 2610040, 2648182, 2686324, 2724466, 2762608, 2800750, 2838892, 2877034, 2915176, 2953318, 2991460, 3029602, 
	3067744, 3105886, 3144028, 3182170, 3220312, 3258454, 3296596, 3334738, 3372880, 3411022, 3449164, 3487306, 3525448, 3563590, 3601732, 3639874, 
	3678016, 3716158, 3754300, 3792442, 3830584, 3868726, 3906868, 3945010, 3983152, 4021294, 4059436, 4097578, 4135720, 4173862, 4212004, 4250146, 
	4288288, 4326430, 4364572, 4402714, 4440856, 4478998, 4517140, 4555282, 4593424, 4631566, 4669708, 4707850, 4745992, 4784134, 4822276, 4860418, 
	4898560, 4936702, 4974844, 5012986, 5051128, 5089270, 5127412, 5165554, 5203696, 5241838, 5279980, 5318122, 5356264, 5394406, 5432548, 5470690, 
	5508832, 5546974, 5585116, 5623258, 5661400, 5699542, 5737684, 5775826, 5813968, 5852110, 5890252, 5928394, 5966536, 6004678, 6042820, 6080962, 
	6119104, 6157246, 6195388, 6233530, 6271672, 6309814, 6347956, 6386098, 6424240, 6462382, 6500524, 6538666, 6576808, 6614950, 6653092, 6691234, 
	6729376, 6767518, 6805660, 6843802, 6881944, 6920086, 6958228, 6996370, 7034512, 7072654, 7110796, 7148938, 7187080, 7225222, 7263364, 7301506, 
	7339648, 7377790, 7415932, 7454074, 7492216, 7530358, 7568500, 7606642, 7644784, 7682926, 7721068, 7759210, 7797352, 7835494, 7873636, 7911778, 
	7949920, 7988062, 8026204, 8064346, 8102488, 8140630, 8178772, 8216914, 8255056, 8293198, 8331340, 8369482, 8407624, 8445766, 8483908, 8522050, 
	8560192, 8598334, 8636476, 8674618, 8712760, 8750902, 8789044, 8827186, 8865328, 8903470, 8941612, 8979754, 9017896, 9056038, 9094180, 9132322, 
};
static const int COEFFS_RV[256] = {
	-6694144, -6641846, -6589548, -6537250, -6484952, -6432654, -6380356, -6328058, -6275760, -6223462, -6171164, -6118866, -6066568, -6014270, -5961972, -5909674, 
	-5857376, -5805078, -5752780, -5700482, -5648184, -5595886, -5543588, -5491290, -5438992, -5386694, -5334396, -5282098, -5229800, -5177502, -5125204, -5072906, 
	-5020608, -4968310, -4916012, -4863714, -4811416, -4759118, -4706820, -4654522, -4602224, -4549926, -4497628, -4445330, -4393032, -4340734, -4288436, -4236138, 
	-4183840, -4131542, -4079244, -4026946, -3974648, -3922350, -3870052, -3817754, -3765456, -3713158, -3660860, -3608562, -3556264, -3503966, -3451668, -3399370, 
	-3347072, -3294774, -3242476, -3190178, -3137880, -3085582, -3033284, -2980986, -2928688, -2876390, -2824092, -2771794, -2719496, -2667198, -2614900, -2562602, 
	-2510304, -2458006, -2405708, -2353410, -2301112, -2248814, -2196516, -2144218, -2091920, -2039622, -1987324, -1935026, -1882728, -1830430, -1778132, -1725834, 
	-1673536, -1621238, -1568940, -1516642, -1464344, -1412046, -1359748, -1307450, -1255152, -1202854, -1150556, -1098258, -1045960,  -993662,  -941364,  -889066, 
	 -836768,  -784470,  -732172,  -679874,  -627576,  -575278,  -522980,  -470682,  -418384,  -366086,  -313788,  -261490,  -209192,  -156894,  -104596,   -52298, 
		   0,    52298,   104596,   156894,   209192,   261490,   313788,   366086,   418384,   470682,   522980,   575278,   627576,   679874,   732172,   784470, 
	  836768,   889066,   941364,   993662,  1045960,  1098258,  1150556,  1202854,  1255152,  1307450,  1359748,  1412046,  1464344,  1516642,  1568940,  1621238, 
	 1673536,  1725834,  1778132,  1830430,  1882728,  1935026,  1987324,  2039622,  2091920,  2144218,  2196516,  2248814,  2301112,  2353410,  2405708,  2458006, 
	 2510304,  2562602,  2614900,  2667198,  2719496,  2771794,  2824092,  2876390,  2928688,  2980986,  3033284,  3085582,  3137880,  3190178,  3242476,  3294774, 
	 3347072,  3399370,  3451668,  3503966,  3556264,  3608562,  3660860,  3713158,  3765456,  3817754,  3870052,  3922350,  3974648,  4026946,  4079244,  4131542, 
	 4183840,  4236138,  4288436,  4340734,  4393032,  4445330,  4497628,  4549926,  4602224,  4654522,  4706820,  4759118,  4811416,  4863714,  4916012,  4968310, 
	 5020608,  5072906,  5125204,  5177502,  5229800,  5282098,  5334396,  5386694,  5438992,  5491290,  5543588,  5595886,  5648184,  5700482,  5752780,  5805078, 
	 5857376,  5909674,  5961972,  6014270,  6066568,  6118866,  6171164,  6223462,  6275760,  6328058,  6380356,  6432654,  6484952,  6537250,  6589548,  6641846, 
};
static const int COEFFS_GU[256] = {
	 1639936,  1627124,  1614312,  1601500,  1588688,  1575876,  1563064,  1550252,  1537440,  1524628,  1511816,  1499004,  1486192,  1473380,  1460568,  1447756, 
	 1434944,  1422132,  1409320,  1396508,  1383696,  1370884,  1358072,  1345260,  1332448,  1319636,  1306824,  1294012,  1281200,  1268388,  1255576,  1242764, 
	 1229952,  1217140,  1204328,  1191516,  1178704,  1165892,  1153080,  1140268,  1127456,  1114644,  1101832,  1089020,  1076208,  1063396,  1050584,  1037772, 
	 1024960,  1012148,   999336,   986524,   973712,   960900,   948088,   935276,   922464,   909652,   896840,   884028,   871216,   858404,   845592,   832780, 
	  819968,   807156,   794344,   781532,   768720,   755908,   743096,   730284,   717472,   704660,   691848,   679036,   666224,   653412,   640600,   627788, 
	  614976,   602164,   589352,   576540,   563728,   550916,   538104,   525292,   512480,   499668,   486856,   474044,   461232,   448420,   435608,   422796, 
	  409984,   397172,   384360,   371548,   358736,   345924,   333112,   320300,   307488,   294676,   281864,   269052,   256240,   243428,   230616,   217804, 
	  204992,   192180,   179368,   166556,   153744,   140932,   128120,   115308,   102496,    89684,    76872,    64060,    51248,    38436,    25624,    12812, 
		   0,   -12812,   -25624,   -38436,   -51248,   -64060,   -76872,   -89684,  -102496,  -115308,  -128120,  -140932,  -153744,  -166556,  -179368,  -192180, 
	 -204992,  -217804,  -230616,  -243428,  -256240,  -269052,  -281864,  -294676,  -307488,  -320300,  -333112,  -345924,  -358736,  -371548,  -384360,  -397172, 
	 -409984,  -422796,  -435608,  -448420,  -461232,  -474044,  -486856,  -499668,  -512480,  -525292,  -538104,  -550916,  -563728,  -576540,  -589352,  -602164, 
	 -614976,  -627788,  -640600,  -653412,  -666224,  -679036,  -691848,  -704660,  -717472,  -730284,  -743096,  -755908,  -768720,  -781532,  -794344,  -807156, 
	 -819968,  -832780,  -845592,  -858404,  -871216,  -884028,  -896840,  -909652,  -922464,  -935276,  -948088,  -960900,  -973712,  -986524,  -999336, -1012148, 
	-1024960, -1037772, -1050584, -1063396, -1076208, -1089020, -1101832, -1114644, -1127456, -1140268, -1153080, -1165892, -1178704, -1191516, -1204328, -1217140, 
	-1229952, -1242764, -1255576, -1268388, -1281200, -1294012, -1306824, -1319636, -1332448, -1345260, -1358072, -1370884, -1383696, -1396508, -1409320, -1422132, 
	-1434944, -1447756, -1460568, -1473380, -1486192, -1499004, -1511816, -1524628, -1537440, -1550252, -1563064, -1575876, -1588688, -1601500, -1614312, -1627124, 
};
static const int COEFFS_GV[256] = {
	 3409920,  3383280,  3356640,  3330000,  3303360,  3276720,  3250080,  3223440,  3196800,  3170160,  3143520,  3116880,  3090240,  3063600,  3036960,  3010320, 
	 2983680,  2957040,  2930400,  2903760,  2877120,  2850480,  2823840,  2797200,  2770560,  2743920,  2717280,  2690640,  2664000,  2637360,  2610720,  2584080, 
	 2557440,  2530800,  2504160,  2477520,  2450880,  2424240,  2397600,  2370960,  2344320,  2317680,  2291040,  2264400,  2237760,  2211120,  2184480,  2157840, 
	 2131200,  2104560,  2077920,  2051280,  2024640,  1998000,  1971360,  1944720,  1918080,  1891440,  1864800,  1838160,  1811520,  1784880,  1758240,  1731600, 
	 1704960,  1678320,  1651680,  1625040,  1598400,  1571760,  1545120,  1518480,  1491840,  1465200,  1438560,  1411920,  1385280,  1358640,  1332000,  1305360, 
	 1278720,  1252080,  1225440,  1198800,  1172160,  1145520,  1118880,  1092240,  1065600,  1038960,  1012320,   985680,   959040,   932400,   905760,   879120, 
	  852480,   825840,   799200,   772560,   745920,   719280,   692640,   666000,   639360,   612720,   586080,   559440,   532800,   506160,   479520,   452880, 
	  426240,   399600,   372960,   346320,   319680,   293040,   266400,   239760,   213120,   186480,   159840,   133200,   106560,    79920,    53280,    26640, 
		   0,   -26640,   -53280,   -79920,  -106560,  -133200,  -159840,  -186480,  -213120,  -239760,  -266400,  -293040,  -319680,  -346320,  -372960,  -399600, 
	 -426240,  -452880,  -479520,  -506160,  -532800,  -559440,  -586080,  -612720,  -639360,  -666000,  -692640,  -719280,  -745920,  -772560,  -799200,  -825840, 
	 -852480,  -879120,  -905760,  -932400,  -959040,  -985680, -1012320, -1038960, -1065600, -1092240, -1118880, -1145520, -1172160, -1198800, -1225440, -1252080, 
	-1278720, -1305360, -1332000, -1358640, -1385280, -1411920, -1438560, -1465200, -1491840, -1518480, -1545120, -1571760, -1598400, -1625040, -1651680, -1678320, 
	-1704960, -1731600, -1758240, -1784880, -1811520, -1838160, -1864800, -1891440, -1918080, -1944720, -1971360, -1998000, -2024640, -2051280, -2077920, -2104560, 
	-2131200, -2157840, -2184480, -2211120, -2237760, -2264400, -2291040, -2317680, -2344320, -2370960, -2397600, -2424240, -2450880, -2477520, -2504160, -2530800, 
	-2557440, -2584080, -2610720, -2637360, -2664000, -2690640, -2717280, -2743920, -2770560, -2797200, -2823840, -2850480, -2877120, -2903760, -2930400, -2957040, 
	-2983680, -3010320, -3036960, -3063600, -3090240, -3116880, -3143520, -3170160, -3196800, -3223440, -3250080, -3276720, -3303360, -3330000, -3356640, -3383280, 
};
static const int COEFFS_BU[256] = {
	-8464128, -8398002, -8331876, -8265750, -8199624, -8133498, -8067372, -8001246, -7935120, -7868994, -7802868, -7736742, -7670616, -7604490, -7538364, -7472238, 
	-7406112, -7339986, -7273860, -7207734, -7141608, -7075482, -7009356, -6943230, -6877104, -6810978, -6744852, -6678726, -6612600, -6546474, -6480348, -6414222, 
	-6348096, -6281970, -6215844, -6149718, -6083592, -6017466, -5951340, -5885214, -5819088, -5752962, -5686836, -5620710, -5554584, -5488458, -5422332, -5356206, 
	-5290080, -5223954, -5157828, -5091702, -5025576, -4959450, -4893324, -4827198, -4761072, -4694946, -4628820, -4562694, -4496568, -4430442, -4364316, -4298190, 
	-4232064, -4165938, -4099812, -4033686, -3967560, -3901434, -3835308, -3769182, -3703056, -3636930, -3570804, -3504678, -3438552, -3372426, -3306300, -3240174, 
	-3174048, -3107922, -3041796, -2975670, -2909544, -2843418, -2777292, -2711166, -2645040, -2578914, -2512788, -2446662, -2380536, -2314410, -2248284, -2182158, 
	-2116032, -2049906, -1983780, -1917654, -1851528, -1785402, -1719276, -1653150, -1587024, -1520898, -1454772, -1388646, -1322520, -1256394, -1190268, -1124142, 
	-1058016,  -991890,  -925764,  -859638,  -793512,  -727386,  -661260,  -595134,  -529008,  -462882,  -396756,  -330630,  -264504,  -198378,  -132252,   -66126, 
		   0,    66126,   132252,   198378,   264504,   330630,   396756,   462882,   529008,   595134,   661260,   727386,   793512,   859638,   925764,   991890, 
	 1058016,  1124142,  1190268,  1256394,  1322520,  1388646,  1454772,  1520898,  1587024,  1653150,  1719276,  1785402,  1851528,  1917654,  1983780,  2049906, 
	 2116032,  2182158,  2248284,  2314410,  2380536,  2446662,  2512788,  2578914,  2645040,  2711166,  2777292,  2843418,  2909544,  2975670,  3041796,  3107922, 
	 3174048,  3240174,  3306300,  3372426,  3438552,  3504678,  3570804,  3636930,  3703056,  3769182,  3835308,  3901434,  3967560,  4033686,  4099812,  4165938, 
	 4232064,  4298190,  4364316,  4430442,  4496568,  4562694,  4628820,  4694946,  4761072,  4827198,  4893324,  4959450,  5025576,  5091702,  5157828,  5223954, 
	 5290080,  5356206,  5422332,  5488458,  5554584,  5620710,  5686836,  5752962,  5819088,  5885214,  5951340,  6017466,  6083592,  6149718,  6215844,  6281970, 
	 6348096,  6414222,  6480348,  6546474,  6612600,  6678726,  6744852,  6810978,  6877104,  6943230,  7009356,  7075482,  7141608,  7207734,  7273860,  7339986, 
	 7406112,  7472238,  7538364,  7604490,  7670616,  7736742,  7802868,  7868994,  7935120,  8001246,  8067372,  8133498,  8199624,  8265750,  8331876,  8398002, 
};
static const int CLAMP_TAB[1024] = {
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31, 
	 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63, 
	 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95, 
	 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 
	192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 
	224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
};

void YUVtoBGRA::translate(th_ycbcr_buffer &YUVBuffer, const th_info &theoraInfo, byte *pixelData, int pixelsSize) {

	// Width and height of all buffers have to be divisible by 2.
	BS_ASSERT((YUVBuffer[0].width & 1)   == 0);
	BS_ASSERT((YUVBuffer[0].height & 1)  == 0);
	BS_ASSERT((YUVBuffer[1].width & 1)   == 0);
	BS_ASSERT((YUVBuffer[2].width & 1)  == 0);

	// UV images have to have a quarter of the Y image resolution
	BS_ASSERT(YUVBuffer[1].width  == YUVBuffer[0].width >> 1);
	BS_ASSERT(YUVBuffer[2].width  == YUVBuffer[0].width >> 1);
	BS_ASSERT(YUVBuffer[1].height == YUVBuffer[0].height >> 1);
	BS_ASSERT(YUVBuffer[2].height == YUVBuffer[0].height >> 1);

	const int *cl = &CLAMP_TAB[320];

	const byte *ySrc0 = YUVBuffer[0].data;
	const byte *ySrc1 = YUVBuffer[0].data + YUVBuffer[0].stride;
	const byte *uSrc  = YUVBuffer[1].data;
	const byte *vSrc  = YUVBuffer[2].data;
	byte *dst0  = &pixelData[0];
	byte *dst1  = &pixelData[0] + YUVBuffer[0].width * 4;

	for (int h = 0; h < YUVBuffer[0].height / 2; ++h) {
		for (int w = 0; w < YUVBuffer[0].width / 2; ++w) {
			int u = *uSrc++;
			int v = *vSrc++;

			int rUV = COEFFS_RV[v];
			int gUV = COEFFS_GU[u] + COEFFS_GV[v];
			int bUV = COEFFS_BU[u];

			int y = *ySrc0++;
			int r = COEFFS_Y[y] + rUV;
			int g = COEFFS_Y[y] + gUV;
			int b = COEFFS_Y[y] + bUV;
			*dst0++ = cl[b / PRECISION];
			*dst0++ = cl[g / PRECISION];
			*dst0++ = cl[r / PRECISION];
			*dst0++ = 255;

			y = *ySrc1++;
			r = COEFFS_Y[y] + rUV;
			g = COEFFS_Y[y] + gUV;
			b = COEFFS_Y[y] + bUV;
			*dst1++ = cl[b / PRECISION];
			*dst1++ = cl[g / PRECISION];
			*dst1++ = cl[r / PRECISION];
			*dst1++ = 255;

			y = *ySrc0++;
			r = COEFFS_Y[y] + rUV;
			g = COEFFS_Y[y] + gUV;
			b = COEFFS_Y[y] + bUV;
			*dst0++ = cl[b / PRECISION];
			*dst0++ = cl[g / PRECISION];
			*dst0++ = cl[r / PRECISION];
			*dst0++ = 255;

			y = *ySrc1++;
			r = COEFFS_Y[y] + rUV;
			g = COEFFS_Y[y] + gUV;
			b = COEFFS_Y[y] + bUV;
			*dst1++ = cl[b / PRECISION];
			*dst1++ = cl[g / PRECISION];
			*dst1++ = cl[r / PRECISION];
			*dst1++ = 255;
		}

		dst0  += YUVBuffer[0].width * 4;
		dst1  += YUVBuffer[0].width * 4;
		ySrc0 += YUVBuffer[0].stride * 2 - YUVBuffer[0].width;
		ySrc1 += YUVBuffer[0].stride * 2 - YUVBuffer[0].width;
		uSrc  += YUVBuffer[1].stride - YUVBuffer[1].width;
		vSrc  += YUVBuffer[2].stride - YUVBuffer[2].width;
	}
}

} // End of namespace Sword25

#endif
