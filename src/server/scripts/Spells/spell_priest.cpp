/*
 * Copyright (C) 2010-2012 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2005-2012 MaNGOS <http://www.getmangos.com/>
 * Copyright (C) 2008-2012 Trinity <http://www.trinitycore.org/>
 * Copyright (C) 2005-2012 ScriptDev2 <http://http://www.scriptdev2.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Scripts for spells with SPELLFAMILY_PRIEST and SPELLFAMILY_GENERIC spells used by priest players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_pri_".
 */

#include "ScriptPCH.h"
#include "SpellAuraEffects.h"

enum PriestSpells
{
    PRIEST_SPELL_PENANCE_R1                      = 47540,
    PRIEST_SPELL_PENANCE_R1_DAMAGE               = 47758,
    PRIEST_SPELL_PENANCE_R1_HEAL                 = 47757,
    PRIEST_SPELL_SWD                             = 32379,
    PRIEST_SPELL_SWD_RETURN                      = 32409,
    PRIEST_SPELL_SWD_GLYPH_MARKER                = 95652,
    PRIEST_SPELL_SWD_GLYPH                       = 55682,
};

// Guardian Spirit
class spell_pri_guardian_spirit : public SpellScriptLoader
{
public:
    spell_pri_guardian_spirit() : SpellScriptLoader("spell_pri_guardian_spirit") { }

    class spell_pri_guardian_spirit_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_guardian_spirit_AuraScript);

        uint32 healPct;

        enum Spell
        {
            PRI_SPELL_GUARDIAN_SPIRIT_HEAL = 48153,
        };

        bool Validate(SpellEntry const * /*spellEntry*/)
        {
            return sSpellStore.LookupEntry(PRI_SPELL_GUARDIAN_SPIRIT_HEAL);
        }

        bool Load()
        {
            healPct = SpellMgr::CalculateSpellEffectAmount(GetSpellProto(), EFFECT_1);
            return true;
        }

        void CalculateAmount(AuraEffect const * /*aurEff*/, int32 & amount, bool & canBeRecalculated)
        {
            // Set absorbtion amount to unlimited
            amount = -1;
        }

        void Absorb(AuraEffect * aurEff, DamageInfo & dmgInfo, uint32 & absorbAmount)
        {
            Unit * target = GetTarget();
            if (dmgInfo.GetDamage() < target->GetHealth())
                return;

            int32 healAmount = int32(target->CountPctFromMaxHealth(healPct));
            // remove the aura now, we don't want 40% healing bonus
            Remove(AURA_REMOVE_BY_ENEMY_SPELL);
            target->CastCustomSpell(target, PRI_SPELL_GUARDIAN_SPIRIT_HEAL, &healAmount, NULL, NULL, true);
            absorbAmount = dmgInfo.GetDamage();
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_guardian_spirit_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
            OnEffectAbsorb += AuraEffectAbsorbFn(spell_pri_guardian_spirit_AuraScript::Absorb, EFFECT_1);
        }
    };

    AuraScript *GetAuraScript() const
    {
        return new spell_pri_guardian_spirit_AuraScript();
    }
};

class spell_pri_mana_burn : public SpellScriptLoader
{
    public:
        spell_pri_mana_burn() : SpellScriptLoader("spell_pri_mana_burn") { }

        class spell_pri_mana_burn_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_mana_burn_SpellScript)
            bool Validate(SpellEntry const * /*spellEntry*/)
            {
                return true;
            }

            void HandleAfterHit()
            {
                Unit * unitTarget = GetHitUnit();
                if (!unitTarget)
                    return;

                unitTarget->RemoveAurasWithMechanic((1 << MECHANIC_FEAR) | (1 << MECHANIC_POLYMORPH));
            }

            void Register()
            {
                AfterHit += SpellHitFn(spell_pri_mana_burn_SpellScript::HandleAfterHit);
            }
        };

        SpellScript * GetSpellScript() const
        {
            return new spell_pri_mana_burn_SpellScript;
        }
};

class spell_pri_pain_and_suffering_proc : public SpellScriptLoader
{
    public:
        spell_pri_pain_and_suffering_proc() : SpellScriptLoader("spell_pri_pain_and_suffering_proc") { }

        // 47948 Pain and Suffering (proc)
        class spell_pri_pain_and_suffering_proc_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_pain_and_suffering_proc_SpellScript)
            void HandleEffectScriptEffect(SpellEffIndex /*effIndex*/)
            {
                // Refresh Shadow Word: Pain on target
                if (Unit *unitTarget = GetHitUnit())
                    if (AuraEffect* aur = unitTarget->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_PRIEST, 0x8000, 0, 0, GetCaster()->GetGUID()))
                        aur->GetBase()->RefreshDuration();
            }

            void Register()
            {
                OnEffect += SpellEffectFn(spell_pri_pain_and_suffering_proc_SpellScript::HandleEffectScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_pri_pain_and_suffering_proc_SpellScript;
        }
};

class spell_pri_penance : public SpellScriptLoader
{
    public:
        spell_pri_penance() : SpellScriptLoader("spell_pri_penance") { }

        class spell_pri_penance_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_penance_SpellScript)
            bool Validate(SpellEntry const * spellEntry)
            {
                if (!sSpellStore.LookupEntry(PRIEST_SPELL_PENANCE_R1))
                    return false;
                // can't use other spell than this penance due to spell_ranks dependency
                if (sSpellMgr->GetFirstSpellInChain(PRIEST_SPELL_PENANCE_R1) != sSpellMgr->GetFirstSpellInChain(spellEntry->Id))
                    return false;

                uint8 rank = sSpellMgr->GetSpellRank(spellEntry->Id);
                if (!sSpellMgr->GetSpellWithRank(PRIEST_SPELL_PENANCE_R1_DAMAGE, rank, true))
                    return false;
                if (!sSpellMgr->GetSpellWithRank(PRIEST_SPELL_PENANCE_R1_HEAL, rank, true))
                    return false;

                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit *unitTarget = GetHitUnit();
                if (!unitTarget || !unitTarget->isAlive())
                    return;

                Unit *caster = GetCaster();

                uint8 rank = sSpellMgr->GetSpellRank(GetSpellInfo()->Id);

                if (caster->IsFriendlyTo(unitTarget))
                    caster->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(PRIEST_SPELL_PENANCE_R1_HEAL, rank), false, 0);
                else
                    caster->CastSpell(unitTarget, sSpellMgr->GetSpellWithRank(PRIEST_SPELL_PENANCE_R1_DAMAGE, rank), false, 0);
            }

            void Register()
            {
                // add dummy effect spell handler to Penance
                OnEffect += SpellEffectFn(spell_pri_penance_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript *GetSpellScript() const
        {
            return new spell_pri_penance_SpellScript;
        }
};

// Reflective Shield
class spell_pri_reflective_shield_trigger : public SpellScriptLoader
{
public:
    spell_pri_reflective_shield_trigger() : SpellScriptLoader("spell_pri_reflective_shield_trigger") { }

    class spell_pri_reflective_shield_trigger_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pri_reflective_shield_trigger_AuraScript);

        enum Spells
        {
            SPELL_PRI_REFLECTIVE_SHIELD_TRIGGERED = 33619,
            SPELL_PRI_REFLECTIVE_SHIELD_R1 = 33201,
        };

        bool Validate(SpellEntry const * /*spellEntry*/)
        {
            return sSpellStore.LookupEntry(SPELL_PRI_REFLECTIVE_SHIELD_TRIGGERED) && sSpellStore.LookupEntry(SPELL_PRI_REFLECTIVE_SHIELD_R1);
        }

        void Trigger(AuraEffect * aurEff, DamageInfo & dmgInfo, uint32 & absorbAmount)
        {
            Unit * target = GetTarget();
            if (dmgInfo.GetAttacker() == target)
                return;
            Unit * caster = GetCaster();
            if (!caster)
                return;
            if (AuraEffect * talentAurEff = target->GetAuraEffectOfRankedSpell(SPELL_PRI_REFLECTIVE_SHIELD_R1, EFFECT_0))
            {
                int32 bp = CalculatePctN(absorbAmount, talentAurEff->GetAmount());
                target->CastCustomSpell(dmgInfo.GetAttacker(), SPELL_PRI_REFLECTIVE_SHIELD_TRIGGERED, &bp, NULL, NULL, true, NULL, aurEff);
            }
        }

        void Register()
        {
             AfterEffectAbsorb += AuraEffectAbsorbFn(spell_pri_reflective_shield_trigger_AuraScript::Trigger, EFFECT_0);
        }
    };

    AuraScript *GetAuraScript() const
    {
        return new spell_pri_reflective_shield_trigger_AuraScript();
    }
};

class spell_priest_flash_heal : public SpellScriptLoader
{
    public:
        spell_priest_flash_heal() : SpellScriptLoader("spell_priest_flash_heal") { }

        class spell_priest_flash_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_priest_flash_heal_SpellScript)

            bool Validate(SpellEntry const * /*spellEntry*/)
            {
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit * caster = GetCaster())
                {
                    if (caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    caster->ToPlayer()->KilledMonsterCredit(44175, 0);
                }
            }

            void Register()
            {
                OnEffect += SpellEffectFn(spell_priest_flash_heal_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_HEAL);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_priest_flash_heal_SpellScript();
        }
};

// Shadow Word: Death
// Spell Id: 32379
class spell_pri_shadow_word_death : public SpellScriptLoader
{
    public:
        spell_pri_shadow_word_death() : SpellScriptLoader("spell_pri_shadow_word_death") { }

        class spell_pri_shadow_word_death_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_shadow_word_death_SpellScript);

            void DamageCaster(SpellEffIndex effIndex)
            {
                if (Unit* caster = GetCaster())
                {
                    Unit* target = GetHitUnit();
                    if (!target)
                        return;

                    if (target->HealthBelowPct(25)) // Deals 3 times more damage to targets below 25% health
                        SetHitDamage(GetHitDamage() * 3);

                    int32 back_damage = GetHitDamage();
                    if (AuraEffect* aurEff = caster->GetDummyAuraEffect(SPELLFAMILY_PRIEST, 2874, 1)) // Pain and Suffering reduces damage
                        back_damage -= aurEff->GetAmount() * back_damage / 100;

                    if (back_damage < (int32)target->GetHealth()) // If SWD dont kill de target
                        caster->CastCustomSpell(caster, PRIEST_SPELL_SWD_RETURN, &back_damage, NULL, NULL, true);

                    if (Aura* swdGlyph = caster->GetAura(PRIEST_SPELL_SWD_GLYPH)) // Glyph of Shadow word: death
                    {
                        if (target->HealthBelowPct(swdGlyph->GetSpellProto()->EffectBasePoints[0]) && !target->HasAura(PRIEST_SPELL_SWD_GLYPH_MARKER)) // If target health is below 25% and doesn't have aura with 6 secs duration used as cooldown.
                        {
                            caster->AddAura(PRIEST_SPELL_SWD_GLYPH_MARKER, target); // Spell used as a cooldown
                            if (Player* plr = caster->ToPlayer())
                                plr->RemoveSpellCooldown(PRIEST_SPELL_SWD, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnEffect += SpellEffectFn(spell_pri_shadow_word_death_SpellScript::DamageCaster, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_pri_shadow_word_death_SpellScript;
        }
};

void AddSC_priest_spell_scripts()
{
    new spell_pri_guardian_spirit();
    new spell_pri_mana_burn;
    new spell_pri_pain_and_suffering_proc;
    new spell_pri_penance;
    new spell_pri_reflective_shield_trigger();
    new spell_priest_flash_heal;
    new spell_pri_shadow_word_death();
}