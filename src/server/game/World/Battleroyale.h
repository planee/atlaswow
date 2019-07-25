#pragma once
#include "Group.h"
#include "Player.h"
#include "Map.h"
#include "Creature.h"
#include <vector>
#include "DatabaseEnv.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "World.h"
#include "MapManager.h"
#include "GameObject.h"
#include "WorldSession.h"
#include "Chat.h"

using namespace std;



class Battleroyale {

public:

    struct Spawn {
        float x;
        float y;
        float z;
        uint16 mapId;
    };


    struct GroupMember {
        ObjectGuid guid;
        bool isAlive;
    };


    struct Game {
        string name;
        vector<ObjectGuid> playersGuid;
        vector<Spawn> spawnsPlayers;
        vector<Spawn> spawnsChest;
        map<uint32/*GroupGUID*/, vector<Battleroyale::GroupMember>> GROUPS;
        bool isStarted = false;
        bool firstRound = false;
        uint8 minPlayer;
        Spawn center;
        uint8 category;
        uint64 diff = 0;
        uint64 diffAnnoncement = 0;
        uint32 firstEventTimer;
        uint32 nextEventTimer;
        uint8 iteration = 0;
        uint32 radius;
        string icon;
        uint8 MAX_ITERATION;
        GameObject* flag;
        vector<GameObject*> tracers;
        map<uint32, uint32> mPlayersKills;
    };


    static map<uint32, Game> m_games;
    static map<uint32 /*accountId*/, uint32 /*points*/> m_playersPoints;

    static void prepareGames();
    static void reloadGameId(uint32 gameId);
    static void addPlayer(Player* player, uint32 gameId);
    static void removePlayer(Player* player, bool removePlayer);
    static void removePlayer(Player* player);
    static void removePlayerFromGroup(Player* player, bool removePlayer);

    static void start(uint32 gameId);

    static uint32 getRating(Player* player);
    static void update(uint64 diff);
    static void updatePlayer(Player* player, uint64 p_time);
    static void onPlayerDie(Player* killed, Player* killer);
    static bool gameIsStarted(uint32 gameId);
    static uint32 checkIfPlayerIsInBattleRoyale(Player* player);


};
