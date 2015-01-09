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
 * Scripts for spells with SPELLFAMILY_HUNTER, SPELLFAMILY_PET and SPELLFAMILY_GENERIC spells used by hunter players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_hun_".
 */

#include "ScriptPCH.h"
#include "SpellAuraEffects.h"

enum HunterSpells
{
    HUNTER_SPELL_READINESS                       = 23989,
    HUNTER_SPELL_BESTIAL_WRATH                   = 19574,
    HUNTER_PET_SPELL_LAST_STAND_TRIGGERED        = 53479,
    HUNTER_PET_HEART_OF_THE_PHOENIX              = 55709,
    HUNTER_PET_HEART_OF_THE_PHOENIX_TRIGGERED    = 54114,
    HUNTER_PET_HEART_OF_THE_PHOENIX_DEBUFF       = 55711,
    HUNTER_PET_SPELL_CARRION_FEEDER_TRIGGERED    = 54045,
    HUNTER_SPELL_INVIGORATION_TRIGGERED          = 53398,
    HUNTER_SPELL_MASTERS_CALL_TRIGGERED          = 62305,
    HUNTER_SPELL_STREADY_SHOT_ATTACK_SPEED       = 53220,
    HUNTER_SPELL_FOCUS_FIRE                      = 82692,
    HUNTER_PET_SPELL_FRENZY                      = 19615,
    HUNTER_PET_SPELL_FOCUS_FIRE_REGEN            = 83468,
    HUNTER_VISUAL_FOCUS_FIRE                     = 88843,
    HUNTER_PET_AURA_FRENZY_TRIGGER               = 20784, // Tamed Pet Passive 07 (DND)
    HUNTER_SPELL_KILL_COMMAND                    = 34026,
    HUNTER_SPELL_KILL_COMMAND_TRIGGER            = 83381,
};

// 53412 Invigoration
class spell_hun_invigoration : public SpellScriptLoader
{
public:
    spell_hun_invigoration() : SpellScriptLoader("spell_hun_invigoration") { }

    class spell_hun_invigoration_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_invigoration_SpellScript)
        bool Validate(SpellEntry const * /*spellEntry*/)
        {
            if (!sSpellStore.LookupEntry(HUNTER_SPELL_INVIGORATION_TRIGGERED))
                return false;
            return true;
        }

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            if (Unit* unitTarget = GetHitUnit())
                if (AuraEffect* aurEff = unitTarget->GetDummyAuraEffect(SPELLFAMILY_HUNTER, 3487, 0))
                    if (roll_chance_i(aurEff->GetAmount()))
                        unitTarget->CastSpell(unitTarget, HUNTER_SPELL_INVIGORATION_TRIGGERED, true);
        }

        void Register()
        {
            OnEffect += SpellEffectFn(spell_hun_invigoration_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_invigoration_SpellScript();
    }
};

class spell_hun_last_stand_pet : public SpellScriptLoader
{
public:
    spell_hun_last_stand_pet() : SpellScriptLoader("spell_hun_last_stand_pet") { }

    class spell_hun_last_stand_pet_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_last_stand_pet_SpellScript)
        bool Validate(SpellEntry const * /*spellEntry*/)
        {
            if (!sSpellStore.LookupEntry(HUNTER_PET_SPELL_LAST_STAND_TRIGGERED))
                return false;
            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit *caster = GetCaster();
            int32 healthModSpellBasePoints0 = int32(caster->CountPctFromMaxHealth(30));
            caster->CastCustomSpell(caster, HUNTER_PET_SPELL_LAST_STAND_TRIGGERED, &healthModSpellBasePoints0, NULL, NULL, true, NULL);
        }

        void Register()
        {
            // add dummy effect spell handler to pet's Last Stand
            OnEffect += SpellEffectFn(spell_hun_last_stand_pet_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_last_stand_pet_SpellScript();
    }
};

class spell_hun_masters_call : public SpellScriptLoader
{
public:
    spell_hun_masters_call() : SpellScriptLoader("spell_hun_masters_call") { }

    class spell_hun_masters_call_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_masters_call_SpellScript)
        bool Validate(SpellEntry const * spellEntry)
        {
            if (!sSpellStore.LookupEntry(HUNTER_SPELL_MASTERS_CALL_TRIGGERED))
                return false;
            if (!sSpellStore.LookupEntry(SpellMgr::CalculateSpellEffectAmount(spellEntry, EFFECT_0)))
                return false;
            if (!sSpellStore.LookupEntry(SpellMgr::CalculateSpellEffectAmount(spellEntry, EFFECT_1)))
                return false;
            return true;
        }

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            if (Unit * target = GetHitUnit())
            {
                target->CastSpell(target, GetEffectValue(), true);
                target->CastSpell(target, HUNTER_SPELL_MASTERS_CALL_TRIGGERED, true);
                // there is a possibility that this effect should access effect 0 (dummy) target, but i dubt that
                // it's more likely that on on retail it's possible to call target selector based on dbc values
                // anyways, we're using GetTargetUnit() here and it's ok
                if (Unit * ally = GetTargetUnit())
                {
                    target->CastSpell(ally, GetEffectValue(), true);
                    target->CastSpell(ally, SpellMgr::CalculateSpellEffectAmount(GetSpellInfo(), EFFECT_0), true);
                }
            }
        }

        void Register()
        {
            OnEffect += SpellEffectFn(spell_hun_masters_call_SpellScript::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_masters_call_SpellScript();
    }
};

class spell_hun_readiness : public SpellScriptLoader
{
public:
    spell_hun_readiness() : SpellScriptLoader("spell_hun_readiness") { }

    class spell_hun_readiness_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_readiness_SpellScript)
        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit *caster = GetCaster();
            if (caster->GetTypeId() != TYPEID_PLAYER)
                return;

            // immediately finishes the cooldown on your other Hunter abilities except Bestial Wrath
            const SpellCooldowns& cm = caster->ToPlayer()->GetSpellCooldownMap();
            for (SpellCooldowns::const_iterator itr = cm.begin(); itr != cm.end();)
            {
                SpellEntry const *spellInfo = sSpellStore.LookupEntry(itr->first);

                ///! If spellId in cooldown map isn't valid, the above will return a null pointer.
                if (spellInfo &&
                    spellInfo->SpellFamilyName == SPELLFAMILY_HUNTER &&
                    spellInfo->Id != HUNTER_SPELL_READINESS &&
                    spellInfo->Id != HUNTER_SPELL_BESTIAL_WRATH &&
                    GetSpellRecoveryTime(spellInfo) > 0)
                    caster->ToPlayer()->RemoveSpellCooldown((itr++)->first,true);
                else
                    ++itr;
            }
        }

        void Register()
        {
            // add dummy effect spell handler to Readiness
            OnEffect += SpellEffectFn(spell_hun_readiness_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_readiness_SpellScript();
    }
};

// 37506 Scatter Shot
class spell_hun_scatter_shot : public SpellScriptLoader
{
public:
    spell_hun_scatter_shot() : SpellScriptLoader("spell_hun_scatter_shot") { }

    class spell_hun_scatter_shot_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_scatter_shot_SpellScript)
        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            if (caster->GetTypeId() != TYPEID_PLAYER)
                return;

            // break Auto Shot and autohit
            caster->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
            caster->AttackStop();
            caster->ToPlayer()->SendAttackSwingCancelAttack();
        }

        void Register()
        {
            OnEffect += SpellEffectFn(spell_hun_scatter_shot_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_scatter_shot_SpellScript();
    }
};

// 53302, 53303, 53304 Sniper Training
enum eSniperTrainingSpells
{
    SPELL_SNIPER_TRAINING_R1        = 53302,
    SPELL_SNIPER_TRAINING_BUFF_R1   = 64418,
};

class spell_hun_sniper_training : public SpellScriptLoader
{
public:
    spell_hun_sniper_training() : SpellScriptLoader("spell_hun_sniper_training") { }

    class spell_hun_sniper_training_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_sniper_training_AuraScript)
        bool Validate(SpellEntry const * /*entry*/)
        {
            if (!sSpellStore.LookupEntry(SPELL_SNIPER_TRAINING_R1))
                return false;
            if (!sSpellStore.LookupEntry(SPELL_SNIPER_TRAINING_BUFF_R1))
                return false;
            return true;
        }

        void HandlePeriodic(AuraEffect const * aurEff)
        {
            PreventDefaultAction();
            if (aurEff->GetAmount() > 0)
                return;

            Unit * pTarget = GetTarget();

            if (!pTarget->ToPlayer()->isInCombat())
                return;

            uint32 spellId = SPELL_SNIPER_TRAINING_BUFF_R1 + GetId() - SPELL_SNIPER_TRAINING_R1;

            if (!pTarget->HasAura(spellId))
            {
                SpellEntry const * triggeredSpellInfo = sSpellStore.LookupEntry(spellId);
                Unit* triggerCaster = GetTriggeredSpellCaster(triggeredSpellInfo, GetCaster(), pTarget);
                triggerCaster->CastSpell(pTarget, triggeredSpellInfo, true, 0, aurEff);
            }
        }

        void HandleUpdatePeriodic(AuraEffect * aurEff)
        {
            Unit * pTarget = GetUnitOwner();
            if (Player* pPlayerTarget = pTarget->ToPlayer())
            {
                int32 baseAmount = aurEff->GetBaseAmount();
                int32 amount = pPlayerTarget->isMoving() ?
                pTarget->CalculateSpellDamage(pTarget, GetSpellProto(), aurEff->GetEffIndex(), &baseAmount) :
                aurEff->GetAmount() - 1;
                aurEff->SetAmount(amount);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_sniper_training_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_hun_sniper_training_AuraScript::HandleUpdatePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript *GetAuraScript() const
    {
        return new spell_hun_sniper_training_AuraScript();
    }
};

class spell_hun_pet_heart_of_the_phoenix : public SpellScriptLoader
{
public:
    spell_hun_pet_heart_of_the_phoenix() : SpellScriptLoader("spell_hun_pet_heart_of_the_phoenix") { }

    class spell_hun_pet_heart_of_the_phoenix_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_pet_heart_of_the_phoenix_SpellScript)
        bool Validate(SpellEntry const * /*spellEntry*/)
        {
            if (!sSpellStore.LookupEntry(HUNTER_PET_HEART_OF_THE_PHOENIX_TRIGGERED))
                return false;
            if (!sSpellStore.LookupEntry(HUNTER_PET_HEART_OF_THE_PHOENIX_DEBUFF))
                return false;
            return true;
        }

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            Unit *caster = GetCaster();
            Unit *owner = caster->GetOwner();
            if (!owner || caster->HasAura(HUNTER_PET_HEART_OF_THE_PHOENIX_DEBUFF))
                return;
            owner->CastCustomSpell(HUNTER_PET_HEART_OF_THE_PHOENIX_TRIGGERED, SPELLVALUE_BASE_POINT0, 100, caster, true);
            caster->CastSpell(caster, HUNTER_PET_HEART_OF_THE_PHOENIX_DEBUFF, true);
        }

        void Register()
        {
            // add dummy effect spell handler to pet's Last Stand
            OnEffect += SpellEffectFn(spell_hun_pet_heart_of_the_phoenix_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }

        bool Load()
        {
            if (!GetCaster()->isPet())
                return false;
            return true;
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_pet_heart_of_the_phoenix_SpellScript();
    }
};

class spell_hun_pet_carrion_feeder : public SpellScriptLoader
{
public:
    spell_hun_pet_carrion_feeder() : SpellScriptLoader("spell_hun_pet_carrion_feeder") { }

    class spell_hun_pet_carrion_feeder_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_pet_carrion_feeder_SpellScript)
        bool Validate(SpellEntry const * /*spellEntry*/)
        {
            if (!sSpellStore.LookupEntry(HUNTER_PET_SPELL_CARRION_FEEDER_TRIGGERED))
                return false;
            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (!GetHitUnit())
                return;
            Unit *caster = GetCaster();
            caster->CastSpell(caster, HUNTER_PET_SPELL_CARRION_FEEDER_TRIGGERED, false);
        }

        void Register()
        {
            // add dummy effect spell handler to pet's Last Stand
            OnEffect += SpellEffectFn(spell_hun_pet_carrion_feeder_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }

        bool Load()
        {
            if (!GetCaster()->isPet())
                return false;
            return true;
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_pet_carrion_feeder_SpellScript();
    }
};

// 56641 Steady Shot
class spell_hun_steady_shot : public SpellScriptLoader
{
public:
    spell_hun_steady_shot() : SpellScriptLoader("spell_hun_steady_shot") { }

    class spell_hun_steady_shot_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_steady_shot_SpellScript)

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            ++castCount;
            if(castCount > 1)
            {
                if(Unit* caster = GetCaster())
                {
                    int32 speed0 = 0;
                    if(caster->HasAura(53221))
                        speed0 = 5;
                    if(caster->HasAura(53222))
                        speed0 = 10;
                    if(caster->HasAura(53224))
                        speed0 = 15;

                    if(speed0)
                        caster->CastCustomSpell(caster, HUNTER_SPELL_STREADY_SHOT_ATTACK_SPEED, &speed0, NULL, NULL, true, 0, 0, caster->GetGUID());

                    castCount = 0;

                    if (caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    caster->ToPlayer()->KilledMonsterCredit(44175, 0);
                }
            }
        }

        void Register()
        {
            OnEffect += SpellEffectFn(spell_hun_steady_shot_SpellScript::HandleDummy, EFFECT_2, SPELL_EFFECT_DUMMY);
        }

        private:
            int castCount;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_steady_shot_SpellScript();
    }
};

// 82692 Focus Fire
class spell_hun_focus_fire : public SpellScriptLoader
{
public:
    spell_hun_focus_fire() : SpellScriptLoader("spell_hun_focus_fire") { }

    // SpellScript
    class spell_hun_focus_fire_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_focus_fire_SpellScript)

        bool Validate(SpellEntry const * /*spellEntry*/)
        {
            if (!sSpellStore.LookupEntry(HUNTER_SPELL_FOCUS_FIRE))
                return false;
            return true;
        }

        SpellCastResult CheckFrenzyStack()
        {
            Unit * pet = GetCaster()->GetGuardianPet();
            if (!pet)
                return SPELL_FAILED_NO_PET;

            if (pet->HasAura(HUNTER_PET_SPELL_FRENZY))
                return SPELL_CAST_OK;
            else
                return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
        }

        void RemoveFrenzyStacks()
        {
            if (Unit* pet = GetCaster()->GetGuardianPet())
                pet->RemoveAura(HUNTER_PET_SPELL_FRENZY);
        }
        
        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_hun_focus_fire_SpellScript::CheckFrenzyStack);
            AfterHit += SpellHitFn(spell_hun_focus_fire_SpellScript::RemoveFrenzyStacks);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_focus_fire_SpellScript();
    }

    // AuraScript
    class spell_hun_focus_fire_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_focus_fire_AuraScript);

        uint32 GetFrenzyStackCount()
        {
            if (Unit* pet = GetUnitOwner()->GetGuardianPet())
            {
                if (pet->HasAura(HUNTER_PET_SPELL_FRENZY))
                    return pet->GetAuraCount(HUNTER_PET_SPELL_FRENZY);
                else
                    return 0;
            }
            return 0;
        }

        void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
        {
            amount = GetSpellProto()->EffectBasePoints[0] * GetFrenzyStackCount();
            sLog->outDetail("Focus fire with %u", amount);
        }

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            sLog->outDetail("OnApply");
            if (Unit* caster = GetUnitOwner())
            {
                if (Unit* pet = caster->GetGuardianPet())
                {
                    int32 basepoint0 = GetAura()->GetEffect(1)->GetAmount() * GetFrenzyStackCount();
                    caster->CastCustomSpell(pet, HUNTER_PET_SPELL_FOCUS_FIRE_REGEN, &basepoint0, NULL, NULL, true);
                }
            }
        }

        void Register()
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_focus_fire_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_RANGED_HASTE);
            OnEffectApply += AuraEffectApplyFn(spell_hun_focus_fire_AuraScript::OnApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hun_focus_fire_AuraScript();
    }
};

// 19615 Frenzy Effect
class spell_hun_frenzy_effect : public SpellScriptLoader
{
public:
    spell_hun_frenzy_effect() : SpellScriptLoader("spell_hun_frenzy_effect") { }

    class spell_hun_frenzy_effect_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_frenzy_effect_AuraScript)

        int32 GetHasteValue()
        {
            if (Unit* pet = GetUnitOwner())
            {
                if (Aura* frenzyAura = pet->GetAura(HUNTER_PET_AURA_FRENZY_TRIGGER))
                    return frenzyAura->GetEffect(0)->GetAmount();
                else
                    return 0;
            }
            return 0;
        }

        void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                Unit* petOwner = pet->GetOwner();
                if (!petOwner)
                    return;
             
                amount = GetHasteValue();
            }
        }

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            // Apply UI Visual when at 5 stack
            if (Unit* petOwner = GetUnitOwner()->GetOwner())
            {
                if (GetStackAmount() >= GetSpellProto()->StackAmount && !petOwner->HasAura(HUNTER_VISUAL_FOCUS_FIRE)) // Added !petOwner->HasAura(HUNTER_VISUAL_FOCUS_FIRE) so it doesnt reapply the aura.
                    petOwner->AddAura(HUNTER_VISUAL_FOCUS_FIRE, petOwner);
            }
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* pet = GetUnitOwner())
            {
                if (Unit * petOwner = pet->GetOwner())
                    petOwner->RemoveAura(HUNTER_VISUAL_FOCUS_FIRE);
            }
        }
                

        void Register()
        {
             OnEffectApply += AuraEffectApplyFn(spell_hun_frenzy_effect_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_MELEE_ATTACK_SPEED, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
             OnEffectRemove += AuraEffectApplyFn(spell_hun_frenzy_effect_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_MELEE_ATTACK_SPEED, AURA_EFFECT_HANDLE_REAL);
             DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_frenzy_effect_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_MELEE_ATTACK_SPEED);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hun_frenzy_effect_AuraScript();
    }
};

// 34026 Kill comamnd
class spell_hun_kill_command : public SpellScriptLoader
{
public:
    spell_hun_kill_command() : SpellScriptLoader("spell_hun_kill_command") { }

    class spell_hun_kill_command_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_kill_command_SpellScript)
        bool Validate(SpellEntry const* /*spellEntry*/)
        {
            if (!sSpellStore.LookupEntry(HUNTER_SPELL_FOCUS_FIRE))
                return false;
            return true;
        }

        SpellCastResult CheckCastMeet()
        {
            Unit* pet = GetCaster()->GetGuardianPet();
            Unit* petTarget = pet->getVictim();
            if (!pet)
                return SPELL_FAILED_NO_PET;

            // Make sure pet has a target and target is within 5 yards
            if (!petTarget || !pet->IsWithinDist(petTarget, 5.0f, true))
            {
                SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_TARGET_TOO_FAR);
                return SPELL_FAILED_CUSTOM_ERROR;
            }

            return SPELL_CAST_OK;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* pet = GetCaster()->GetGuardianPet();

            if (!pet)
                return;

            pet->CastSpell(pet->getVictim(), HUNTER_SPELL_KILL_COMMAND_TRIGGER, true);
        }
        
        
        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_hun_kill_command_SpellScript::CheckCastMeet);
            OnEffect += SpellEffectFn(spell_hun_kill_command_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hun_kill_command_SpellScript();
    }
};

void AddSC_hunter_spell_scripts()
{
    new spell_hun_invigoration();
    new spell_hun_last_stand_pet();
    new spell_hun_masters_call();
    new spell_hun_readiness();
    new spell_hun_scatter_shot();
    new spell_hun_sniper_training();
    new spell_hun_pet_heart_of_the_phoenix();
    new spell_hun_pet_carrion_feeder();
    new spell_hun_steady_shot();
    new spell_hun_focus_fire();
    new spell_hun_frenzy_effect();
    new spell_hun_kill_command();
}