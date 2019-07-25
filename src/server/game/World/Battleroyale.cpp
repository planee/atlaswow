#include "Battleroyale.h"
#include "Log.h"
#include <iostream>
#include <iomanip>
#include "SpellHistory.h"
#include "Cell.h"
#include "GroupMgr.h"

map<uint32, Battleroyale::Game> Battleroyale::m_games = {};

enum CATEGORIES {
    SOLO = 0,
    GROUP_2 = 2,
    GROUP_3 = 3,
    GROUP_4 = 4,
    GROUP_5 = 5,
};

const int ITERATION_LEVEL = 4;

uint32 GetGroupMemberAlive(uint32 groupGuid, uint32 gameId) {

    uint32 count = 0;

    for (auto IJ = Battleroyale::m_games[gameId].GROUPS.begin(); IJ != Battleroyale::m_games[gameId].GROUPS.end(); ++IJ) {
        if (IJ->first == uint32(groupGuid)) {
            for (auto member : IJ->second) {
                if (member.isAlive == true) {
                    count++;
                }
            }
        }
    }
    return count;
}


bool someoneIsDisconnectedInGroup(Group* group) {

    uint32 memberCount = group->GetMembersCount();

    uint32 count = 0;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* player = itr->GetSource();
        if (player)
            count++;
    }



    return count != memberCount;
}

void removeLootOnDeath(Player* player) {
    for (int i = 400400; i < 400450; i++)
    {
        if (player->HasItemCount(i))
            player->DestroyItemCount(i, 100, true);
    }
}


bool isAlive(Player* player, uint32 gameId) {

    Group* group = player->GetGroup();

    bool isAlive = false;


    for (auto IJ = Battleroyale::m_games[gameId].GROUPS.begin(); IJ != Battleroyale::m_games[gameId].GROUPS.end(); ++IJ)
        if (IJ->first == uint32(group->GetGUID()))
            for (auto member : IJ->second)
                if (member.guid == uint32(player->GetGUID())) {
                    isAlive = member.isAlive;
                    break;
                }

    return isAlive;
}


float calculateRadius(int radius, int iteration, int MAX_ITERATION) {
    return radius - (iteration * (radius / MAX_ITERATION));
}

void giveRewardByRating(Player* player) {

    QueryResult result = WorldDatabase.PQuery("SELECT * FROM battleroyale_rewards");
    uint32 rating = Battleroyale::getRating(player);

    do
    {
        Field* fields = result->Fetch();
        uint32 rank = fields[0].GetUInt32();
        uint32 minRating = fields[1].GetUInt32();
        uint32 item1 = fields[2].GetUInt32();
        uint32 spellId = fields[3].GetUInt32();
        uint32 titleId = fields[4].GetUInt32();
        uint32 achievementId = fields[5].GetUInt32();

        CharTitlesEntry const* titleInfo = sCharTitlesStore.LookupEntry(titleId);
        AchievementEntry const* achievementEntry = sAchievementStore.LookupEntry(achievementId);
        if (!titleInfo)
            return;

        std::string msg = "Congratulation! You as just reached a new rank!";


        if (rating >= minRating && !player->HasTitle(titleInfo)) {
            if(item1 > 0)
            player->AddItem(item1, 1);
            if(spellId > 0)
            player->LearnSpell(spellId, false);
            if (rank >= 3)
                player->AddItem(505051, rank);


            player->SetTitle(titleInfo);
            player->CompletedAchievement(achievementEntry);
            ChatHandler(player->GetSession()).SendGlobalSysMessage(msg.c_str());
            break;
        }

    } while (result->NextRow());
}


void rewardPlayer(Player* player, uint32 position, bool isWinner, uint32 gameId) {

    TC_LOG_ERROR("ERROR", "Call rewards players");

    if (!player)
        return;

    uint32 killCount = 0;
    uint32 top1 = 0;
    uint32 totalDeath = 0;

    if (Battleroyale::m_games[gameId].mPlayersKills.count(player->GetGUID()) > 0)
        killCount = Battleroyale::m_games[gameId].mPlayersKills[player->GetGUID()];



    if (position == 1)
        top1 = 1;
    else
        totalDeath = 1;


    TC_LOG_ERROR("ERROR", "position %u, totalKill %u, top1 %u", position, killCount, top1);


    uint32 category = Battleroyale::m_games[gameId].category;

    CharacterDatabase.DirectPExecute("INSERT INTO battleroyale_stats (guid, kills, mapId, position) VALUES ('%u', '%u', '%u', '%u')", player->GetSession()->GetAccountId(), killCount, gameId, position + 1);
    CharacterDatabase.DirectPExecute("UPDATE battleroyale_players_stats SET totalKills = totalKills + %u, totalTop1 = totalTop1 + %u, totalDeath = totalDeath + %u WHERE guid = %u AND type = %u", killCount, top1, totalDeath, player->GetSession()->GetAccountId(), category);

    uint32 rating = Battleroyale::getRating(player);

    std::string msg = "Your rating right now : " + std::to_string(rating);
    TC_LOG_ERROR("CALL", "rating %u", rating);
    ChatHandler(player->GetSession()).SendSysMessage(msg.c_str());

    giveRewardByRating(player);
}


void rewardAllPlayersInGroup(Group* grp, uint32 position, uint32 gameId, bool remove) {

    for (auto IJ = Battleroyale::m_games[gameId].GROUPS.begin(); IJ != Battleroyale::m_games[gameId].GROUPS.end(); ++IJ)
        if (IJ->first == uint32(grp->GetGUID())) {
            for (auto member : IJ->second) {
                Player* player = ObjectAccessor::FindPlayer(member.guid);
                if (player) {
                    rewardPlayer(player, position, false, gameId);
                    if (isAlive(player, gameId) && remove) {
                        player->ResurrectPlayer(100.f);
                        player->TeleportTo(0, 4289.833984f, -2768.049805f, 6.868990f, 3.639589f); // TP TO LOBBY
                        removeLootOnDeath(player);
                        Battleroyale::m_games[gameId].playersGuid.erase(std::remove(Battleroyale::m_games[gameId].playersGuid.begin(), Battleroyale::m_games[gameId].playersGuid.end(), player->GetGUID()), Battleroyale::m_games[gameId].playersGuid.end());
                    }
                }
            }
            break;
        }

}


Player* getTheLastPlayer(uint32 gameId) {
    Player* player = nullptr;


    ObjectGuid guid = Battleroyale::m_games[gameId].playersGuid[0];

    player = ObjectAccessor::FindPlayer(guid);

    if (player)
        return player;

    return player;
}

void checkReward(Player* player, uint32 gameId) {
    int playersCount = Battleroyale::m_games[gameId].playersGuid.size();
    int groupCount = Battleroyale::m_games[gameId].GROUPS.size();
    std::string mapName = Battleroyale::m_games[gameId].name;

    if (playersCount == 1 && Battleroyale::m_games[gameId].isStarted == true && Battleroyale::m_games[gameId].category == SOLO) {
            Player* lastPlayer = getTheLastPlayer(gameId);
            Battleroyale::removePlayer(lastPlayer, true);
            std::string msg = lastPlayer->GetName() + " just made top 1 on " + mapName;
            ChatHandler(lastPlayer->GetSession()).SendGlobalSysMessage(msg.c_str());
            Battleroyale::reloadGameId(gameId);
    }


    if (Battleroyale::m_games[gameId].isStarted == true && Battleroyale::m_games[gameId].category > SOLO && Battleroyale::m_games[gameId].GROUPS.size() <= 1) {
        Player* lastPlayer = getTheLastPlayer(gameId);
        rewardAllPlayersInGroup(lastPlayer->GetGroup(), groupCount, gameId, true);
        std::string msg = lastPlayer->GetName() + " and his group, just made top 1 on " + mapName;
        ChatHandler(lastPlayer->GetSession()).SendGlobalSysMessage(msg.c_str());
        Battleroyale::reloadGameId(gameId);
    }
}



std::vector<Battleroyale::Spawn> loadSpawnsPlayersByGameId(uint32 gameId) {
    std::vector<Battleroyale::Spawn> spawns = {};

    QueryResult result = WorldDatabase.PQuery("SELECT * FROM battleroyale_spawns WHERE id = %u", gameId);

    if (!result)
        return spawns;

    do
    {
        Battleroyale::Spawn spawn;
        Field* fields = result->Fetch();
        spawn.x = fields[1].GetFloat();
        spawn.y = fields[2].GetFloat();
        spawn.z = fields[3].GetFloat();
        spawn.mapId = fields[4].GetUInt32();

        spawns.push_back(spawn);

    } while (result->NextRow());

    return spawns;
}

std::vector<Battleroyale::Spawn> loadSpawnChestsByGameId(uint32 gameId) {
    std::vector<Battleroyale::Spawn> spawns = {};

    QueryResult result = WorldDatabase.PQuery("SELECT * FROM battleroyale_chests_spawns WHERE id = %u", gameId);

    if (!result)
        return spawns;

    do
    {
        Battleroyale::Spawn spawn;
        Field* fields = result->Fetch();
        spawn.x = fields[1].GetFloat();
        spawn.y = fields[2].GetFloat();
        spawn.z = fields[3].GetFloat();
        spawn.mapId = fields[4].GetUInt32();

        spawns.push_back(spawn);

    } while (result->NextRow());

    return spawns;
}

GameObject* summonGameObject(uint32 mapId, uint32 entry, const Position &pos, uint32 respawnTime) {


    Map* map = sMapMgr->CreateBaseMap(mapId);
    if (!map)
        return nullptr;

    // Create gameobject
    GameObject* go = new GameObject;
    if (!go->Create(map->GenerateLowGuid<HighGuid::GameObject>(), entry, map, PHASEMASK_NORMAL, pos, QuaternionData(), 255, GO_STATE_READY))
    {
        delete go;
        return nullptr;
    }

    // Add to world
    map->AddToMap(go);
    go->setActive(true);
    go->SetFarVisible(true);

    return go;
}


void spawnChestAtStart(uint8 gameId) {
    std::vector<Battleroyale::Spawn> spawns = Battleroyale::m_games[gameId].spawnsChest;

    for (auto const& position : spawns) {
        Position pos;
        pos.m_positionX = position.x;
        pos.m_positionY = position.y;
        pos.m_positionZ = position.z;
        summonGameObject(position.mapId, 500500, pos, 1200);
    }

}


void Battleroyale::reloadGameId(uint32 gameId) {

    QueryResult result = WorldDatabase.PQuery("SELECT * FROM battleroyale_areas WHERE id = %u ORDER BY size", gameId);

    Battleroyale::Game game;


    if (!result)
        return;

    do
    {
        std::vector<ObjectGuid> playersGuid;
        Field* fields = result->Fetch();
        game.name = fields[1].GetString();
        game.playersGuid = playersGuid;
        game.spawnsPlayers = loadSpawnsPlayersByGameId(gameId);
        game.spawnsChest = loadSpawnChestsByGameId(gameId);
        game.minPlayer = fields[3].GetInt8();
        game.category = fields[4].GetInt8();
        game.firstEventTimer = fields[5].GetUInt32();
        game.nextEventTimer = fields[6].GetUInt32();
        game.radius = fields[7].GetUInt32();
        game.icon = fields[8].GetString();
        Battleroyale::m_games[gameId] = game;

    } while (result->NextRow());
}


void Battleroyale::prepareGames()
{
    QueryResult result = WorldDatabase.PQuery("SELECT * FROM battleroyale_areas ORDER BY size ASC");

    if (!result)
        return;

    do
    {
        std::vector<ObjectGuid> playersGuid;
        Field* fields = result->Fetch();
        uint32 id = fields[0].GetUInt32();
        Battleroyale::m_games[id].name = fields[1].GetString();
        Battleroyale::m_games[id].playersGuid = playersGuid;
        Battleroyale::m_games[id].spawnsPlayers = loadSpawnsPlayersByGameId(id);
        Battleroyale::m_games[id].spawnsChest = loadSpawnChestsByGameId(id);
        Battleroyale::m_games[id].minPlayer = fields[3].GetInt8();
        Battleroyale::m_games[id].category = fields[4].GetInt8();
        Battleroyale::m_games[id].firstEventTimer = fields[5].GetUInt32();
        Battleroyale::m_games[id].nextEventTimer = fields[6].GetUInt32();
        Battleroyale::m_games[id].radius = fields[7].GetUInt32();
        Battleroyale::m_games[id].icon = fields[8].GetString();
        Battleroyale::m_games[id].MAX_ITERATION = fields[9].GetUInt8();

    } while (result->NextRow());
}


void sendMessageToEveryone(uint32 gameId, const char* str) {
    for (auto const& guid : Battleroyale::m_games[gameId].playersGuid) {
        Player* player = ObjectAccessor::FindPlayer(guid);

        if (!player)
            return;
    }
}

void Battleroyale::addPlayer(Player* player, uint32 gameId)
{

    if (!player)
        return;

    bool gameIsStarted = Battleroyale::m_games[gameId].isStarted;
    uint8 minPlayers = Battleroyale::m_games[gameId].minPlayer;
    uint8 category = Battleroyale::m_games[gameId].category;
    uint8 currentPlayers = 0;

    if (gameIsStarted) {
        ChatHandler chat(player->GetSession());
        chat.SendSysMessage("This battleroyale is already in progress. ");
        return;
    }

    if (Battleroyale::checkIfPlayerIsInBattleRoyale(player) > 0) {
        ChatHandler chat(player->GetSession());
        chat.SendSysMessage("You are already in a battleroyale queue.");
        return;
    }


    if (category > SOLO) {
        if (!player->GetGroup()) {
            ChatHandler chat(player->GetSession());
            chat.SendSysMessage("You need to be in group.");
            return;
        }

        if (player->GetGroup()->GetMembersCount() < Battleroyale::m_games[gameId].category) {
            ChatHandler chat(player->GetSession());
            std::string ss;

            ss = "Not enough players in your group. Min players" + std::to_string(Battleroyale::m_games[gameId].category);
            chat.SendSysMessage(ss.c_str());
            return;
        }

        if(someoneIsDisconnectedInGroup(player->GetGroup())) {
            ChatHandler chat(player->GetSession());
            chat.SendSysMessage("Someone in your group is disconnected!");
            return;
        }

        std::vector<Battleroyale::GroupMember> members;
    
        for (GroupReference* itr = player->GetGroup()->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            if (Player* pl = itr->GetSource()) {
                Battleroyale::GroupMember member;
                member.guid = pl->GetGUID();
                member.isAlive = true;
                members.push_back(member);
                Battleroyale::m_games[gameId].playersGuid.push_back(pl->GetGUID());
                currentPlayers = Battleroyale::m_games[gameId].playersGuid.size();
            }
        }

        player->GetGroup()->ConvertToRaid();
        Battleroyale::m_games[gameId].GROUPS[uint32(player->GetGroup()->GetGUID())] = members;
    }
    else {
        if (player->GetGroup()) {
            ChatHandler chat(player->GetSession());
            chat.SendSysMessage("You can't be in group and joining a solo queue.");
            return;
        }
        else {
            Battleroyale::m_games[gameId].playersGuid.push_back(player->GetGUID());
            currentPlayers = Battleroyale::m_games[gameId].playersGuid.size();
        }
    }


    //std::string msgToEveryone = "You are now in the queue in " + Battleroyale::m_games[gameId].name;
    //sendMessageToEveryone(gameId, msg.c_str());

    if (currentPlayers >= minPlayers) {
        Battleroyale::start(gameId);
    }
    else {
        ChatHandler chat(player->GetSession());
        uint32 count = Battleroyale::m_games[gameId].minPlayer - Battleroyale::m_games[gameId].playersGuid.size();
        std::string msg = "You are now in the queue in " + Battleroyale::m_games[gameId].name + ". Need " + std::to_string(count) + " players before starting the game";
        chat.SendSysMessage(msg.c_str());
    }

}


void Battleroyale::removePlayer(Player* player, bool lastPlayer)
{
    for (auto it = Battleroyale::m_games.begin(); it != Battleroyale::m_games.end(); ++it)
    {
        for (auto const& guid : it->second.playersGuid) {

            if (Player* p = ObjectAccessor::FindPlayer(guid)) {
                if (p->GetGUID() == player->GetGUID()) {
                    uint32 position = it->second.playersGuid.size();
                    p->TeleportTo(0, 4289.833984f, -2768.049805f, 6.868990f, 3.639589f); // TP TO LOBBY
                    removeLootOnDeath(p);
                    rewardPlayer(player, position, false, it->first);

                    if(p->isDead())
                        p->ResurrectPlayer(100.f);

                    it->second.playersGuid.erase(std::remove(it->second.playersGuid.begin(), it->second.playersGuid.end(), player->GetGUID()), it->second.playersGuid.end());

                    if (it->second.category > SOLO) {
                        TC_LOG_ERROR("ERROR", "Someone try to leaving group");
                        for (auto IJ = it->second.GROUPS.begin(); IJ != it->second.GROUPS.end(); ++IJ)
                            for (auto& member : IJ->second) {
                                if (member.guid == uint32(p->GetGUID())) {
                                    member.isAlive = false;
                                    uint32 groupGuid = IJ->first;
                                    if (GetGroupMemberAlive(groupGuid, it->first) == 0) {
                                        TC_LOG_ERROR("ERROR", "Members alive 0");
                                        auto iter = it->second.GROUPS.find(groupGuid);
                                        if (iter != it->second.GROUPS.end())
                                            it->second.GROUPS.erase(iter);
                                    }
                                    break;
                                }
                            }
                        }

                    checkReward(p, it->first);

                    if (lastPlayer)
                        break;

                    for (auto const& guid : Battleroyale::m_games[it->first].playersGuid) {
                        if (Player * p = ObjectAccessor::FindPlayer(guid)) {
                            ChatHandler chat(p->GetSession());
                            std::string msg = player->GetName() + " just died!" + " Players alive : " + std::to_string(Battleroyale::m_games[it->first].playersGuid.size());
                            chat.SendSysMessage(msg.c_str());
                        }
                    }
                    break;
                }
            }
        }
    }
}

void Battleroyale::removePlayer(Player* player)
{
    for (auto it = Battleroyale::m_games.begin(); it != Battleroyale::m_games.end(); ++it)
    {
        for (auto const& guid : it->second.playersGuid) {
            Player* p = ObjectAccessor::FindPlayer(guid);
            if (!p)
                return;

            if (p->GetGUID() == player->GetGUID()) {
                it->second.playersGuid.erase(std::remove(it->second.playersGuid.begin(), it->second.playersGuid.end(), player->GetGUID()), it->second.playersGuid.end());
                ChatHandler(player->GetSession()).SendSysMessage("You are now longer in queue.");
                break;
            }
        }

        if (player->GetGroup()) {
            auto iter = it->second.GROUPS.find(player->GetGroup()->GetGUID());
            if (iter != it->second.GROUPS.end()) {
                it->second.GROUPS.erase(iter);
                for (GroupReference* itr = player->GetGroup()->GetFirstMember(); itr != nullptr; itr = itr->next())
                {
                    if (Player * playerInGroup = itr->GetSource()) {
                        it->second.playersGuid.erase(std::remove(it->second.playersGuid.begin(), it->second.playersGuid.end(), playerInGroup->GetGUID()), it->second.playersGuid.end());
                        ChatHandler(playerInGroup->GetSession()).SendSysMessage("Your group has been removed from the queue.");
                    }
                }
            }
        }
    }
}






void Battleroyale::removePlayerFromGroup(Player* player, bool removePlayer)
{

    if (!player)
        return;

    for (auto it = Battleroyale::m_games.begin(); it != Battleroyale::m_games.end(); ++it)
        for (auto const& guid : it->second.playersGuid)
            if (Player* p = ObjectAccessor::FindPlayer(guid)) 
                if (p->GetGUID() == player->GetGUID()) {
                    Group* grp = p->GetGroup();

                    it->second.playersGuid.erase(std::remove(it->second.playersGuid.begin(), it->second.playersGuid.end(), p->GetGUID()), it->second.playersGuid.end());
                    for (auto IJ = it->second.GROUPS.begin(); IJ != it->second.GROUPS.end(); ++IJ) {
                        if (IJ->first == uint32(grp->GetGUID())) {
                            for (auto &member : IJ->second) {
                                if (member.guid == uint32(p->GetGUID())) {
                                    member.isAlive = false;
                                    break;
                                }
                            }
                        }
                    }
                     

                    uint32 totalMemberAliveInHisGroup = GetGroupMemberAlive(player->GetGroup()->GetGUID(), it->first);
                    uint32 totalGroupAlive = it->second.GROUPS.size();

                    if (totalMemberAliveInHisGroup == 0) {
                        rewardAllPlayersInGroup(player->GetGroup(), totalGroupAlive, it->first, false);

                        auto iter = it->second.GROUPS.find(player->GetGroup()->GetGUID());
                        if (iter != it->second.GROUPS.end())
                            it->second.GROUPS.erase(iter);
                    }

                    checkReward(player, it->first);

                    if(p->isDead())
                        p->ResurrectPlayer(100.f);

                    p->TeleportTo(0, 4289.833984f, -2768.049805f, 6.868990f, 3.639589f); // TP TO LOBBY
                    removeLootOnDeath(p);

                    for (auto const& guid : Battleroyale::m_games[it->first].playersGuid) {
                        if (Player * p = ObjectAccessor::FindPlayer(guid)) {
                            ChatHandler chat(p->GetSession());
                            std::string msg = player->GetName() + " just died!" + " Players alive : " + std::to_string(Battleroyale::m_games[it->first].playersGuid.size());
                            chat.SendSysMessage(msg.c_str());
                        }
                    }

          }
 
}


void removeSpawn(std::vector<Battleroyale::Spawn>&spawns, Battleroyale::Spawn spawn) {
    for (auto iter = spawns.begin(); iter != spawns.end(); ++iter) {
        if (iter->mapId == spawn.mapId && iter->x == spawn.x && iter->y == spawn.y && iter->z == spawn.z) {
            iter = spawns.erase(iter);
            break;
        }
    }
}

void Battleroyale::start(uint32 gameId)
{
    Battleroyale::m_games[gameId].isStarted = true;
    spawnChestAtStart(gameId);
    uint8 category = Battleroyale::m_games[gameId].category;
    std::vector<Battleroyale::Spawn>&spawns = Battleroyale::m_games[gameId].spawnsPlayers;

    uint32 count = 0;
    if (category > SOLO) {
        for (auto it = Battleroyale::m_games.begin(); it != Battleroyale::m_games.end(); ++it) {
            for (auto IJ = it->second.GROUPS.begin(); IJ != it->second.GROUPS.end(); ++IJ) {
                uint32 rand = urand(0, spawns.size() - 1);
                for (auto member : IJ->second) {
                    Player* player = ObjectAccessor::FindPlayer(member.guid);
                    if (player) {
                        player->GetSpellHistory()->ResetAllCooldowns();

                        if(player->isDead())
                            player->ResurrectPlayer(100.f);

                        player->TeleportTo(spawns[rand].mapId, spawns[rand].x, spawns[rand].y, spawns[rand].z, player->GetOrientation());
                    }
                }
                removeSpawn(spawns, spawns[rand]);
            }
        }
        TC_LOG_ERROR("ERROR", "Started : Total groups %u", Battleroyale::m_games[gameId].GROUPS.size());
    }
    else {
        for (auto const& guid : Battleroyale::m_games[gameId].playersGuid) {
            if (Player * p = ObjectAccessor::FindPlayer(guid)) {
                uint32 rand = urand(0, spawns.size() - 1);
                removeSpawn(spawns, spawns[rand]);
                p->TeleportTo(spawns[rand].mapId, spawns[rand].x, spawns[rand].y, spawns[rand].z, p->GetOrientation());
                p->GetSpellHistory()->ResetAllCooldowns();
                count++;
                ChatHandler chat(p->GetSession());
                uint32 timer = Battleroyale::m_games[gameId].firstEventTimer / IN_MILLISECONDS;
                std::string msg = "First event in " + std::to_string(timer) + " sec(s) be prepared!";
                chat.SendSysMessage(msg.c_str());
            }
        }
    }
}


Battleroyale::Spawn getRandomSpawn(std::vector<Battleroyale::Spawn> spawns) {
    Battleroyale::Spawn spawn;
    spawn = spawns[urand(0, spawns.size() - 1)];
    return spawn;
}



void sendCenterToPlayer(Player *player, float x, float y) {

    WorldPacket data(SMSG_GOSSIP_POI, 4 + 4 + 4 + 4 + 4 + 10);  // guess size
    data << uint32(99);
    data << float(x);
    data << float(y);
    data << uint32(7); // Icon
    data << uint32(0); // Importance
    data << "Center";


    player->GetSession()->SendPacket(&data);
}


void spawnCreatureAroundCircle(int radius, GameObject* flag, int timer, int iteration, int MAX_ITERATION) {


    float x = flag->GetPositionX();
    float y = flag->GetPositionY();
    for (int j = -radius / 10 ; j <= radius / 10; j+= 2)
    {

            float y2 = sqrt(radius * (MAX_ITERATION - iteration) - (pow(j, 2)));
            float x_temp = x + j;
            float y_temp = y2 + y;

            float x_temp1 = x + j;
            float y_temp1 = -y2 + y;


            float floorZ = flag->GetMapHeight(x_temp, y_temp, MAX_HEIGHT);

            Position pos;

            pos.m_positionX = x_temp;
            pos.m_positionY = y_temp;
            pos.m_positionZ = floorZ;

            flag->SummonGameObject(185578, pos, QuaternionData(), timer, GO_SUMMON_TIMED_DESPAWN);


            float floorZ2 = flag->GetMapHeight(x_temp1, y_temp1, MAX_HEIGHT);

            Position pos2;

            pos2.m_positionX = x_temp1;
            pos2.m_positionY = y_temp1;
            pos2.m_positionZ = floorZ2;

            flag->SummonGameObject(185578, pos2, QuaternionData(), timer, GO_SUMMON_TIMED_DESPAWN);

    }
}

uint32 Battleroyale::getRating(Player* player)
{

    QueryResult result = CharacterDatabase.PQuery("SELECT battleroyale_players_stats.totalDeath, SUM((battleroyale_players_stats.totalKills + (totalTop1 *3))) as rating FROM battleroyale_players_stats WHERE season = 1 AND battleroyale_players_stats.guid = %u ORDER BY rating DESC", player->GetSession()->GetAccountId());

    if (!result)
        return 0;

    Field* fields = result->Fetch();
    uint32 totalDeath = fields[0].GetUInt32();
    int rating = fields[1].GetUInt32();

    if (rating == 0)
        return rating;
    else
        return rating - totalDeath;
}

void Battleroyale::update(uint64 diff)
{
    for (auto it = Battleroyale::m_games.begin(); it != Battleroyale::m_games.end(); ++it) {
        if (it->second.isStarted) {

            it->second.diff += diff;
            it->second.diffAnnoncement += diff;

            if (it->second.firstRound == true && it->second.diffAnnoncement >= 10 * IN_MILLISECONDS && (it->second.iteration < it->second.MAX_ITERATION)) { // second event timer
                it->second.diffAnnoncement = 0;
                for (auto const& guid : it->second.playersGuid) {
                    if (Player * p = ObjectAccessor::FindPlayer(guid)) {
                        ChatHandler chat(p->GetSession());
                        uint32 timer = (it->second.nextEventTimer / IN_MILLISECONDS) - (it->second.diff / IN_MILLISECONDS);
                        std::string msg = "Next reduce in " + std::to_string(timer) + " seconds";
                        chat.SendSysMessage(msg.c_str());

                        p->GetSession()->SendAreaTriggerMessage(msg.c_str());
                    }
                }
            }

            if (it->second.firstRound == false && (it->second.diff >= it->second.firstEventTimer)) { // first event
                Battleroyale::Spawn spawn = getRandomSpawn(it->second.spawnsPlayers);

                Position pos1;

                pos1.m_positionX = spawn.x;
                pos1.m_positionY = spawn.y;
                pos1.m_positionZ = spawn.z;

                GameObject* flag = summonGameObject(spawn.mapId, 181853, pos1, 0);

                it->second.flag = flag;

                for (auto const& guid : it->second.playersGuid) {
                    if (Player * p = ObjectAccessor::FindPlayer(guid)) {
                        p->GetSession()->SendAreaTriggerMessage("The area are getting smaller, the center appeared on your map!");
                        it->second.center.x = spawn.x;
                        it->second.center.y = spawn.y;
                        sendCenterToPlayer(p, spawn.x, spawn.y);
                    }
                }

                spawnCreatureAroundCircle(it->second.radius, flag, it->second.nextEventTimer, it->second.iteration, it->second.MAX_ITERATION);
                it->second.firstRound = true;
                it->second.diff = 0;
            }

            if ((it->second.diff >= it->second.nextEventTimer) && it->second.firstRound == true) {

                if (it->second.MAX_ITERATION >= 9)
                    it->second.iteration += 3;
                if(it->second.MAX_ITERATION >= 12)
                    it->second.iteration += 4;
                if(it->second.MAX_ITERATION >= 15)
                    it->second.iteration += 5;

                int MAX_ITERATION = it->second.MAX_ITERATION;
                int radius = calculateRadius(it->second.radius, it->second.iteration, MAX_ITERATION);
                spawnCreatureAroundCircle(radius, it->second.flag, it->second.nextEventTimer, it->second.iteration, MAX_ITERATION);

                for (auto const& guid : it->second.playersGuid) {
                    if (Player * p = ObjectAccessor::FindPlayer(guid)) {
                        sendCenterToPlayer(p, it->second.center.x, it->second.center.y);
                        p->GetSession()->SendAreaTriggerMessage("The area are getting smaller!");
                    }
                }

                it->second.diff = 0;
            }
        }
    }
}

void Battleroyale::updatePlayer(Player* player, uint64 p_time)
{
    for (auto it = Battleroyale::m_games.begin(); it != Battleroyale::m_games.end(); ++it)
    {
        if (it->second.isStarted == true && it->second.firstRound == true && (it->second.iteration >= 1 || it->second.minPlayer < 8)) {
            for (auto const& guid : it->second.playersGuid) {
                if (Player * p = ObjectAccessor::FindPlayer(guid)) {
                    if (p->GetGUID() == player->GetGUID()) {
                        GameObject* flag = it->second.flag;
                        float distance = player->GetDistance(flag->GetPosition());
                        int MAX_ITERATION = it->second.MAX_ITERATION;
                        float radius = calculateRadius(it->second.radius, it->second.iteration, MAX_ITERATION);
                        if (distance > (radius / 10)) {
                            if (player->HasAura(39672) == false)
                                player->AddAura(39672, player);
                        }
                        else {
                            if (player->GetAuraCount(39672) > 0)
                                player->RemoveAura(39672);
                        }
                        break;
                    }
                }
            }
        }
    }
}

void updateKillCount(Player* player, uint32 gameId) {

    if (Battleroyale::m_games[gameId].mPlayersKills.count(player->GetGUID()) > 0)
        Battleroyale::m_games[gameId].mPlayersKills[player->GetGUID()] += 1;
    else 
        Battleroyale::m_games[gameId].mPlayersKills[player->GetGUID()] = 1;
}

void Battleroyale::onPlayerDie(Player* player, Player* killer)
{
    uint32 gameId = Battleroyale::checkIfPlayerIsInBattleRoyale(player);
    if (gameId > 0) {
        updateKillCount(killer, gameId);
        killer->HandleEmoteCommand(TEXT_EMOTE_CHEER);
        killer->CastSpell(killer, 30161);
        player->GetSession()->SendAreaTriggerMessage("30 Secs until release.");
    }
}

bool Battleroyale::gameIsStarted(uint32 gameId)
{
    return Battleroyale::m_games[gameId].isStarted;
}

uint32 Battleroyale::checkIfPlayerIsInBattleRoyale(Player* player)
{

    uint32 gameId = 0;
    for (auto it = Battleroyale::m_games.begin(); it != Battleroyale::m_games.end(); ++it)
    {
            for (auto const& guid : it->second.playersGuid) {
                Player* p = ObjectAccessor::FindPlayer(guid);

                if (!p)
                    return gameId;


                if (p->GetGUID() == player->GetGUID()) {
                    gameId = it->first;
                    break;
                }
            }
    }

    return gameId;
}

