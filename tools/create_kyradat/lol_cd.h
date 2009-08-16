const ExtractEntry lolCDFile1E[] = {
	{ kLolHistory, 0x1FAA0, 0x1FC71 },
	{ -1, 0, 0 }
};

const ExtractEntry lolCDFile2[] = {
	{ kLolCharacterDefs, 0x00029D60, 0x0002A1F2 },
	{ kLolIngameSfxFiles, 0x0002A330, 0x0002AC22 },
	{ kLolIngameSfxIndex, 0x0002AC22, 0x0002B00A },
	{ kLolMusicTrackMap, 0x0002A240, 0x0002A330 },
	{ kLolGMSfxIndex, 0x0002B010, 0x0002B10A },
	{ kLolMT32SfxIndex, 0x0002B110, 0x0002B20A },
	//{ kLolADLSfxIndex, 0x0002B210, 0x0002B30A },
	{ kLolSpellProperties, 0x0002B5E0, 0x0002B6F8 },
	{ kLolGameShapeMap, 0x0002B35C, 0x0002B470 },
	{ kLolSceneItemOffs, 0x0003200F, 0x00032017 },
	{ kLolCharInvIndex, 0x0002B470, 0x0002B47A },
	{ kLolCharInvDefs, 0x0002B47A, 0x0002B4D2 },
	{ kLolCharDefsMan, 0x0002B4D2, 0x0002B4E4 },
	{ kLolCharDefsWoman, 0x0002B4E4, 0x0002B4F6 },
	{ kLolCharDefsKieran, 0x0002B4F6, 0x0002B508 },
	//{ kLolCharDefsUnk, 0x0002B508, 0x0002B51A },
	{ kLolCharDefsAkshel, 0x0002B51A, 0x0002B52C },
	{ kLolExpRequirements, 0x0002B830, 0x0002B85C },
	{ kLolMonsterModifiers, 0x0002B85C, 0x0002B874 },
	{ kLolMonsterLevelOffsets, 0x00031FDF, 0x00031FFF },
	{ kLolMonsterDirFlags, 0x00031FFF, 0x0003200F },
	{ kLolMonsterScaleY, 0x00031BC0, 0x00031BDE },
	{ kLolMonsterScaleX, 0x00031BDE, 0x00031BFE },
	{ kLolMonsterScaleWH, 0x000285C0, 0x00028642 },
	{ kLolFlyingItemShp, 0x00028642, 0x000286C4 },
	{ kLolInventoryDesc, 0x00032706, 0x0003271C },

	{ kLolLevelShpList, 0x00032826, 0x000328A5 },
	{ kLolLevelDatList, 0x000328A5, 0x000329A4 },
	{ kLolCompassDefs, 0x000286C4, 0x000287C4 },
	{ kLolItemPrices, 0x0002F8C0, 0x0002F91C },
	{ kLolStashSetup, 0x000324A5, 0x000324B1 },

	{ kLolDscUnk1, 0x00032017, 0x0003203B },
	{ kLolDscShapeIndex1, 0x0003203B, 0x0003205F },
	{ kLolDscShapeIndex2, 0x0003205F, 0x00032069 },
	{ kLolDscScaleWidthData, 0x00032069, 0x000320B1 },
	{ kLolDscScaleHeightData, 0x000320B1, 0x000320F9 },
	{ kLolDscX, 0x000320F9, 0x00032141 },
	{ kLolDscY, 0x00032141, 0x00032165 },
	{ kLolDscTileIndex, 0x00032165, 0x00032177 },
	{ kLolDscUnk2, 0x00032177, 0x0003218E },
	{ kLolDscDoorShapeIndex, 0x0003218E, 0x000321A5 },
	{ kLolDscDimData1, 0x00031C03, 0x00031D47 },
	{ kLolDscDimData2, 0x00031D47, 0x00031FD0 },
	{ kLolDscBlockMap, 0x00031B64, 0x00031B70 },
	{ kLolDscDimMap, 0x00031B70, 0x00031B82 },
	{ kLolDscDoorScale, 0x00031B82, 0x00031B92 },
	{ kLolDscShapeOvlIndex, 0x00031B92, 0x00031BBA },
	{ kLolDscDoor4, 0x00031BBA, 0x00031BC2 },
	{ kLolDscBlockIndex, 0x00033B53, 0x00033B9B },
	{ kLolDscDoor1, 0x0002B550, 0x0002B5D0 },
	{ kLolDscDoorX, 0x00027CC0, 0x00028140 },
	{ kLolDscDoorY, 0x00028140, 0x000285C0 },

	{ kLolScrollXTop, 0x00033D40, 0x00033D4A },
	{ kLolScrollYTop, 0x00033D4A, 0x00033D54 },
	{ kLolScrollXBottom, 0x00033D54, 0x00033D5E },
	{ kLolScrollYBottom, 0x00033D5E, 0x00033D68 },

	{ kLolButtonDefs, 0x00028E80, 0x000296AA },
	{ kLolButtonList1, 0x000296B0, 0x00029700 },
	{ kLolButtonList2, 0x00029700, 0x0002971E },
	{ kLolButtonList3, 0x00029720, 0x00029724 },
	{ kLolButtonList4, 0x00029730, 0x0002974E },
	{ kLolButtonList5, 0x00029750, 0x00029770 },
	{ kLolButtonList6, 0x00029770, 0x0002978C },
	{ kLolButtonList7, 0x00029790, 0x00029796 },
	{ kLolButtonList8, 0x000297A0, 0x000297A4 },

	{ kLolLegendData, 0x000321F0, 0x0003222C },
	{ kLolMapCursorOvl, 0x0003222C, 0x00032245 },
	{ kLolMapStringId, 0x000287D0, 0x0002880C },
	//{ kLolMapPal, 0x0001D9C0, 0x0001DBC0 },

	{ kLolSpellbookAnim, 0x00032D94, 0x00032DCC },
	{ kLolSpellbookCoords, 0x00032DCC, 0x00032DE4 },
	{ kLolHealShapeFrames, 0x000297D0, 0x00029820 },
	{ kLolLightningDefs, 0x00032740, 0x00032754 },
	{ kLolFireballCoords, 0x0001D9C0, 0x0001DBC0 },

	{ -1, 0, 0 }
};

const Game lolGames[] = {
	{ kLol, EN_ANY, kLolCD, "9d1778314de80598c0b0d032e2a1a1cf", lolCDFile1E },
	{ kLol, EN_ANY, kLolCD, "263998ec600afca1cc7b935c473df670", lolCDFile2 },
	GAME_DUMMY_ENTRY
};
