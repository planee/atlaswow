#include "StatSystem.h"

void StatSystem::prepareStats()
{
    StatSystem::StatDestributed = true;
}

void StatSystem::onPlayerConnect(Player* player)
{

}

void StatSystem::UpdateStatsPlayer(Player* player, uint32 stat, float amount)
{
    uint32 stats = 0;


    switch (stat)
    {
    case 1: // Spirit
        
            stats = player->GetFlatModifierValue(UNIT_MOD_STAT_SPIRIT, TOTAL_VALUE) + amount;
        

        player->SetStatFlatModifier(UNIT_MOD_STAT_SPIRIT, TOTAL_VALUE, stats);
        break;
    case 2: // Strength
        
            stats = player->GetFlatModifierValue(UNIT_MOD_STAT_STRENGTH, TOTAL_VALUE) + amount;
        

        player->SetStatFlatModifier(UNIT_MOD_STAT_STRENGTH, TOTAL_VALUE, stats);
        break;
    case 3: // Stamina
        
            stats = player->GetFlatModifierValue(UNIT_MOD_STAT_STAMINA, TOTAL_VALUE) + amount;
        

        player->SetStatFlatModifier(UNIT_MOD_STAT_STAMINA, TOTAL_VALUE, stats);
        break;
    case 4: // Agility
        
            stats = player->GetFlatModifierValue(UNIT_MOD_STAT_AGILITY, TOTAL_VALUE) + amount;
        

        player->SetStatFlatModifier(UNIT_MOD_STAT_AGILITY, TOTAL_VALUE, stats);
        break;
    case 5: // Intellect
        
            stats = player->GetFlatModifierValue(UNIT_MOD_STAT_INTELLECT, TOTAL_VALUE) + amount;
        

        player->SetStatFlatModifier(UNIT_MOD_STAT_INTELLECT, TOTAL_VALUE, stats);
        break;
    case 6: // melee attack power
        
            stats = player->GetFlatModifierValue(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE) + amount;
        

        player->SetStatFlatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE, stats);
        break;
    case 7:
        
            player->ApplyRatingMod(CR_CRIT_MELEE, amount, true);
        break;
    case 8:
        
            player->ApplyRatingMod(CR_HIT_MELEE, amount, true);
        break;
    case 9:
        
            player->ApplyRatingMod(CR_EXPERTISE, amount, true);
        break;
    case 10:
        
            player->ApplyRatingMod(CR_ARMOR_PENETRATION, amount, true);
        break;
    case 11:
        
            player->ApplyRatingMod(CR_HASTE_MELEE, amount, true);
        break;
    case 12:
        
            stats = player->GetFlatModifierValue(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE) + amount;
        player->SetStatFlatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, stats);
        break;
    case 13:
        
            player->ApplyRatingMod(CR_CRIT_RANGED, amount, true);
        break;
    case 14:
        
            player->ApplyRatingMod(CR_HIT_RANGED, amount, true);
        break;
    case 15:
        
            player->ApplyRatingMod(CR_HASTE_RANGED, amount, true);
        break;
    case 16:
        
            player->ApplySpellPowerBonus(amount, true);
        break;
    case 17:
        
            player->ApplyRatingMod(CR_CRIT_SPELL, amount, true);
        break;
    case 18:
        
            player->ApplyRatingMod(CR_HIT_SPELL, amount, true);
        break;
    case 19:
        
            player->ApplyRatingMod(CR_HASTE_SPELL, amount, true);
        break;
    case 20:
        
            player->ApplySpellPenetrationBonus(amount, true);
        break;
    case 21:
        
            player->ApplyRatingMod(CR_DODGE, amount, true);
        break;
    case 22:
        
            player->ApplyRatingMod(CR_PARRY, player->GetRatingBonusValue(CR_PARRY) + amount, true);
        break;
    case 23:
        
            player->ApplyRatingMod(CR_BLOCK, amount, true);
        break;
    }
}

