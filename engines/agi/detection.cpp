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

#include "base/plugins.h"

#include "common/advancedDetector.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "graphics/thumbnail.h"

#include "agi/agi.h"
#include "agi/preagi.h"
#include "agi/wagparser.h"


namespace Agi {

struct AGIGameDescription {
	Common::ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

uint32 AgiBase::getGameID() const {
	return _gameDescription->gameID;
}

uint32 AgiBase::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform AgiBase::getPlatform() const {
	return _gameDescription->desc.platform;
}

Common::Language AgiBase::getLanguage() const {
	return _gameDescription->desc.language;
}

uint16 AgiBase::getVersion() const {
	return _gameDescription->version;
}

uint16 AgiBase::getGameType() const {
	return _gameDescription->gameType;
}

}

static const PlainGameDescriptor agiGames[] = {
	{"agi", "Sierra AGI game"},
	{"agi-fanmade", "Fanmade AGI game"},
	{"agidemo", "AGI Demo"},
	{"bc", "The Black Cauldron"},
	{"caitlyn", "Caitlyn's Destiny"},
	{"ddp", "Donald Duck's Playground"},
	{"goldrush", "Gold Rush!"},
	{"kq1", "King's Quest I: Quest for the Crown"},
	{"kq2", "King's Quest II: Romancing the Throne"},
	{"kq3", "King's Quest III: To Heir Is Human"},
	{"kq4", "King's Quest IV: The Perils of Rosella"},
	{"lsl1", "Leisure Suit Larry in the Land of the Lounge Lizards"},
	{"mickey", "Mickey\'s Space Adventure"},
	{"mixedup", "Mixed-Up Mother Goose"},
	{"mh1", "Manhunter 1: New York"},
	{"mh2", "Manhunter 2: San Francisco"},
	{"pq1", "Police Quest I: In Pursuit of the Death Angel"},
	{"serguei1", "Serguei's Destiny 1"},
	{"serguei2", "Serguei's Destiny 2"},
	{"sq0", "Space Quest 0: Replicated"},
	{"sq1", "Space Quest I: The Sarien Encounter"},
	{"sq2", "Space Quest II: Vohaul's Revenge"},
	{"sqx", "Space Quest X: The Lost Chapter"},
	{"tetris", "AGI Tetris"},
	{"troll", "Troll\'s Tale"},
	{"winnie", "Winnie the Pooh in the Hundred Acre Wood"},
	{"xmascard", "Xmas Card"},

	{0, 0}
};


namespace Agi {

#define GAME_LVFPN(id,name,fname,md5,size,lang,ver,features,gid,platform,interp) { \
		{ \
			id, \
			name, \
			AD_ENTRY1s(fname,md5,size),		\
			lang, \
			platform, \
			Common::ADGF_NO_FLAGS \
		}, \
		gid, \
		interp, \
		features, \
		ver, \
	}

#define GAME(id,name,md5,ver,gid) GAME_LVFPN(id,name,"logdir",md5,-1,Common::EN_ANY,ver,0,gid,Common::kPlatformPC,GType_V2)
#define GAME3(id,name,fname,md5,ver,gid) GAME_LVFPN(id,name,fname,md5,-1,Common::EN_ANY,ver,0,gid,Common::kPlatformPC,GType_V3)

#define GAME_P(id,name,md5,ver,gid,platform) GAME_LVFPN(id,name,"logdir",md5,-1,Common::EN_ANY,ver,0,gid,platform,GType_V2)

#define GAME_FP(id,name,md5,ver,flags,gid,platform) GAME_LVFPN(id,name,"logdir",md5,-1,Common::EN_ANY,ver,flags,gid,platform,GType_V2)

#define GAME_PS(id,name,md5,size,ver,gid,platform) GAME_LVFPN(id,name,"logdir",md5,size,Common::EN_ANY,ver,0,gid,platform,GType_V2)

#define GAME_LPS(id,name,md5,size,lang,ver,gid,platform) GAME_LVFPN(id,name,"logdir",md5,size,lang,ver,0,gid,platform,GType_V2)

#define GAME_LFPS(id,name,md5,size,lang,ver,flags,gid,platform) GAME_LVFPN(id,name,"logdir",md5,size,lang,ver,flags,gid,platform,GType_V2)

#define GAME3_P(id,name,fname,md5,ver,gid,platform) GAME_LVFPN(id,name,fname,md5,-1,Common::EN_ANY,ver,0,gid,platform,GType_V3)

#define GAMEpre_P(id,name,fname,md5,ver,gid,platform) GAME_LVFPN(id,name,fname,md5,-1,Common::EN_ANY,ver,0,gid,platform,GType_PreAGI)

#define GAMEpre_PS(id,name,fname,md5,size,ver,gid,platform) GAME_LVFPN(id,name,fname,md5,size,Common::EN_ANY,ver,0,gid,platform,GType_PreAGI)

#define GAME3_PS(id,name,fname,md5,size,ver,gid,platform) GAME_LVFPN(id,name,fname,md5,size,Common::EN_ANY,ver,0,gid,platform,GType_V3)

#define FANMADE_ILVF(id,name,md5,lang,ver,features) GAME_LVFPN("agi-fanmade",name,"logdir",md5,-1,lang,ver,(GF_FANMADE|features),GID_FANMADE,Common::kPlatformPC,GType_V2)

#define FANMADE_LVF(name,md5,lang,ver,features) FANMADE_ILVF("agi-fanmade",name,md5,lang,ver,features)

#define FANMADE_LF(name,md5,lang,features) FANMADE_LVF(name,md5,lang,0x2917,features)
#define FANMADE_IF(id,name,md5,features) FANMADE_ILVF(id,name,md5,Common::EN_ANY,0x2917,features)

#define FANMADE_V(name,md5,ver) FANMADE_LVF(name,md5,Common::EN_ANY,ver,0)
#define FANMADE_F(name,md5,features) FANMADE_LF(name,md5,Common::EN_ANY,features)
#define FANMADE_L(name,md5,lang) FANMADE_LF(name,md5,lang,0)
#define FANMADE_I(id,name,md5) FANMADE_IF(id,name,md5,0)

#define FANMADE(name,md5) FANMADE_F(name,md5,0)

static const AGIGameDescription gameDescriptions[] = {

	// AGI Demo 1 (PC) 05/87 [AGI 2.425]
	GAME("agidemo", "Demo 1 1987-05-20", "9c4a5b09cc3564bc48b4766e679ea332", 0x2440, GID_AGIDEMO),

	// AGI Demo 2 (IIgs) 1.0C (Censored)
	GAME_P("agidemo", "Demo 2 1987-11-24 1.0C", "580ffdc569ff158f56fb92761604f70e", 0x2917, GID_AGIDEMO, Common::kPlatformApple2GS),

	// AGI Demo 2 (PC 3.5") 11/87 [AGI 2.915]
	GAME("agidemo", "Demo 2 1987-11-24 3.5\"", "e8ebeb0bbe978172fe166f91f51598c7", 0x2917, GID_AGIDEMO),

	// AGI Demo 2 (PC 5.25") 11/87 [v1] [AGI 2.915]
	GAME("agidemo", "Demo 2 1987-11-24 [version 1] 5.25\"", "852ac303a374df62571642ca1e2d1f0a", 0x2917, GID_AGIDEMO),

	// AGI Demo 2 (PC 5.25") 01/88 [v2] [AGI 2.917]
	GAME("agidemo", "Demo 2 1987-11-25 [version 2] 5.25\"", "1503f02086ea9f388e7e041c039eaa69", 0x2917, GID_AGIDEMO),

	// AGI Demo 3 (PC) 09/88 [AGI 3.002.102]
	GAME3("agidemo", "Demo 3 1988-09-13", "dmdir", "289c7a2c881f1d973661e961ced77d74", 0x3149, GID_AGIDEMO),

	// Black Cauldron (Amiga) 2.00 6/14/87
	GAME_P("bc", "2.00 1987-06-14", "7b01694af21213b4727bb94476f64eb5", 0x2440, GID_BC, Common::kPlatformAmiga),

	// Black Cauldron (Apple IIgs) 1.0O 2/24/89 (CE)
	// Menus not tested
	GAME3_P("bc", "1.0O 1989-02-24 (CE)", "bcdir", "dc09d30b147242692f4f85b9811962db", 0x3149, GID_BC, Common::kPlatformApple2GS),

	// Black Cauldron (PC) 2.00 6/14/87 [AGI 2.439]
	GAME("bc", "2.00 1987-06-14", "7f598d4712319b09d7bd5b3be10a2e4a", 0x2440, GID_BC),

	// Black Cauldron (Russian)
	GAME_LPS("bc", "",  "b7de782dfdf8ea7dde8064f09804bcf5", 357, Common::RU_RUS, 0x2440, GID_BC, Common::kPlatformPC),

	// Black Cauldron (PC 5.25") 2.10 11/10/88 [AGI 3.002.098]
	GAME3("bc", "2.10 1988-11-10 5.25\"", "bcdir", "0c5a9acbcc7e51127c34818e75806df6", 0x3149, GID_BC),

// TODO
// These aren't supposed to work now as they require unsupported agi engine 2.01
#if 0
	// Donald Duck's Playground (Amiga) 1.0C
	// Menus not tested
	GAME_P("ddp", "1.0C 1987-04-27", "550971d196f65190a5c760d2479406ef", 0x2272, GID_DDP, Common::kPlatformAmiga),

	// Donald Duck's Playground (ST) 1.0A 8/8/86
	// Menus not tested
	GAME("ddp", "1.0A 1986-08-08", "64388812e25dbd75f7af1103bc348596", 0x2272, GID_DDP),

	// reported by Filippos (thebluegr) in bugreport #1654500
	// Menus not tested
	GAME_PS("ddp", "1.0C 1986-06-09", "550971d196f65190a5c760d2479406ef", 132, 0x2272, GID_DDP, Common::kPlatformPC),
#endif

	// Gold Rush! (Amiga) 1.01 1/13/89 aka 2.05 3/9/89	# 2.316
	GAME3_PS("goldrush", "1.01 1989-01-13 aka 2.05 1989-03-09", "dirs", "a1d4de3e75c2688c1e2ca2634ffc3bd8", 2399, 0x3149, GID_GOLDRUSH, Common::kPlatformAmiga),

	// Gold Rush! (Apple IIgs) 1.0M 2/28/89 (CE) aka 2.01 12/22/88
	// Menus not tested
	GAME3_P("goldrush", "1.0M 1989-02-28 (CE) aka 2.01 1988-12-22", "grdir", "3f7b9ce62631434389f85371b11921d6", 0x3149, GID_GOLDRUSH, Common::kPlatformApple2GS),

	// Gold Rush! (ST) 1.01 1/13/89 aka 2.01 12/22/88
	GAME3_P("goldrush", "1.01 1989-01-13 aka 2.01 1988-12-22", "grdir", "4dd4d50480a3d6c206fa227ce8142735", 0x3149, GID_GOLDRUSH, Common::kPlatformAtariST),

	// Gold Rush! (PC 5.25") 2.01 12/22/88 [AGI 3.002.149]
	GAME3("goldrush", "2.01 1988-12-22 5.25\"", "grdir", "db733d199238d4009a9e95f11ece34e9", 0x3149, GID_GOLDRUSH),

	// Gold Rush! (PC 3.5") 2.01 12/22/88 [AGI 3.002.149]
	GAME3("goldrush", "2.01 1988-12-22 3.5\"", "grdir", "6a285235745f69b4b421403659497216", 0x3149, GID_GOLDRUSH),

	{
		// Gold Rush! (PC 5.25") 2.01 12/22/88 [AGI 3.002.149]
		{
			"goldrush",
			"2.01 1988-12-22",
			{
				{ "grdir", 0, "db733d199238d4009a9e95f11ece34e9", 2399},
				{ "vol.0", 0, "4b6423d143674d3757ab1b875d25951d", 25070},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			Common::ADGF_NO_FLAGS
		},
		GID_GOLDRUSH,
		GType_V3,
		GF_MACGOLDRUSH,
		0x3149,
	},


	// King's Quest 1 (Amiga) 1.0U		# 2.082
	// The original game did not have menus, they are enabled under ScummVM
	GAME_FP("kq1", "1.0U 1986", "246c695324f1c514aee2b904fa352fad", 0x2440, GF_MENUS, GID_KQ1, Common::kPlatformAmiga),

	// King's Quest 1 (ST) 1.0V
	// The original game did not have menus, they are enabled under ScummVM
	GAME_FP("kq1", "1.0V 1986", "c3a017e556c4b0eece366a4cd9abb657", 0x2272, GF_MENUS, GID_KQ1, Common::kPlatformAtariST),

	// King's Quest 1 (IIgs) 1.0S-88223
	// Menus not tested
	GAME_P("kq1", "1.0S 1988-02-23", "f4277aa34b43d37382bc424c81627617", 0x2272, GID_KQ1, Common::kPlatformApple2GS),

	// King's Quest 1 (Mac) 2.0C
	GAME_P("kq1", "2.0C 1987-03-26", "d4c4739d4ac63f7dbd29255425077d48", 0x2440, GID_KQ1, Common::kPlatformMacintosh),

	// King's Quest 1 (PC 5.25"/3.5") 2.0F [AGI 2.917]
	GAME("kq1", "2.0F 1987-05-05 5.25\"/3.5\"", "10ad66e2ecbd66951534a50aedcd0128", 0x2917, GID_KQ1),

	// King's Quest 2 (IIgs) 2.0A 6/16/88 (CE)
	GAME_P("kq2", "2.0A 1988-06-16 (CE)", "5203c8b95250a2ecfee93ddb99414753", 0x2917, GID_KQ2, Common::kPlatformApple2GS),

	// King's Quest 2 (Amiga) 2.0J (Broken)
	GAME_P("kq2", "2.0J 1987-01-29 [OBJECT decrypted]", "b866f0fab2fad91433a637a828cfa410", 0x2440, GID_KQ2, Common::kPlatformAmiga),

	// King's Quest 2 (Mac) 2.0R
	GAME_P("kq2", "2.0R 1988-03-23", "cbdb0083317c8e7cfb7ac35da4bc7fdc", 0x2440, GID_KQ2, Common::kPlatformMacintosh),

	// King's Quest 2 (PC) 2.1 [AGI 2.411]; entry from DAGII, but missing from Sarien?
	// XXX: any major differences from 2.411 to 2.440?
	GAME("kq2", "2.1 1987-04-10", "759e39f891a0e1d86dd29d7de485c6ac", 0x2440, GID_KQ2),

	// King's Quest 2 (PC 5.25"/3.5") 2.2 [AGI 2.426]
	GAME("kq2", "2.2 1987-05-07 5.25\"/3.5\"", "b944c4ff18fb8867362dc21cc688a283", 0x2917, GID_KQ2),

	// King's Quest 2 (Russian)
	GAME_LPS("kq2", "", "35211c574ececebdc723b23e35f99275", 543, Common::RU_RUS, 0x2917, GID_KQ2, Common::kPlatformPC),

	// King's Quest 3 (Amiga) 1.01 11/8/86
	// The original game did not have menus, they are enabled under ScummVM
	GAME_FP("kq3", "1.01 1986-11-08", "8ab343306df0e2d98f136be4e8cfd0ef", 0x2440, GF_MENUS, GID_KQ3, Common::kPlatformAmiga),

	// King's Quest 3 (ST) 1.02 11/18/86
	// Does not have menus, crashes if menus are enforced. Therefore, ESC pauses the game
	GAME_FP("kq3", "1.02 1986-11-18", "8846df2654302b623217ba8bd6d657a9", 0x2272, GF_MENUS, GID_KQ3, Common::kPlatformAtariST),

	// King's Quest 3 (Mac) 2.14 3/15/88
	GAME_P("kq3", "2.14 1988-03-15", "7639c0da5ce94848227d409351fabda2", 0x2440, GID_KQ3, Common::kPlatformMacintosh),

	// King's Quest 3 (IIgs) 2.0A 8/28/88 (CE)
	GAME_P("kq3", "2.0A 1988-08-28 (CE)", "ac30b7ca5a089b5e642fbcdcbe872c12", 0x2917, GID_KQ3, Common::kPlatformApple2GS),

	// King's Quest 3 (Amiga) 2.15 11/15/89	# 2.333
	GAME3_PS("kq3", "2.15 1989-11-15", "dirs", "8e35bded2bc5cf20f5eec2b15523b155", 1805, 0x3086, GID_KQ3, Common::kPlatformAmiga),

	// King's Quest 3 (PC) 1.01 11/08/86 [AGI 2.272]
	// Does not have menus, crashes if menus are enforced. Therefore, ESC pauses the game
	GAME_FP("kq3", "1.01 1986-11-08", "9c2b34e7ffaa89c8e2ecfeb3695d444b", 0x2272, GF_ESCPAUSE, GID_KQ3, Common::kPlatformPC),

	// King's Quest 3 (Russian)
	GAME_LFPS("kq3", "", "5856dec6ccb9c4b70aee21044a19270a", 390, Common::RU_RUS, 0x2272, GF_ESCPAUSE, GID_KQ3, Common::kPlatformPC),

	// King's Quest 3 (PC 5.25") 2.00 5/25/87 [AGI 2.435]
	GAME("kq3", "2.00 1987-05-25 5.25\"", "18aad8f7acaaff760720c5c6885b6bab", 0x2440, GID_KQ3),

	// King's Quest 3 (Mac) 2.14 3/15/88
	// Menus not tested
	GAME_P("kq3", "2.14 1988-03-15 5.25\"", "7650e659c7bc0f1e9f8a410b7a2e9de6", 0x2440, GID_KQ3, Common::kPlatformMacintosh),

	// King's Quest 3 (PC 3.5") 2.14 3/15/88 [AGI 2.936]
	GAME("kq3", "2.14 1988-03-15 3.5\"", "d3d17b77b3b3cd13246749231d9473cd", 0x2936, GID_KQ3),

	// King's Quest 4 (PC 5.25") 2.3 9/27/88 [AGI 3.002.086]
	GAME3("kq4", "2.3 1988-09-27", "kq4dir", "6d7714b8b61466a5f5981242b993498f", 0x3086, GID_KQ4),

	// King's Quest 4 (PC 5.25") 2.0 (Information provided by UsualNoise on ScummVM forums). Not sure about the AGI version.
	GAME3("kq4", "2.0", "kq4dir", "f50f7f997208ca0e35b2650baec43a2d", 0x3086, GID_KQ4),

	// King's Quest 4 (IIgs) 1.0K 11/22/88 (CE)
	// Menus not tested
	GAME3_P("kq4", "1.0K 1988-11-22", "kq4dir", "8536859331159f15012e35dc82cb154e", 0x3086, GID_KQ4, Common::kPlatformApple2GS),

	// King's Quest 4 (PC 3.5") 2.0 7/27/88 [AGI 3.002.086]
	GAME3("kq4", "2.0 1988-07-27 3.5\"", "kq4dir", "fe44655c42f16c6f81046fdf169b6337", 0x3086, GID_KQ4),

	// King's Quest 4 (PC 3.5") 2.2 9/27/88 [AGI 3.002.086]
	// Menus not tested
	GAME3("kq4", "2.2 1988-09-27 3.5\"", "kq4dir", "7470b3aeb49d867541fc66cc8454fb7d", 0x3086, GID_KQ4),

	// King's Quest 4 demo (PC) [AGI 3.002.102]
	// Menus not tested
	GAME3("kq4", "Demo 1988-12-20", "dmdir", "a3332d70170a878469d870b14863d0bf", 0x3149, GID_KQ4),

	// Leisure Suit Larry 1 (PC 5.25"/3.5") 1.00 6/1/87 [AGI 2.440]
	GAME("lsl1", "1.00 1987-06-01 5.25\"/3.5\"", "1fe764e66857e7f305a5f03ca3f4971d", 0x2440, GID_LSL1),

	// Leisure Suit Larry 1 (ST) 1.04 6/18/87
	GAME_P("lsl1", "1.04 1987-06-18", "8b579f8673fe9448c2538f5ed9887cf0", 0x2440, GID_LSL1, Common::kPlatformAtariST),

	// Leisure Suit Larry 1 (Amiga) 1.05 6/26/87	# x.yyy
	GAME_P("lsl1", "1.05 1987-06-26", "3f5d26d8834ca49c147fb60936869d56", 0x2440, GID_LSL1, Common::kPlatformAmiga),

	// Leisure Suit Larry 1 (IIgs) 1.0E
	GAME_P("lsl1", "1.0E 1987", "5f9e1dd68d626c6d303131c119582ad4", 0x2440, GID_LSL1, Common::kPlatformApple2GS),

	// Leisure Suit Larry 1 (Mac) 1.05 6/26/87
	GAME_P("lsl1", "1.05 1987-06-26", "8a0076429890531832f0dc113285e31e", 0x2440, GID_LSL1, Common::kPlatformMacintosh),

	// Manhunter NY (ST) 1.03 10/20/88
	GAME3_P("mh1", "1.03 1988-10-20", "mhdir", "f2d58056ad802452d60776ee920a52a6", 0x3149, GID_MH1, Common::kPlatformAtariST),

	// Manhunter NY (IIgs) 2.0E 10/05/88 (CE)
	GAME3_P("mh1", "2.0E 1988-10-05 (CE)", "mhdir", "2f1509f76f24e6e7d213f2dadebbf156", 0x3149, GID_MH1, Common::kPlatformApple2GS),

	// Manhunter NY (Amiga) 1.06 3/18/89
	GAME3_P("mh1", "1.06 1989-03-18", "dirs", "92c6183042d1c2bb76236236a7d7a847", 0x3149, GID_MH1, Common::kPlatformAmiga),

	// reported by Filippos (thebluegr) in bugreport #1654500
	// Manhunter NY (PC 5.25") 1.22 8/31/88 [AGI 3.002.107]
	GAME3_PS("mh1", "1.22 1988-08-31", "mhdir", "0c7b86f05fe02c2e26cff1b07450b82a", 2123, 0x3149, GID_MH1, Common::kPlatformPC),

	// Manhunter NY (PC 3.5") 1.22 8/31/88 [AGI 3.002.102]
	GAME3_PS("mh1", "1.22 1988-08-31", "mhdir", "5b625329021ad49fd0c1d6f2d6f54bba", 2141, 0x3149, GID_MH1, Common::kPlatformPC),

	// Manhunter SF (ST) 1.0 7/29/89
	GAME3_P("mh2", "1.0 1989-07-29", "mh2dir", "5e3581495708b952fea24438a6c7e040", 0x3149, GID_MH1, Common::kPlatformAtariST),

	// Manhunter SF (Amiga) 3.06 8/17/89		# 2.333
	GAME3_PS("mh2", "3.06 1989-08-17", "dirs", "b412e8a126368b76696696f7632d4c16", 2573, 0x3086, GID_MH2, Common::kPlatformAmiga),

	// Manhunter SF (PC 5.25") 3.03 8/17/89 [AGI 3.002.149]
	GAME3("mh2", "3.03 1989-08-17 5.25\"", "mh2dir", "b90e4795413c43de469a715fb3c1fa93", 0x3149, GID_MH2),

	// Manhunter SF (PC 3.5") 3.02 7/26/89 [AGI 3.002.149]
	GAME3("mh2", "3.02 1989-07-26 3.5\"", "mh2dir", "6fb6f0ee2437704c409cf17e081ba152", 0x3149, GID_MH2),

	// Mickey's Space Adventure
	// Preagi game
	GAMEpre_P("mickey", "", "1.pic", "b6ec04c91a05df374792872c4d4ce66d", 0x0000, GID_MICKEY, Common::kPlatformPC),

#if 0
	// Mixed-Up Mother Goose (Amiga) 1.1
	// Problematic: crashes
	// Menus not tested
	GAME3_PS("mixedup", "1.1 1986-12-10", "dirs", "5c1295fe6daaf95831195ba12894dbd9", 2021, 0x3086, GID_MIXEDUP, Common::kPlatformAmiga),
#endif

	// Mixed Up Mother Goose (IIgs)
	GAME_P("mixedup", "1987", "3541954a7303467c6df87665312ffb6a", 0x2917, GID_MIXEDUP, Common::kPlatformApple2GS),

	// Mixed-Up Mother Goose (PC) [AGI 2.915]
	GAME("mixedup", "1987-11-10", "e524655abf9b96a3b179ffcd1d0f79af", 0x2917, GID_MIXEDUP),

#if 0
	// Mixed Up Mother Goose (PC) [AGI 2.915] (Broken)
	// Menus not tested
	GAME("mixedup", "[corrupt/OBJECT from disk 1]", "e524655abf9b96a3b179ffcd1d0f79af", 0x2917, GID_MIXEDUP),
#endif

	// Police Quest 1 (PC) 2.0E 11/17/87 [AGI 2.915]
	GAME("pq1", "2.0E 1987-11-17", "2fd992a92df6ab0461d5a2cd83c72139", 0x2917, GID_PQ1),

	// Police Quest 1 (Mac) 2.0G 12/3/87
	GAME_P("pq1", "2.0G 1987-12-03", "805750b66c1c5b88a214e67bfdca17a1", 0x2440, GID_PQ1, Common::kPlatformMacintosh),

	// Police Quest 1 (IIgs) 2.0B-88421
	GAME_P("pq1", "2.0B 1988-04-21", "e7c175918372336461e3811d594f482f", 0x2917, GID_PQ1, Common::kPlatformApple2GS),

	// Police Quest 1 (Amiga) 2.0B 2/22/89	# 2.310
	GAME3_PS("pq1", "2.0B 1989-02-22", "dirs", "cfa93e5f2aa7378bddd10ad6746a2ffb", 1613, 0x3149, GID_PQ1, Common::kPlatformAmiga),

	// Police Quest 1 (IIgs) 2.0A-88318
	GAME_P("pq1", "2.0A 1988-03-18", "8994e39d0901de3d07cecfb954075bb5", 0x2917, GID_PQ1, Common::kPlatformApple2GS),

	// Police Quest 1 (PC) 2.0A 10/23/87 [AGI 2.903/2.911]
	GAME("pq1", "2.0A 1987-10-23", "b9dbb305092851da5e34d6a9f00240b1", 0x2917, GID_PQ1),

	// Police Quest 1 (Russian)
	GAME_LPS("pq1", "", "604cc8041d24c4c7e5fa8baf386ef76e", 360, Common::RU_RUS, 0x2917, GID_PQ1, Common::kPlatformPC),

	// Police Quest 1 2.0G 12/3/87
	GAME("pq1", "2.0G 1987-12-03 5.25\"/ST", "231f3e28170d6e982fc0ced4c98c5c1c", 0x2440, GID_PQ1),

	// Police Quest 1 (PC) 2.0G 12/3/87; entry from DAGII, but missing from Sarien?
	// not sure about disk format -- dsymonds
	GAME("pq1", "2.0G 1987-12-03", "d194e5d88363095f55d5096b8e32fbbb", 0x2917, GID_PQ1),

	// Space Quest 1 (ST) 1.1A
	// The original game did not have menus, they are enabled under ScummVM
	GAME_FP("sq1", "1.1A 1986-02-06", "6421fb64b0e6604c9dd065975d9279e9", 0x2440, GF_MENUS, GID_SQ1, Common::kPlatformAtariST),

	// Space Quest 1 (PC) 1.1A [AGI 2.272]
	// The original game did not have menus, they are enabled under ScummVM
	GAME_FP("sq1", "1.1A 1986-11-13", "8d8c20ab9f4b6e4817698637174a1cb6", 0x2272, GF_MENUS, GID_SQ1, Common::kPlatformPC),

	// The original game did not have menus, they are enabled under ScummVM
	// Space Quest 1 (Amiga) 1.2			# 2.082
	GAME_FP("sq1", "1.2 1986", "0b216d931e95750f1f4837d6a4b821e5", 0x2440, GF_MENUS | GF_OLDAMIGAV20, GID_SQ1, Common::kPlatformAmiga),

	// Space Quest 1 (Mac) 1.5D
	GAME_P("sq1", "1.5D 1987-04-02", "ce88419aadd073d1c6682d859b3d8aa2", 0x2440, GID_SQ1, Common::kPlatformMacintosh),

	// Space Quest 1 (IIgs) 2.2
	GAME_P("sq1", "2.2 1987", "64b9b3d04c1066d36e6a6e56187a83f7", 0x2917, GID_SQ1, Common::kPlatformApple2GS),

	// Space Quest 1 (PC) 1.0X [AGI 2.089]
	// Does not have menus, crashes if menus are enforced. Therefore, ESC pauses the game
	GAME_FP("sq1", "1.0X 1986-09-24", "af93941b6c51460790a9efa0e8cb7122", 0x2089, GF_ESCPAUSE, GID_SQ1, Common::kPlatformPC),

	// Space Quest 1 (Russian)
	GAME_LFPS("sq1", "", "a279eb8ddbdefdb1ea6adc827a1d632a", 372, Common::RU_RUS, 0x2089, GF_ESCPAUSE, GID_SQ1, Common::kPlatformPC),

	// Space Quest 1 (PC 5.25"/3.5") 2.2 [AGI 2.426/2.917]
	GAME("sq1", "2.2 1987-05-07 5.25\"/3.5\"", "5d67630aba008ec5f7f9a6d0a00582f4", 0x2440, GID_SQ1),

	// Space Quest 2 (PC 3.5") 2.0D [AGI 2.936]
	GAME("sq2", "2.0D 1988-03-14 3.5\"", "85390bde8958c39830e1adbe9fff87f3", 0x2936, GID_SQ2),

	// Space Quest 2 (IIgs) 2.0A 7/25/88 (CE)
	GAME_P("sq2", "2.0A 1988-07-25 (CE)", "5dfdac98dd3c01fcfb166529f917e911", 0x2936, GID_SQ2, Common::kPlatformApple2GS),

	{
		// Space Quest 2 (Amiga) 2.0F
		{
			"sq2",
			"2.0F 1986-12-09 [VOL.2->PICTURE.16 broken]",
			{
				{ "logdir", 0, "28add5125484302d213911df60d2aded", 426},
				{ "object", 0, "5dc52be721257719f4b311a84ce22b16", 372},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			Common::ADGF_NO_FLAGS
		},
		GID_SQ2,
		GType_V2,
		0,
		0x2936,
	},


	// Space Quest 2 (Mac) 2.0D
	GAME_P("sq2", "2.0D 1988-04-04", "bfbebe0b59d83f931f2e1c62ce9484a7", 0x2936, GID_SQ2, Common::kPlatformMacintosh),

	// reported by Filippos (thebluegr) in bugreport #1654500
	// Space Quest 2 (PC 5.25") 2.0A [AGI 2.912]
	GAME_PS("sq2", "2.0A 1987-11-06 5.25\"", "ad7ce8f800581ecc536f3e8021d7a74d", 423, 0x2917, GID_SQ2, Common::kPlatformPC),

	// Space Quest 2 (Russian)
	GAME_LPS("sq2", "", "ba21c8934caf28e3ba45ce7d1cd6b041", 423, Common::RU_RUS, 0x2917, GID_SQ2, Common::kPlatformPC),

	// Space Quest 2 (PC 3.5") 2.0A [AGI 2.912]
	GAME_PS("sq2", "2.0A 1987-11-06 3.5\"", "6c25e33d23b8bed42a5c7fa63d588e5c", 423, 0x2917, GID_SQ2, Common::kPlatformPC),

	// Space Quest 2 (PC 5.25"/ST) 2.0C/A [AGI 2.915]
	// Menus not tested
	GAME("sq2", "2.0C/A 5.25\"/ST", "bd71fe54869e86945041700f1804a651", 0x2917, GID_SQ2),

	// Space Quest 2 (PC 3.5") 2.0F [AGI 2.936]
	GAME("sq2", "2.0F 1989-01-05 3.5\"", "28add5125484302d213911df60d2aded", 0x2936, GID_SQ2),

	// Troll's Tale
	GAMEpre_PS("troll", "", "troll.img", "62903f264b3d849be4214b3a5c42a2fa", 184320, 0x0000, GID_TROLL, Common::kPlatformPC),
	// Winnie the Pooh in the Hundred Acre Wood
	GAMEpre_P("winnie", "", "title.pic", "2e7900c1ccaa7671d65405f6d1efed30", 0x0000, GID_WINNIE, Common::kPlatformPC),

	// Winnie the Pooh in the Hundred Acre Wood (Amiga)
	GAMEpre_P("winnie", "", "title", "2e7900c1ccaa7671d65405f6d1efed30", 0x0000, GID_WINNIE, Common::kPlatformAmiga),

	// Winnie the Pooh in the Hundred Acre Wood (C64)
	GAMEpre_P("winnie", "", "title.pic", "d4eb97cffc866110f71e1ec9f84fe643", 0x0000, GID_WINNIE, Common::kPlatformC64),

	// Winnie the Pooh in the Hundred Acre Wood (Apple //gs)
	GAMEpre_P("winnie", "", "title.pic", "45e06010a3c61d78f4661103c901ae11", 0x0000, GID_WINNIE, Common::kPlatformApple2GS),

	// Xmas Card 1986 (PC) [AGI 2.272]
	GAME("xmascard", "1986-11-13 [version 1]", "3067b8d5957e2861e069c3c0011bd43d", 0x2272, GID_XMASCARD),

	FANMADE_F("2 Player Demo", "4279f46b3cebd855132496476b1d2cca", GF_AGIMOUSE),
	FANMADE("AGI Contest 1 Template", "d879aed25da6fc655564b29567358ae2"),
	FANMADE("AGI Contest 2 Template", "5a2fb2894207eff36c72f5c1b08bcc07"),
	FANMADE("AGI Mouse Demo 0.60 demo 1", "c07e2519de674c67386cb2cc6f2e3904"),
	FANMADE("AGI Mouse Demo 0.60 demo 2", "cc49d8b88ed6faf4f53ce92c84e0fe1b"),
	FANMADE("AGI Mouse Demo 0.70", "3497c291e4afb6f758e61740678a2aec"),
	FANMADE_F("AGI Mouse Demo 1.00", "20397f0bf0ef936f416bb321fb768fc7", GF_AGIMOUSE),
	FANMADE_F("AGI Mouse Demo 1.10", "f4ad396b496d6167635ad0b410312ab8", GF_AGIMOUSE|GF_AGIPAL),
	FANMADE("AGI Piano (v1.0)", "8778b3d89eb93c1d50a70ef06ef10310"),
	FANMADE("AGI Quest (v1.46-TJ0)", "1cf1a5307c1a0a405f5039354f679814"),
	FANMADE_I("tetris", "", "7a874e2db2162e7a4ce31c9130248d8a"),
	FANMADE_V("AGI Trek (Demo)", "c02882b8a8245b629c91caf7eb78eafe", 0x2440),
	FANMADE_F("AGI256 Demo", "79261ac143b2e2773b2753674733b0d5", GF_AGI256),
	FANMADE_F("AGI256-2 Demo", "3cad9b3aff1467cebf0c5c5b110985c5", GF_AGI256_2),
	FANMADE_LF("Abrah: L'orphelin de l'espace (v1.2)", "b7b6d1539e14d5a26fa3088288e1badc", Common::FR_FRA, GF_AGIPAL),
	FANMADE("Acidopolis", "7017db1a4b726d0d59e65e9020f7d9f7"),
	FANMADE("Agent 0055 (v1.0)", "c2b34a0c77acb05482781dda32895f24"),
	FANMADE("Agent 06 vs. The Super Nazi", "136f89ca9f117c617e88a85119777529"),
	FANMADE("Agent Quest", "59e49e8f72058a33c00d60ee1097e631"),
	FANMADE("Al Pond - On Holiday (v1.0)", "a84975496b42d485920e886e92eed68b"),
	FANMADE("Al Pond - On Holiday (v1.1)", "7c95ac4689d0c3bfec61e935f3093634"),
	FANMADE("Al Pond - On Holiday (v1.3)", "8f30c260de9e1dd3d8b8f89cc19d2633"),
	FANMADE("Al Pond 1 - Al Lives Forever (v1.0)", "e8921c3043b749b056ff51f56d1b451b"),
	FANMADE("Al Pond 1 - Al Lives Forever (v1.3)", "fb4699474054962e0dbfb4cf12ca52f6"),
	FANMADE("Apocalyptic Quest (v0.03 Teaser)", "42ced528b67965d3bc3b52c635f94a57"),
	FANMADE_F("Apocalyptic Quest (v4.00 Alpha 1)", "e15581628d84949b8d352d224ec3184b", GF_AGIMOUSE),
	FANMADE_F("Apocalyptic Quest (v4.00 Alpha 2)", "0eee850005860e46345b38fea093d194", GF_AGIMOUSE),
	FANMADE_F("Band Quest (Demo)", "7326abefd793571cc17ed0db647bdf34", GF_AGIMOUSE),
	FANMADE_F("Band Quest (Early Demo)", "de4758dd34676b248c8301b32d93bc6f", GF_AGIMOUSE),
	FANMADE("Beyond the Titanic 2", "9b8de38dc64ffb3f52b7877ea3ebcef9"),
	FANMADE("Biri Quest 1", "1b08f34f2c43e626c775c9d6649e2f17"),
	FANMADE("Bob The Farmboy", "e4b7df9d0830addee5af946d380e66d7"),
	FANMADE_F("Boring Man 1: The Toad to Robinland", "d74481cbd227f67ace37ce6a5493039f", GF_AGIMOUSE),
	FANMADE_F("Boring Man 2: Ho Man! This Game Sucks!", "250032ba105bdf7c1bc4fed767c2d37e", GF_AGIMOUSE),
	FANMADE("Botz", "a8fabe4e807adfe5ec02bfec6d983695"),
	FANMADE("Brian's Quest (v1.0)", "0964aa79b9cdcff7f33a12b1d7e04b9c"),
	FANMADE("CPU-21 (v1.0)", "35b7cdb4d17e890e4c52018d96e9cbf4"),
	FANMADE_I("caitlyn", "Demo", "5b8a3cdb2fc05469f8119d49f50fbe98"),
	FANMADE_I("caitlyn", "", "818469c484cae6dad6f0e9a353f68bf8"),
	FANMADE("Car Driver (v1.1)", "2311611d2d36d20ccc9da806e6cba157"),
	FANMADE("Coco Coq (English) - Coco Coq In Grostesteing's Base (v.1.0.3)", "97631f8e710544a58bd6da9e780f9320"),
	FANMADE_L("Coco Coq (French) - Coco Coq Dans la Base de Grostesteing (v1.0.2)", "ef579ebccfe5e356f9a557eb3b2d8649", Common::FR_FRA),
	FANMADE("Corby's Murder Mystery (v1.0)", "4ebe62ac24c5a8c7b7898c8eb070efe5"),
	FANMADE_F("DG: The AGIMouse Adventure (English v1.1)", "efe453b92bc1487ea69fbebede4d5f26", GF_AGIMOUSE|GF_AGIPAL),
	FANMADE_LF("DG: The AGIMouse Adventure (French v1.1)", "eb3d17ca466d672cbb95947e8d6e846a", Common::FR_FRA, GF_AGIMOUSE|GF_AGIPAL),
	FANMADE("DG: The Adventure Game (English v1.1)", "0d6376d493fa7a21ec4da1a063e12b25"),
	FANMADE_L("DG: The Adventure Game (French v1.1)", "258bdb3bb8e61c92b71f2f456cc69e23", Common::FR_FRA),
	FANMADE("Dashiki (16 Colors)", "9b2c7b9b0283ab9f12bedc0cb6770a07"),
	FANMADE_F("Dashiki (256 Colors)", "c68052bb209e23b39b55ff3d759958e6", GF_AGIMOUSE|GF_AGI256),
	FANMADE("Date Quest 1 (v1.0)", "ba3dcb2600645be53a13170aa1a12e69"),
	FANMADE("Date Quest 2 (v1.0 Demo)", "1602d6a2874856e928d9a8c8d2d166e9"),
	FANMADE("Date Quest 2 (v1.0)", "f13f6fc85aa3e6e02b0c20408fb63b47"),
	FANMADE("Dave's Quest (v0.07)", "f29c3660de37bacc1d23547a167f27c9"),
	FANMADE("Dave's Quest (v0.17)", "da3772624cc4a86f7137db812f6d7c39"),
	FANMADE("Disco Nights (Demo)", "dc5a2b21182ba38bdcd992a3a978e690"),
	FANMADE("Dogs Quest - The Quest for the Golden Bone (v1.0)", "f197357edaaea0ff70880602d2f09b3e"),
	FANMADE("Dr. Jummybummy's Space Adventure", "988bd81785f8a452440a2a8ac67f96aa"),
	FANMADE("Ed Ward", "98be839b9f30cbedea4c9cee5442d827"),
	FANMADE("Elfintard", "c3b847e9e9e978af9708df76a0751dc2"),
	FANMADE("Enclosure (v1.01)", "f08e66fee9ecdde77db7ee9a10c96ba2"),
	FANMADE("Enclosure (v1.03)", "e4a0613ed02401502e506ba3565a8c40"),
	FANMADE("Epic Fighting (v0.1)", "aff24a1b3bdd676187685c4d95ba4294"),
	FANMADE("Escape Quest (v0.0.3)", "2346b65619b1da0298b715b06d1a45a1"),
	FANMADE("Escape from the Desert (beta 1)", "dfdc634d340854bd6ece28024010758d"),
	FANMADE("Escape from the Salesman", "e723ca4fe0f6f56affe039fbb4dbeb6c"),
	FANMADE("Fu$k Quest 1 (final)", "1cd0587422313f6ca77d6a95988e88ed"),
	FANMADE("Fu$k Quest 1", "1cd0587422313f6ca77d6a95988e88ed"),
	FANMADE("Fu$k Quest 2 - Romancing the Bone (Teaser)", "d288355d71d9bb1639260ccaa3b2fbfe"),
	FANMADE("Fu$k Quest 2 - Romancing the Bone", "294beeb7765c7ea6b05ed7b9bf7bff4f"),
	FANMADE("Gennadi Tahab Autot - Mission Pack 1 - Kuressaare", "bfa5fe71978e6ccf3d4eedd430124015"),
	FANMADE("Go West, Young Hippie", "ff31484ea465441cb5f3a0f8e956b716"),
	FANMADE("Good Man (demo v3.41)", "3facd8a8f856b7b6e0f6c3200274d88c"),

	{
		// Groza
		{
			"agi-fanmade",
			"Groza (russian) [AGDS sample]",
			AD_ENTRY1("logdir", "421da3a18004122a966d64ab6bd86d2e"),
			Common::RU_RUS,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_FANMADE,
		GType_V2,
		GF_AGDS,
		0x2440,
	},

	{
		// Get Outta SQ
		{
			"agi-fanmade",
			"Get Outta Space Quest",
			AD_ENTRY1("logdir", "aaea5b4a348acb669d13b0e6f22d4dc9"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GID_GETOUTTASQ,
		GType_V2,
		0,
		0x2440,
	},

	FANMADE_F("Half-Death - Terror At White-Mesa", "b62c05d0ace878261392073f57ae788c", GF_AGIMOUSE),
	FANMADE("Hank's Quest (v1.0 English) - Victim of Society", "64c15b3d0483d17888129100dc5af213"),
	FANMADE("Hank's Quest (v1.1 English) - Victim of Society", "86d1f1dd9b0c4858d096e2a60cca8a14"),
	FANMADE_L("Hank's Quest (v1.81 Dutch) - Slachtoffer Van Het Gebeuren", "41e53972d55ff3dff9e90d15fe1b659f", Common::NL_NLD),
	FANMADE("Hank's Quest (v1.81 English) - Victim of Society", "7a776383282f62a57c3a960dafca62d1"),
	FANMADE("Herbao (v0.2)", "6a5186fc8383a9060517403e85214fc2"),
	FANMADE_F("Hitler's Legacy (v.0004q)", "a412881269ba34584bd0a3268e5a9863", GF_AGIMOUSE),
	FANMADE("Hobbits", "4a1c1ef3a7901baf0ab45fde0cfadd89"),
	FANMADE_F("Isabella Coq - A Present For My Dad", "55c6819f2330c4d5d6459874c9f123d9", GF_AGIMOUSE),
	FANMADE("Jack & Julia - VAMPYR", "8aa0b9a26f8d5a4421067ab8cc3706f6"),
	FANMADE("Jeff's Quest (v.5 alpha Jun 1)", "10f1720eed40c12b02a0f32df3e72ded"),
	FANMADE("Jeff's Quest (v.5 alpha May 31)", "51ff71c0ed90db4e987a488ed3bf0551"),
	FANMADE("Jen's Quest (Demo 1)", "361afb5bdb6160213a1857245e711939"),
	FANMADE("Jen's Quest (Demo 2)", "3c321eee33013b289ab8775449df7df2"),
	FANMADE("Jiggy Jiggy Uh! Uh!", "bc331588a71e7a1c8840f6cc9b9487e4"),
	FANMADE("Jimmy In: The Alien Attack (v0.1)", "a4e9db0564a494728de7873684a4307c"),
	FANMADE("Joe McMuffin In \"What's Cooking, Doc\" (v1.0)", "8a3de7e61a99cb605fa6d233dd91c8e1"),
	FANMADE_LVF("Jolimie, le Village Maudit (v0.5)", "21818501636b3cb8ad5de5c1a66de5c2", Common::FR_FRA, 0x2936, GF_AGIMOUSE|GF_AGIPAL),
	FANMADE_LVF("Jolimie, le Village Maudit (v1.1)", "68d7aef1161bb5972fe03efdf29ccb7f", Common::FR_FRA, 0x2936, GF_AGIMOUSE|GF_AGIPAL),
	FANMADE("Journey Of Chef", "aa0a0b5a6364801ae65fdb96d6741df5"),
	FANMADE("Jukebox (v1.0)", "c4b9c5528cc67f6ba777033830de7751"),
	FANMADE("Justin Quest (v1.0 in development)", "103050989da7e0ffdc1c5e1793a4e1ec"),
	FANMADE("J\xf5ulumaa (v0.05) (Estonian)", "53982ecbfb907e41392b3961ad1c3475"),
	FANMADE("Kings Quest 2  - Breast Intentions (v2.0 Mar 26)", "a25d7379d281b1b296d4785df90a8e78"),
	FANMADE("Kings Quest 2  - Breast Intentions (v2.0 Aug 16)", "6b4f796d0421d2e12e501b511962e03a"),
	FANMADE("Lasse Holm: The Quest for Revenge (v1.0)", "f9fbcc8a4ef510bfbb92423296ff4abb"),
	FANMADE("Lawman for Hire", "c78b28bfd3767dd455b992cd8b7854fa"),
	FANMADE("Lefty Goes on Vacation (Not in The Right Place)", "ccdc49a33870310b01f2c48b8a1f3c34"),
	FANMADE("Les Ins\xe3parables (v1.0)", "4b780887cab0ecabc5eca319acb3acf2"),
	FANMADE("Little Pirate (Demo 2 v0.6)", "437068efe4ec32d436da09d6f2ea56e1"),
	FANMADE("Lost Eternity (v1.0)", "95f15c5632feb8a39e9ca3d9af35fcc9"),
	FANMADE("MD Quest - The Search for Michiel (v0.10)", "2a6fcb21d2b5e4144c38ed817fabe8ee"),
	FANMADE("Maale Adummin Quest", "ddfbeb33feb7cf78504fe4dba14ec63b"),
	FANMADE("Monkey Man", "2322d03f997e8cc235d4578efff69cfa"),
	FANMADE_F("Napalm Quest (v0.5)", "b659afb491d967bb34810d1c6ce22093", GF_AGIMOUSE),
	FANMADE("Naturette 1 (English v1.2)", "0a75884e7f010974a230bdf269651117"),
	FANMADE("Naturette 1 (English v1.3)", "f15bbf999ac55ebd404aa1eb84f7c1d9"),
	FANMADE_L("Naturette 1 (French v1.2)", "d3665622cc41aeb9c7ecf4fa43f20e53", Common::FR_FRA),
	FANMADE_F("Naturette 2: Daughter of the Moon (v1.0)", "bdf76a45621c7f56d1c9d40292c6137a", GF_AGIMOUSE|GF_AGIPAL),
	FANMADE_F("Naturette 3: Adventure in Treeworld (v1.0a)", "6dbb0e7fc75fec442e6d9e5a06f1530e", GF_AGIMOUSE|GF_AGIPAL),
	FANMADE_F("Naturette 4: From a Planet to Another Planet (Not Finished)", "13be8cd9cf35aeff0a39b8757057fbc8", GF_AGIMOUSE),
	// FIXME: Actually Naturette 4 has both English and French language support built into it. How to add that information?
	FANMADE_F("Naturette 4: From a Planet to Another Planet (2007-10-05)", "8253706b6ef5423a79413b216760297c", GF_AGIMOUSE|GF_AGIPAL),
	FANMADE("New AGI Hangman Test", "d69c0e9050ccc29fd662b74d9fc73a15"),
	FANMADE("Nick's Quest - In Pursuit of QuakeMovie (v2.1 Gold)", "e29cbf9222551aee40397fabc83eeca0"),
	FANMADE_F("Open Mic Night (v0.1)", "70000a2f67aac27d1133d019df70246d", GF_AGIMOUSE|GF_AGIPAL),
	FANMADE("Operation: Recon", "0679ce8405411866ccffc8a6743370d0"),
	FANMADE("Patrick's Quest (Demo v1.0)", "f254f5b894b98fec5f92acc07fb62841"),
	FANMADE("Phantasmagoria", "87d20c1c11aee99a4baad3797b63146b"),
	FANMADE("Pharaoh Quest (v0.0)", "51c630899d076cf799e573dadaa2276d"),
	FANMADE("Phil's Quest - the Search for Tolbaga", "5e7ca45c360e03164b8358e49900c588"),
	FANMADE("Pinkun Maze Quest (v0.1)", "148ff0843af389928b3939f463bfd20d"),
	FANMADE("Pirate Quest", "bb612a919ed2b9ea23bbf03ce69fed42"),
	FANMADE("Pothead (v0.1)", "d181101385d3a45082f418cd4b3c5b01"),
	FANMADE("President's Quest", "4937d0e8ecadb7888faeb347799b0388"),
	FANMADE("Prince Quest", "266248d75c3130c8ccc9c9bf2ad30a0d"),
	FANMADE("Professor (English) - The Professor is Missing (Mar 17)", "6232de31cc204affdf2e92dfe3dc0e4d"),
	FANMADE("Professor (English) - The Professor is Missing (Mar 22)", "b5fcf0ca2f0d1c073be82f01e2170961"),
	FANMADE_L("Professor (French) - Le Professeur a Disparu", "7d9f8a4d4610bb9b0b97caa17590c2d3", Common::FR_FRA),
	FANMADE("Quest for Glory VI - Hero's Adventure", "d26765c3075064c80d284c5e06e33a7e"),
	FANMADE("Quest for Home", "d2895dc1cd3930f2489af0f843b144b3"),
	FANMADE("Quest for Ladies (demo v1.1 Apr 1)", "3f6e02f16e1154a0daf296c8895edd97"),
	FANMADE("Quest for Ladies (demo v1.1 Apr 6)", "f75e7b6a0769a3fa926eea0854711591"),
	FANMADE("Quest for Piracy 1 - Enter the Silver Pirate (v0.15)", "d23f5c2a26f6dc60c686f8a2436ea4a6"),
	FANMADE("Quest for a Record Deal", "f4fbd7abf056d2d3204f790da5ac89ab"),
	FANMADE("Ralph's Quest (v0.1)", "5cf56378aa01a26ec30f25295f0750ca"),
	FANMADE("Residence 44 Quest (Dutch v0.99)", "7c5cc64200660c70240053b33d379d7d"),
	FANMADE("Residence 44 Quest (English v0.99)", "fe507851fddc863d540f2bec67cc67fd"),
	FANMADE("Residence 44 Quest (English v1.0a)", "f99e3f69dc8c77a45399da9472ef5801"),
	FANMADE("SQ2Eye (v0.3)", "2be2519401d38ad9ce8f43b948d093a3"),
	//	FANMADE("SQ2Eye (v0.4)", "2be2519401d38ad9ce8f43b948d093a3"),
	FANMADE("SQ2Eye (v0.41)", "f0e82c55f10eb3542d7cd96c107ae113"),
	FANMADE("SQ2Eye (v0.42)", "d7beae55f6328ef8b2da47b1aafea40c"),
	FANMADE("SQ2Eye (v0.43)", "2a895f06e45de153bb4b77c982009e06"),
	FANMADE("SQ2Eye (v0.44)", "5174fc4b6d8a477ba0ff0575cd64e0aa"),
	FANMADE("SQ2Eye (v0.45)", "6e06f8bb7b90ce6f6aabf1a0e620159c"),
	FANMADE("SQ2Eye (v0.46)", "bf0ad7a035ff9113951d09d1efe380c4"),
	FANMADE("SQ2Eye (v0.47)", "85dc3be1d33ff932c292b74f9037abaa"),
	FANMADE("SQ2Eye (v0.48)", "587574252972a5b5c070a647973a9b4a"),
	FANMADE("SQ2Eye (v0.481)", "fc9234beb49804ae869696ce5af8ef30"),
	FANMADE("SQ2Eye (v0.482)", "3ed84b7b87fa6840f25c15f250a11ffb"),
	FANMADE("SQ2Eye (v0.483)", "647c31298d3f9cda641231b893e347c0"),
	FANMADE("SQ2Eye (v0.484)", "f2c86fae7b9046d408c62c8c49a4b882"),
	FANMADE("SQ2Eye (v0.485)", "af59e36bc28f44545458b68a93e91e67"),
	FANMADE("SQ2Eye (v0.486)", "3fd86436e93456770dbdd4593eded70a"),
	FANMADE("Save Santa (v1.0)", "4644f6beb5802081772f14be56ae196c"),
	FANMADE("Save Santa (v1.3)", "f8afdb6efc5af5e7c0228b44633066af"),
	FANMADE("Schiller (preview 1)", "ade39dea968c959cfebe1cf935d653e9"),
	FANMADE("Schiller (preview 2)", "62cd1f8fc758bf6b4aa334e553624cef"),
	FANMADE_IF("serguei1", "(v1.0)", "b86725f067e456e10cdbdf5f58e01dec", GF_AGIMOUSE|GF_AGIPAL),
	FANMADE_IF("serguei1", "v1.1 2002 Sep 5", "91975c1fb4b13b0f9a8e9ff74731030d", GF_AGIMOUSE|GF_AGIPAL),
	FANMADE_IF("serguei1", "v1.1 2003 Apr 10", "91975c1fb4b13b0f9a8e9ff74731030d", GF_AGIMOUSE|GF_AGIPAL),
	FANMADE_IF("serguei2", "v0.1.1 Demo", "906ccbc2ddedb29b63141acc6d10cd28", GF_AGIMOUSE),
	FANMADE_IF("serguei2", "v1.3.1 Demo (March 22nd 2008)", "ad1308fcb8f48723cd388e012ebf5e20", GF_AGIMOUSE|GF_AGIPAL),
	FANMADE("Shifty (v1.0)", "2a07984d27b938364bf6bd243ac75080"),
	FANMADE_F("Sliding Tile Game (v1.00)", "949bfff5d8a81c3139152eed4d84ca75", GF_AGIMOUSE),
	FANMADE("Snowboarding Demo (v1.0)", "24bb8f29f1eddb5c0a099705267c86e4"),
	FANMADE("Solar System Tour", "b5a3d0f392dfd76a6aa63f3d5f578403"),
	FANMADE("Sorceror's Appraisal", "fe62615557b3cb7b08dd60c9d35efef1"),
	FANMADE_I("sq0", "v1.03", "d2fd6f7404e86182458494e64375e590"),
	FANMADE_I("sq0", "v1.04", "2ad9d1a4624a98571ee77dcc83f231b6"),
	FANMADE_I("sqx", "v10.0 Feb 05", "c992ae2f8ab18360404efdf16fa9edd1"),
	FANMADE_I("sqx", "v10.0 Jul 18", "812edec45cefad559d190ffde2f9c910"),
	FANMADE_F("Space Trek (v1.0)", "807a1aeadb2ace6968831d36ab5ea37a", GF_CLIPCOORDS),
	FANMADE("Special Delivery", "88764dfe61126b8e73612c851b510a33"),
	FANMADE("Speeder Bike Challenge (v1.0)", "2deb25bab379285ca955df398d96c1e7"),
	FANMADE("Star Commander 1 - The Escape (v1.0)", "a7806f01e6fa14ebc029faa58f263750"),
	FANMADE("Star Pilot: Bigger Fish", "8cb26f8e1c045b75c6576c839d4a0172"),
	FANMADE_F("Street Quest (Demo)", "cf2aa94a7eb78dce6892c37f03e310d6", GF_AGIPAL),
	FANMADE("Tales of the Tiki", "8103c9c87e3964690a14a3d0d83f7ddc"),
	FANMADE("Tex McPhilip 1 - Quest For The Papacy", "3c74b9a24b51aa8020ac82bee3132266"),
	FANMADE("Tex McPhilip 2 - Road To Divinity (v1.5)", "7387e8df854440bc26620ca0ea43af9a"),
	FANMADE("Tex McPhilip 3 - A Destiny of Sin (Demo v0.25)", "992d12031a486ad84e592ff5d7c9d782"),
	FANMADE("The 13th Disciple (v1.00)", "887719ad59afce9a41ec057dbb73ad73"),
	FANMADE("The Adventures of a Crazed Hermit", "6e3086cbb794d3299a9c5a9792295511"),
	FANMADE("The Grateful Dead", "c2146631afacf8cb455ce24f3d2d46e7"),
	FANMADE("The Legend of Shay-Larah 1 - The Lost Prince", "04e720c8e30c9cf12db22ea14a24a3dd"),
	FANMADE("The Legend of Zelda: The Fungus of Time (Demo v1.00)", "dcaf8166ceb62a3d9b9aea7f3b197c09"),
	FANMADE("The Legendary Harry Soupsmith (Demo 1998 Apr 2)", "64c46b0d6fc135c9835afa80980d2831"),
	FANMADE("The Legendary Harry Soupsmith (Demo 1998 Aug 19)", "8d06d82970f2c591d880a95476efbcf0"),
	FANMADE("The Long Haired Dude: Encounter of the 18-th Kind", "86ea17b9fc2f3e537a7e40863d352c29"),
	FANMADE("The Lost Planet (v0.9)", "590dffcbd932a9fbe554be13b769cac0"),
	FANMADE("The Lost Planet (v1.0)", "58564df8b6394612dd4b6f5c0fd68d44"),
	FANMADE("The New Adventure of Roger Wilco (v1.00)", "e5f0a7cb8d49f66b89114951888ca688"),
	FANMADE("The Ruby Cast (v0.02)", "ed138e461bb1516e097007e017ab62df"),
	FANMADE("The Shadow Plan", "c02cd10267e721f4e836b1431f504a0a"),
	FANMADE("Time Quest (Demo v0.1)", "12e1a6f03ea4b8c5531acd0400b4ed8d"),
	FANMADE("Time Quest (Demo v0.2)", "7b710608abc99e0861ac59b967bf3f6d"),
	FANMADE("Tonight The Shrieking Corpses Bleed (Demo v0.11)", "bcc57a7c8d563fa0c333107ae1c0a6e6"),
	FANMADE("Tonight The Shrieking Corpses Bleed (v1.01)", "36b38f621b38e8d104aa0807302dc8c9"),
	FANMADE("Turks' Quest - Heir to the Planet", "3d19254b737c8b218e5bc4580542b79a"),
	FANMADE("URI Quest (v0.173 Feb 27)", "3986eefcf546dafc45f920ae91a697c3"),
	FANMADE("URI Quest (v0.173 Jan 29)", "494150940d34130605a4f2e67ee40b12"),
	FANMADE("Voodoo Girl - Queen of the Darned (v1.2 2002 Jan 1)", "ae95f0c77d9a97b61420fd192348b937"),
	FANMADE("Voodoo Girl - Queen of the Darned (v1.2 2002 Mar 29)", "11d0417b7b886f963d0b36789dac4c8f"),
	FANMADE("Wizaro (v0.1)", "abeec1eda6eaf8dbc52443ea97ff140c"),

	{ AD_TABLE_END_MARKER, 0, 0, 0, 0 }
};

/**
 * The fallback game descriptor used by the AGI engine's fallbackDetector.
 * Contents of this struct are to be overwritten by the fallbackDetector.
 */
static AGIGameDescription g_fallbackDesc = {
	{
		"",
		"",
		AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
		Common::UNK_LANG,
		Common::kPlatformPC,
		Common::ADGF_NO_FLAGS
	},
	GID_FANMADE,
	GType_V2,
	GF_FANMADE,
	0x2917,
};

static const Common::ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Agi::gameDescriptions,
	// Size of that superset structure
	sizeof(Agi::AGIGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	agiGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"agi",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0
};

} // End of namespace Agi

using namespace Agi;

class AgiMetaEngine : public Common::AdvancedMetaEngine {
	mutable Common::String	_gameid;
	mutable Common::String	_extra;

public:
	AgiMetaEngine() : Common::AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "AGI preAGI + v2 + v3 Engine";
	}
	virtual const char *getCopyright() const {
		return "Sierra AGI Engine (C) Sierra On-Line Software";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const Common::ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;

	const Common::ADGameDescription *fallbackDetect(const Common::FSList &fslist) const;
};

bool AgiMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate);
}

bool AgiBase::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}


bool AgiMetaEngine::createInstance(OSystem *syst, Engine **engine, const Common::ADGameDescription *desc) const {
	const Agi::AGIGameDescription *gd = (const Agi::AGIGameDescription *)desc;
	bool res = true;

	switch (gd->gameType) {
	case Agi::GType_PreAGI:
		*engine = new Agi::PreAgiEngine(syst, gd);
		break;
	case Agi::GType_V2:
	case Agi::GType_V3:
		*engine = new Agi::AgiEngine(syst, gd);
		break;
	default:
		res = false;
		error("AGI engine: unknown gameType");
	}

	return res;
}

SaveStateList AgiMetaEngine::listSaves(const char *target) const {
	const uint32 AGIflag = MKID_BE('AGI:');
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringList filenames;
	char saveDesc[31];
	Common::String pattern = target;
	pattern += ".???";

	filenames = saveFileMan->listSavefiles(pattern.c_str());
	sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	for (Common::StringList::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				uint32 type = in->readUint32BE();
				if (type == AGIflag)
					in->read(saveDesc, 31);
				saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
				delete in;
			}
		}
	}

	return saveList;
}

int AgiMetaEngine::getMaximumSaveSlot() const { return 999; }

void AgiMetaEngine::removeSaveState(const char *target, int slot) const {
	char fileName[MAX_PATH];
	sprintf(fileName, "%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

SaveStateDescriptor AgiMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	const uint32 AGIflag = MKID_BE('AGI:');
	char fileName[MAX_PATH];
	sprintf(fileName, "%s.%03d", target, slot);

	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fileName);

	if (in) {
		if (in->readUint32BE() != AGIflag) {
			delete in;
			return SaveStateDescriptor();
		}

		char name[32];
		in->read(name, 31);

		SaveStateDescriptor desc(slot, name);

		desc.setDeletableFlag(true);
		desc.setWriteProtectedFlag(false);

		char saveVersion = in->readByte();
		if (saveVersion >= 4) {
			Graphics::Surface *thumbnail = new Graphics::Surface();
			assert(thumbnail);
			if (!Graphics::loadThumbnail(*in, *thumbnail)) {
				delete thumbnail;
				thumbnail = 0;
			}

			desc.setThumbnail(thumbnail);

			uint32 saveDate = in->readUint32BE();
			uint16 saveTime = in->readUint16BE();

			int day = (saveDate >> 24) & 0xFF;
			int month = (saveDate >> 16) & 0xFF;
			int year = saveDate & 0xFFFF;

			desc.setSaveDate(year, month, day);

			int hour = (saveTime >> 8) & 0xFF;
			int minutes = saveTime & 0xFF;

			desc.setSaveTime(hour, minutes);

			// TODO: played time
		}


		delete in;

		return desc;
	}

	return SaveStateDescriptor();
}

const Common::ADGameDescription *AgiMetaEngine::fallbackDetect(const Common::FSList &fslist) const {
	typedef Common::HashMap<Common::String, int32> IntMap;
	IntMap allFiles;
	bool matchedUsingFilenames = false;
	bool matchedUsingWag = false;
	int wagFileCount = 0;
	WagFileParser wagFileParser;
	Common::FSNode wagFileNode;
	Common::String description;

	// // Set the defaults for gameid and extra
	_gameid = "agi-fanmade";
	_extra.clear();

	// Set the default values for the fallback descriptor's ADGameDescription part.
	g_fallbackDesc.desc.language = Common::UNK_LANG;
	g_fallbackDesc.desc.platform = Common::kPlatformPC;
	g_fallbackDesc.desc.flags = Common::ADGF_NO_FLAGS;

	// Set default values for the fallback descriptor's AGIGameDescription part.
	g_fallbackDesc.gameID = GID_FANMADE;
	g_fallbackDesc.features = GF_FANMADE;
	g_fallbackDesc.version = 0x2917;

	// First grab all filenames and at the same time count the number of *.wag files
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory()) continue;
		Common::String filename = file->getName();
		filename.toLowercase();
		allFiles[filename] = true; // Save the filename in a hash table

		if (filename.hasSuffix(".wag")) {
			// Save latest found *.wag file's path (Can be used to open the file, the name can't)
			wagFileNode = *file;
			wagFileCount++; // Count found *.wag files
		}
	}

	if (allFiles.contains("logdir") && allFiles.contains("object") &&
		allFiles.contains("picdir") && allFiles.contains("snddir") &&
		allFiles.contains("viewdir") && allFiles.contains("vol.0") &&
		allFiles.contains("words.tok")) { // Check for v2

		// The default AGI interpreter version 0x2917 is okay for v2 games
		// so we don't have to change it here.
		matchedUsingFilenames = true;

		// Check for AGIPAL by checking for existence of any of the files "pal.100" - "pal.109"
		bool agipal = false;
		char agipalFile[] = "pal.xxx";
		for (uint i = 100; i <= 109; i++) {
			sprintf(agipalFile, "pal.%d", i);
			if (allFiles.contains(agipalFile)) {
				agipal = true; // We found a file "pal.x" where 100 <= x <= 109 so it's AGIPAL
				break;
			}
		}

		if (agipal) { // Check if it is AGIPAL
			description = "Unknown v2 AGIPAL Game";
			g_fallbackDesc.features |= GF_AGIPAL; // Add AGIPAL feature flag
		} else { // Not AGIPAL so just plain v2
			description = "Unknown v2 Game";
		}
	} else { // Try v3
		char name[8];

		for (IntMap::const_iterator f = allFiles.begin(); f != allFiles.end(); ++f) {
			if (f->_key.hasSuffix("vol.0")) {
				memset(name, 0, 8);
				strncpy(name, f->_key.c_str(), MIN((uint)8, f->_key.size() > 5 ? f->_key.size() - 5 : f->_key.size()));

				if (allFiles.contains("object") && allFiles.contains("words.tok") &&
					allFiles.contains(Common::String(name) + "dir")) {
					matchedUsingFilenames = true;
					description = "Unknown v3 Game";
					g_fallbackDesc.version = 0x3149; // Set the default AGI version for an AGI v3 game
					break;
				}
			}
		}
	}

	// WinAGI produces *.wag file with interpreter version, game name and other parameters.
	// If there's exactly one *.wag file and it parses successfully then we'll use its information.
	if (wagFileCount == 1 && wagFileParser.parse(wagFileNode)) {
		matchedUsingWag = true;

		const WagProperty *wagAgiVer = wagFileParser.getProperty(WagProperty::PC_INTVERSION);
		const WagProperty *wagGameID = wagFileParser.getProperty(WagProperty::PC_GAMEID);
		const WagProperty *wagGameDesc = wagFileParser.getProperty(WagProperty::PC_GAMEDESC);
		const WagProperty *wagGameVer = wagFileParser.getProperty(WagProperty::PC_GAMEVERSION);
		const WagProperty *wagGameLastEdit = wagFileParser.getProperty(WagProperty::PC_GAMELAST);

		// If there is an AGI version number in the *.wag file then let's use it
		if (wagAgiVer != NULL && wagFileParser.checkAgiVersionProperty(*wagAgiVer)) {
			// TODO/FIXME: Check that version number is something we support before trying to use it.
			//     If the version number is unsupported then it'll get switched to 0x2917 later.
			//     But there's the possibility that file based detection has detected something else
			//     than a v2 AGI game. So there's a possibility for conflicting information.
			g_fallbackDesc.version = wagFileParser.convertToAgiVersionNumber(*wagAgiVer);
		}

		// Set gameid according to *.wag file information if it's present and it doesn't contain whitespace.
		if (wagGameID != NULL && !Common::String(wagGameID->getData()).contains(" ")) {
			_gameid = wagGameID->getData();
			debug(3, "Agi::fallbackDetector: Using game id (%s) from WAG file", _gameid.c_str());
		}

		// Set game description and extra according to *.wag file information if they're present
		if (wagGameDesc != NULL) {
			description = wagGameDesc->getData();
			debug(3, "Agi::fallbackDetector: Game description (%s) from WAG file", wagGameDesc->getData());

			// If there's game version in the *.wag file, set extra to it
			if (wagGameVer != NULL) {
				_extra = wagGameVer->getData();
				debug(3, "Agi::fallbackDetector: Game version (%s) from WAG file", wagGameVer->getData());
			}

			// If there's game last edit date in the *.wag file, add it to extra
			if (wagGameLastEdit != NULL) {
				if (!_extra.empty() ) _extra += " ";
				_extra += wagGameLastEdit->getData();
				debug(3, "Agi::fallbackDetector: Game's last edit date (%s) from WAG file", wagGameLastEdit->getData());
			}
		}
	} else if (wagFileCount > 1) { // More than one *.wag file, confusing! So let's not use them.
		warning("More than one (%d) *.wag files found. WAG files ignored", wagFileCount);
	}

	// Check that the AGI interpreter version is a supported one
	if (!(g_fallbackDesc.version >= 0x2000 && g_fallbackDesc.version < 0x4000)) {
		warning("Unsupported AGI interpreter version 0x%x in AGI's fallback detection. Using default 0x2917", g_fallbackDesc.version);
		g_fallbackDesc.version = 0x2917;
	}

	// Set game type (v2 or v3) according to the AGI interpreter version number
	if (g_fallbackDesc.version >= 0x2000 && g_fallbackDesc.version < 0x3000)
		g_fallbackDesc.gameType = GType_V2;
	else if (g_fallbackDesc.version >= 0x3000 && g_fallbackDesc.version < 0x4000)
		g_fallbackDesc.gameType = GType_V3;

	// Check if we found a match with any of the fallback methods
	if (matchedUsingWag || matchedUsingFilenames) {
		_extra = description + (!_extra.empty() ? " " : "") + _extra; // Let's combine the description and extra

		// Override the gameid & extra values in g_fallbackDesc.desc. This only works
		// until the fallback detector is called again, and while the MetaEngine instance
		// is alive (as else the string storage is modified/deleted).
		g_fallbackDesc.desc.gameid = _gameid.c_str();
		g_fallbackDesc.desc.extra = _extra.c_str();

		printf("Your game version has been detected using fallback matching as a\n");
		printf("variant of %s (%s).\n", g_fallbackDesc.desc.gameid, g_fallbackDesc.desc.extra);
		printf("If this is an original and unmodified version or new made Fanmade game,\n");
		printf("please report any, information previously printed by ScummVM to the team.\n");

		return (const Common::ADGameDescription *)&g_fallbackDesc;
	}

	return 0;
}

#if PLUGIN_ENABLED_DYNAMIC(AGI)
	REGISTER_PLUGIN_DYNAMIC(AGI, PLUGIN_TYPE_ENGINE, AgiMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(AGI, PLUGIN_TYPE_ENGINE, AgiMetaEngine);
#endif

namespace Agi {

Common::Error AgiBase::loadGameState(int slot) {
	static char saveLoadSlot[12];
	sprintf(saveLoadSlot, "%s.%.3d", _targetName.c_str(), slot);
	loadGame(saveLoadSlot);
	return Common::kNoError;	// TODO: return success/failure
}

Common::Error AgiBase::saveGameState(int slot, const char *desc) {
	static char saveLoadSlot[12];
	sprintf(saveLoadSlot, "%s.%.3d", _targetName.c_str(), slot);
	saveGame(saveLoadSlot, desc);
	return Common::kNoError;	// TODO: return success/failure
}

bool AgiBase::canLoadGameStateCurrently() {
	return (!(getGameType() == GType_PreAGI) && getflag(fMenusWork));
}

bool AgiBase::canSaveGameStateCurrently() {
	return (!(getGameType() == GType_PreAGI) && getflag(fMenusWork));
}

} // End of namespace Agi
