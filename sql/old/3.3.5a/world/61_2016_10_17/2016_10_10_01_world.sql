-- 
UPDATE `creature_template` SET `ScriptName`="" WHERE `entry` IN (27017);
DELETE FROM `creature_text` WHERE `entry`=27017;
INSERT INTO `creature_text` (`entry`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `comment`, `broadcasttextid`) VALUES
(27017,0,0,'Initiating energy collection.',12,0,100,0,0,0,'War Golem',26153),
(27017,1,0,'Energy collection complete.',12,0,100,0,0,0,'War Golem',26154);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (2701700) AND `source_type`=9;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (27017) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(27017, 0, 0, 0, 38, 0, 100, 512, 0, 1, 4000, 4000, 80, 2701700, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Depleted War Golem - On Data Set 0 1 - Actionlist"),
(2701700, 9, 0, 0, 0, 0, 100, 512, 0, 0, 0, 0, 69, 0, 0, 0, 0, 0, 0, 19, 26407, 25, 1, 0, 0, 0, 0, "Depleted War Golem - Actionlist - Move to pos"),
(2701700, 9, 1, 0, 0, 0, 100, 512, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Depleted War Golem - Actionlist - Say text 1"),
(2701700, 9, 2, 0, 0, 0, 100, 512, 3000, 3000, 0, 0, 11, 47799, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Depleted War Golem - Actionlist - cast"),
(2701700, 9, 3, 0, 0, 0, 100, 512, 5000, 5000, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Depleted War Golem - Actionlist - Say text 2"),
(2701700, 9, 4, 0, 0, 0, 100, 512, 0, 0, 0, 0, 11, 47797, 0, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, "Depleted War Golem - Actionlist - Cast 'War Golem Charge Credit'"),
(2701700, 9, 5, 0, 0, 0, 100, 512, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Depleted War Golem - Actionlist - evade");
