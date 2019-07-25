#include "Battleroyale.h"
#include "GossipDef.h"
#include "ScriptedGossip.h"
#include "Chat.h"
#include "Player.h"
#include "WorldSession.h"
#include "SpellDefines.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "DBCStores.h"


std::string getCategory(uint32 category) {
    switch (category)
    {
    case 0:
        return "SOLO";
        break;
    case 2:
        return "DUO";
        break;
    case 3:
        return "TRIO";
        break;
    case 4:
        return "SQUAD";
        break;
    default:
        break;
    }
}
class BattleroyaleTagNPC : public CreatureScript
{
public:
    BattleroyaleTagNPC() : CreatureScript("BattleroyaleTagNPC") { }

    struct BattleroyaleTagNPC_AI : public ScriptedAI
    {
        BattleroyaleTagNPC_AI(Creature* creature) : ScriptedAI(creature) { }

        bool GossipHello(Player* player) override
        {
            for (auto it = Battleroyale::m_games.begin(); it != Battleroyale::m_games.end(); it++)
            {
                std::string icon = "|cff00ff00|TInterface\\icons\\" +it->second.icon + ":30:30:-18:0|t|r";
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, icon + it->second.name + " | Players : " +  std::to_string(it->second.playersGuid.size()) + "/" + std::to_string(it->second.minPlayer), GOSSIP_SENDER_MAIN, it->first);
            }

            uint32 gameId = Battleroyale::checkIfPlayerIsInBattleRoyale(player);
            if(gameId > 0)
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Leave the queue", GOSSIP_SENDER_MAIN, 9999);

            SendGossipMenuFor(player, 55002, me->GetGUID());
            return true;
        }

        bool GossipSelect(Player* player, uint32 /*uiSender*/, uint32 gameId) override
        {
            uint32 action = player->PlayerTalkClass->GetGossipOptionAction(gameId);

            ClearGossipMenuFor(player);

            if (!player || !me)
                return false;


            if (action == 9999) {
                Battleroyale::removePlayer(player);
            }
            else {
                Battleroyale::addPlayer(player, action);
            }

            CloseGossipMenuFor(player);
            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new BattleroyaleTagNPC_AI(creature);
    }
};

enum vendors {
    CHAIN_VENDOR = 190015,
    CLOTH_VENDOR = 190016,
    PLATE_VENDOR = 190013,
    LEATHER_VENDOR = 190014,
    WEAPONS_VENDOR = 190018,
};

class BattleroyaleWeeklyVendor : public CreatureScript
{
public:
    BattleroyaleWeeklyVendor() : CreatureScript("BattleroyaleWeeklyVendor") { }

    struct BattleroyaleWeeklyVendor_AI : public ScriptedAI
    {
        BattleroyaleWeeklyVendor_AI(Creature* creature) : ScriptedAI(creature) { }

        bool GossipHello(Player* player) override
        {
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "I would like to see what are you selling this week.", GOSSIP_SENDER_MAIN, 0);
            SendGossipMenuFor(player, 55002, me->GetGUID());
            return true;
        }

        bool GossipSelect(Player* player, uint32 /*uiSender*/, uint32 gameId) override
        {
            uint32 action = player->PlayerTalkClass->GetGossipOptionAction(gameId);


            QueryResult result = WorldDatabase.PQuery("SELECT WEEK(NOW()) as week");

            Field* fields = result->Fetch();
            uint32 week = fields[0].GetUInt32();

            ClearGossipMenuFor(player);

            if (!player || !me)
                return false;

            player->GetSession()->SendListInventory(me->GetGUID(), 800800 + week);
            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new BattleroyaleWeeklyVendor_AI(creature);
    }
};

class BattleroyaleWeaponsVendor : public CreatureScript
{
public:
    BattleroyaleWeaponsVendor() : CreatureScript("BattleroyaleWeaponsVendor") { }

    struct BattleroyaleWeaponsVendor_AI : public ScriptedAI
    {
        BattleroyaleWeaponsVendor_AI(Creature* creature) : ScriptedAI(creature) { }

        bool GossipHello(Player* player) override
        {
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Staves", GOSSIP_SENDER_MAIN, 800852);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Polearms", GOSSIP_SENDER_MAIN, 800854);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Two-Handed Maces", GOSSIP_SENDER_MAIN, 800855);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Two-Handed Swords", GOSSIP_SENDER_MAIN, 800856);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Two-Handed Axes", GOSSIP_SENDER_MAIN, 800853);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Daggers", GOSSIP_SENDER_MAIN, 800859);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "One-Handed Maces", GOSSIP_SENDER_MAIN, 800860);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "One-Handed Swords", GOSSIP_SENDER_MAIN, 800861);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "One-Handed Axes", GOSSIP_SENDER_MAIN, 800862);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Fist Weapons", GOSSIP_SENDER_MAIN,11);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Shields", GOSSIP_SENDER_MAIN, 800858);
            SendGossipMenuFor(player, 55002, me->GetGUID());
            return true;
        }

        bool GossipSelect(Player* player, uint32 /*uiSender*/, uint32 vendorId) override
        {
            uint32 action = player->PlayerTalkClass->GetGossipOptionAction(vendorId);

            ClearGossipMenuFor(player);

            if (!player || !me)
                return false;

           
            player->GetSession()->SendListInventory(me->GetGUID(), action);

            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new BattleroyaleWeaponsVendor_AI(creature);
    }
};


std::string GetSpellIcon(uint32 spellId) {

    std::ostringstream ss;
    ss << "|TInterface";
    const SpellEntry* dispInfo = sSpellStore.LookupEntry(spellId);
    if (dispInfo)
    {
        ss << "\\icons\\" << dispInfo->activeIconID;
    }
    if (!dispInfo)
        ss << "/InventoryItems/WoWUnknownItem01";

    ss << ":" << 30 << ":" << 30 << ":" << -18 << ":" << 0 << "|t";
    return ss.str();
}

class BattleroyaleMountsVendor : public CreatureScript
{
public:
    BattleroyaleMountsVendor() : CreatureScript("BattleroyaleMountsVendor") { }

    struct BattleroyaleMountsVendor_AI : public ScriptedAI
    {
        BattleroyaleMountsVendor_AI(Creature* creature) : ScriptedAI(creature) { }



        uint32 getCost(uint32 mountId) {
            QueryResult result = WorldDatabase.PQuery("SELECT cost FROM mounts_vendor WHERE id = %u", mountId);
            Field* fields = result->Fetch();
            return fields[0].GetUInt32();
        }

        uint32 getSpellId(uint32 mountId) {
            QueryResult result = WorldDatabase.PQuery("SELECT spellid FROM mounts_vendor WHERE id = %u", mountId);
            Field* fields = result->Fetch();
            return fields[0].GetUInt32();
        }
        bool hasEnough(Player* player, uint32 mountId) {
            QueryResult result = WorldDatabase.PQuery("SELECT cost FROM mounts_vendor WHERE id = %u", mountId);

            Field* fields = result->Fetch();
            uint32 cost = fields[0].GetUInt32();


            if (cost > player->GetItemCount(505050))
                return false;



            return true;
        }

        uint32 lastId = 10;

        bool GossipHello(Player* player) override
        {

            lastId = 10;
            QueryResult result = WorldDatabase.PQuery("SELECT * FROM mounts_vendor ORDER BY id LIMIT 10");

            if (!result)
                return false;

            do
            {
                Field* fields = result->Fetch();
                uint32 id = fields[0].GetUInt32();
                uint32 spellId = fields[1].GetUInt32();
                std::string name = fields[2].GetCString();
                uint32 cost = fields[3].GetUInt32();
                std::string icon = fields[4].GetString();

                std::ostringstream ss;
                ss << "|TInterface";
                ss << "\\icons\\" << icon;
                ss << ":" << 30 << ":" << 30 << ":" << -18 << ":" << 0 << "|t";

                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, ss.str() + name + " | Cost : " + std::to_string(cost), GOSSIP_SENDER_MAIN, id);

            } while (result->NextRow());

            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Next page", GOSSIP_SENDER_MAIN, 1000);
            SendGossipMenuFor(player, 55002, me->GetGUID());
            return true;
        }

        bool GossipSelect(Player* player, uint32 /*uiSender*/, uint32 vendorId) override
        {
            uint32 action = player->PlayerTalkClass->GetGossipOptionAction(vendorId);

            ClearGossipMenuFor(player);

            if (!player || !me)
                return false;


            if (action < 1000) {
                if (player->HasSpell(getSpellId(action))) {
                    ChatHandler chat(player->GetSession());
                    chat.SendSysMessage("You have already this mount.");
                    CloseGossipMenuFor(player);
                }

                if (hasEnough(player, action)) {
                    player->DestroyItemCount(505050, getCost(action), true);
                    player->LearnSpell(getSpellId(action), false);
                }
                else {
                    ChatHandler chat(player->GetSession());
                    chat.SendSysMessage("You don't have enough of Atlas credit.");
                }

                CloseGossipMenuFor(player);
            }


            if (action == 1001)
                lastId -= 20;

            if (action >= 1000) {


                QueryResult result = WorldDatabase.PQuery("SELECT * FROM mounts_vendor WHERE id > %u ORDER BY id LIMIT 10", lastId);

                if (!result)
                    return false;

                do
                {
                    Field* fields = result->Fetch();
                    uint32 id = fields[0].GetUInt32();
                    uint32 spellId = fields[1].GetUInt32();
                    std::string name = fields[2].GetCString();
                    uint32 cost = fields[3].GetUInt32();
                    std::string icon = fields[4].GetString();

                    std::ostringstream ss;
                    ss << "|TInterface";
                    ss << "\\icons\\" << icon;
                    ss << ":" << 30 << ":" << 30 << ":" << -18 << ":" << 0 << "|t";
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, ss.str() + name + " | Cost : " + std::to_string(cost), GOSSIP_SENDER_MAIN, id);

                } while (result->NextRow());


                lastId += 10;

                if (result->GetRowCount() == 10)
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Next page", GOSSIP_SENDER_MAIN, 1000);

                if (lastId > 10)
                    AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Previous page", GOSSIP_SENDER_MAIN, 1001);

                SendGossipMenuFor(player, 55002, me->GetGUID());
            }



            return true;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new BattleroyaleMountsVendor_AI(creature);
    }
};


class Battleroyale_command : public CommandScript
{
public:
    Battleroyale_command() : CommandScript("Battleroyale_command") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> BattleroyaleTemplate =
        {
            { "join",      SEC_ADMINISTRATOR, true , &HandlejoinGame, "" },
            { "leave",      SEC_ADMINISTRATOR, true , &HandleLeaveGame, "" },
            { "stats",      SEC_ADMINISTRATOR, true , &HandleStats, "" },
        };

        static std::vector<ChatCommand> commandTable =
        {
            { "atlas", SEC_ADMINISTRATOR, true, nullptr                         , "", BattleroyaleTemplate }
        };
        return commandTable;
    }


    static bool HandlejoinGame(ChatHandler* handler, const char* args)
    {

        if (!*args)
            return false;


        Player* playerTarget = handler->GetSession()->GetPlayer();

        char* spawnId = strtok((char*)args, " ");
        uint32 id = atoi(spawnId);

        if (!id)
        {
            ChatHandler(playerTarget->GetSession()).SendSysMessage("Incorrect Syntax, example : .atlas join 1");
            return false;
        }
        else {
            Battleroyale::addPlayer(playerTarget, id);
        }
        return true;
    }
    static bool HandleStats(ChatHandler* handler, const char* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Player* playerTarget = handler->getSelectedPlayer();
        if (!playerTarget)
            playerTarget = player;

        QueryResult result = CharacterDatabase.PQuery("SELECT SUM(battleroyale_players_stats.totalKills), SUM(battleroyale_players_stats.totalDeath), SUM(battleroyale_players_stats.totalTop1) FROM battleroyale_players_stats WHERE season = 1 AND guid = 1", playerTarget->GetSession()->GetAccountId());

        Field* fields = result->Fetch();
        uint32 totalKills = fields[0].GetUInt32();
        uint32 totalDeath = fields[1].GetUInt32();
        uint32 totalTop1 = fields[2].GetUInt32();
        uint32 ranking = Battleroyale::getRating(player);



        std::string msg = "Rating : " + std::to_string(ranking) + " - Total kill(s) : " + std::to_string(totalKills) + " - Total Death(s) : " + std::to_string(totalDeath) + " - Total top1(s) : " + std::to_string(totalTop1) + ". You can also use this command on any player.";

        ChatHandler(player->GetSession()).SendSysMessage(msg.c_str());

        return true;
    }
    static bool HandleLeaveGame(ChatHandler* handler, const char* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        uint32 gameId = Battleroyale::checkIfPlayerIsInBattleRoyale(player);
        if (gameId > 0)
            Battleroyale::removePlayer(player);
        else
            ChatHandler(player->GetSession()).SendSysMessage("You are not in queue");

        return true;
    }
    static bool HandleStatus(ChatHandler* handler, const char* /*args*/)
    {
        return true;
    }
};

void AddSC_BattleroyaleScript()
{
    new BattleroyaleTagNPC();
    new Battleroyale_command();
    new BattleroyaleWeeklyVendor();
    new BattleroyaleWeaponsVendor();
    new BattleroyaleMountsVendor();
}
