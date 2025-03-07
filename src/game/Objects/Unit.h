/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2011 MaNGOSZero <https://github.com/mangos/zero>
 * Copyright (C) 2011-2016 Nostalrius <https://nostalrius.org>
 * Copyright (C) 2016-2017 Elysium Project <https://github.com/elysium-project>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __UNIT_H
#define __UNIT_H

#include "Common.h"
#include "Object.h"
#include "Opcodes.h"
#include "SpellAuraDefines.h"
#include "UpdateFields.h"
#include "SharedDefines.h"
#include "ThreatManager.h"
#include "HostileRefManager.h"
#include "FollowerReference.h"
#include "FollowerRefManager.h"
#include "Utilities/EventProcessor.h"
#include "MotionMaster.h"
#include "DBCStructure.h"
#include "Path.h"
#include "WorldPacket.h"
#include "Timer.h"
#include <list>

enum UnitMoveType
{
    MOVE_WALK           = 0,
    MOVE_RUN            = 1,
    MOVE_RUN_BACK       = 2,
    MOVE_SWIM           = 3,
    MOVE_SWIM_BACK      = 4,
    MOVE_TURN_RATE      = 5,
};

#define MAX_MOVE_TYPE 6

extern float baseMoveSpeed[MAX_MOVE_TYPE];

#define MOVEMENT_PACKET_TIME_DELAY 0

enum MovementChangeType
{
    INVALID,

    ROOT,
    WATER_WALK,
    SET_HOVER,
    FEATHER_FALL,

    SPEED_CHANGE_WALK,
    SPEED_CHANGE_RUN,
    SPEED_CHANGE_RUN_BACK,
    SPEED_CHANGE_SWIM,
    SPEED_CHANGE_SWIM_BACK,
    RATE_CHANGE_TURN,

    TELEPORT,
    KNOCK_BACK
};

struct PlayerMovementPendingChange
{
    uint32 movementCounter = 0;
    MovementChangeType movementChangeType = INVALID;
    uint32 time = 0;

    float newValue = 0.0f; // used if speed or height change
    bool apply = false; // used if movement flag change
    struct KnockbackInfo
    {
        float vcos = 0.0f;
        float vsin = 0.0f;
        float speedXY = 0.0f;
        float speedZ = 0.0f;
    } knockbackInfo; // used if knockback

    PlayerMovementPendingChange();
};

enum SpellInterruptFlags
{
    SPELL_INTERRUPT_FLAG_MOVEMENT     = 0x01,
    SPELL_INTERRUPT_FLAG_DAMAGE       = 0x02,
    SPELL_INTERRUPT_FLAG_INTERRUPT    = 0x04,
    SPELL_INTERRUPT_FLAG_AUTOATTACK   = 0x08,
    SPELL_INTERRUPT_FLAG_ABORT_ON_DMG = 0x10,               // _complete_ interrupt on direct damage
};

enum SpellChannelInterruptFlags
{
    CHANNEL_FLAG_ENTER_COMBAT = 0x0001, // Nostalrius: guessed
    CHANNEL_FLAG_DAMAGE       = 0x0002,
    CHANNEL_FLAG_INTERRUPT    = 0x0004,
    CHANNEL_FLAG_MOVEMENT     = 0x0008,
    CHANNEL_FLAG_TURNING      = 0x0010,
    CHANNEL_FLAG_DAMAGE2      = 0x0080,
    CHANNEL_FLAG_DELAY        = 0x4000
};

enum SpellAuraInterruptFlags
{
    // Certains ont ete repris de TrinityCore (3.3.5)
    // ATTENTION: Tous ne sont pas encore implementes.
    // TC: Par exemple buff BG restauration
    AURA_INTERRUPT_FLAG_HITBYSPELL                  = 0x00000001,   // 0    removed when getting hit by a negative spell?
    AURA_INTERRUPT_FLAG_DAMAGE                      = 0x00000002,   // 1    removed by any damage
    // Non implemente
    AURA_INTERRUPT_FLAG_CAST                        = 0x00000004,   // 2
    AURA_INTERRUPT_FLAG_MOVE                        = 0x00000008,   // 3    removed by any movement
    AURA_INTERRUPT_FLAG_TURNING                     = 0x00000010,   // 4    removed by any turning
    AURA_INTERRUPT_FLAG_ENTER_COMBAT                = 0x00000020,   // 5    removed by entering combat - Feign death only
    AURA_INTERRUPT_FLAG_NOT_MOUNTED                 = 0x00000040,   // 6    removed by unmounting
    AURA_INTERRUPT_FLAG_NOT_ABOVEWATER              = 0x00000080,   // 7    removed by entering water
    AURA_INTERRUPT_FLAG_NOT_UNDERWATER              = 0x00000100,   // 8    removed by leaving water
    AURA_INTERRUPT_FLAG_NOT_SHEATHED                = 0x00000200,   // 9    removed by unsheathing
    AURA_INTERRUPT_FLAG_TALK                        = 0x00000400,   // 10   on interact with NPC (gossip, trainer, stable pet ...) - implemented
    AURA_INTERRUPT_FLAG_USE                         = 0x00000800,   // 11   on interact with GO - implement
    // TC - implemente
    AURA_INTERRUPT_FLAG_MELEE_ATTACK                = 0x00001000,   // 12   removed by attack?
    // TC - non implemente
    AURA_INTERRUPT_FLAG_SPELL_ATTACK                = 0x00002000,   // 13
    AURA_INTERRUPT_FLAG_UNK14                       = 0x00004000,   // 14   unused in 1.12
    // TC - non implemente
    AURA_INTERRUPT_FLAG_TRANSFORM                   = 0x00008000,   // 15   removed by casting a spell?
    AURA_INTERRUPT_FLAG_ON_CAST_SPELL               = 0x00010000,   // 16
    AURA_INTERRUPT_FLAG_MOUNTING                    = 0x00020000,   // 17   removed by mounting
    AURA_INTERRUPT_FLAG_NOT_SEATED                  = 0x00040000,   // 18   removed by standing up (used by food and drink mostly and sleep/Fake Death like)
    AURA_INTERRUPT_FLAG_CHANGE_MAP                  = 0x00080000,   // 19   leaving map/getting teleported
    AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION    = 0x00100000,   // 20   removed by auras that make you invulnerable, or make other to loose selection on you
    AURA_INTERRUPT_FLAG_UNK21                       = 0x00200000,   // 21
    AURA_INTERRUPT_FLAG_TELEPORTED                  = 0x00400000,   // 22
    // TC - non implemente (9 sorts)
    AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT            = 0x00800000,   // 23   removed by entering pvp combat
    AURA_INTERRUPT_FLAG_DIRECT_DAMAGE               = 0x01000000    // 24   removed by any direct damage
};

enum SpellModOp
{
    SPELLMOD_DAMAGE                 = 0,
    SPELLMOD_DURATION               = 1,
    SPELLMOD_THREAT                 = 2,
    SPELLMOD_ATTACK_POWER           = 3,
    SPELLMOD_CHARGES                = 4,
    SPELLMOD_RANGE                  = 5,
    SPELLMOD_RADIUS                 = 6,
    SPELLMOD_CRITICAL_CHANCE        = 7,
    SPELLMOD_ALL_EFFECTS            = 8,
    SPELLMOD_NOT_LOSE_CASTING_TIME  = 9,
    SPELLMOD_CASTING_TIME           = 10,
    SPELLMOD_COOLDOWN               = 11,
    SPELLMOD_SPEED                  = 12,
    SPELLMOD_COST                   = 14,
    SPELLMOD_CRIT_DAMAGE_BONUS      = 15,
    SPELLMOD_RESIST_MISS_CHANCE     = 16,
    SPELLMOD_JUMP_TARGETS           = 17,
    SPELLMOD_CHANCE_OF_SUCCESS      = 18,                   // Only used with SPELL_AURA_ADD_FLAT_MODIFIER and affects proc spells
    SPELLMOD_ACTIVATION_TIME        = 19,
    SPELLMOD_EFFECT_PAST_FIRST      = 20,
    SPELLMOD_CASTING_TIME_OLD       = 21,
    SPELLMOD_DOT                    = 22,
    SPELLMOD_HASTE                  = 23,
    SPELLMOD_SPELL_BONUS_DAMAGE     = 24,
    SPELLMOD_MULTIPLE_VALUE         = 27,
    SPELLMOD_RESIST_DISPEL_CHANCE   = 28,
    MAX_SPELLMOD                    = 29,
};

enum SpellFacingFlags
{
    SPELL_FACING_FLAG_INFRONT = 0x0001
};

#define BASE_MELEERANGE_OFFSET 1.33f
#define BASE_MINDAMAGE 1.0f
#define BASE_MAXDAMAGE 2.0f
#define BASE_ATTACK_TIME 2000

// byte value (UNIT_FIELD_BYTES_1,0)
enum UnitStandStateType
{
    UNIT_STAND_STATE_STAND             = 0,
    UNIT_STAND_STATE_SIT               = 1,
    UNIT_STAND_STATE_SIT_CHAIR         = 2,
    UNIT_STAND_STATE_SLEEP             = 3,
    UNIT_STAND_STATE_SIT_LOW_CHAIR     = 4,
    UNIT_STAND_STATE_SIT_MEDIUM_CHAIR  = 5,
    UNIT_STAND_STATE_SIT_HIGH_CHAIR    = 6,
    UNIT_STAND_STATE_DEAD              = 7,
    UNIT_STAND_STATE_KNEEL             = 8
};

#define MAX_UNIT_STAND_STATE             9

/* byte flag value not exist in 1.12, moved/merged in (UNIT_FIELD_BYTES_1,3), in post-1.x it's in (UNIT_FIELD_BYTES_1,2)
enum UnitStandFlags
*/

// byte flags value (UNIT_FIELD_BYTES_1,3)
enum UnitBytes1_Flags
{
    UNIT_BYTE1_FLAG_ALWAYS_STAND = 0x01, // Ajoute a la mort. Non affichage du nom des vivants.
    UNIT_BYTE1_FLAGS_CREEP       = 0x02,
    UNIT_BYTE1_FLAG_UNTRACKABLE  = 0x04,
    UNIT_BYTE1_FLAG_ALL          = 0xFF
};

// byte value (UNIT_FIELD_BYTES_2,0)
enum SheathState
{
    SHEATH_STATE_UNARMED  = 0,                              // non prepared weapon
    SHEATH_STATE_MELEE    = 1,                              // prepared melee weapon
    SHEATH_STATE_RANGED   = 2                               // prepared ranged weapon
};

#define MAX_SHEATH_STATE    3

// byte flags value (UNIT_FIELD_BYTES_2,1)
enum UnitBytes2_Flags
{
    UNIT_BYTE2_FLAG_PVP         = 0x01,
    UNIT_BYTE2_FLAG_UNK1        = 0x02,
    UNIT_BYTE2_FLAG_FFA_PVP     = 0x04,
    UNIT_BYTE2_FLAG_UNK3        = 0x08,                     // FLAG_SANCTUARY on 3.3.5
    UNIT_BYTE2_FLAG_AURAS       = 0x10,                     // show possitive auras as positive, and allow its dispel
    UNIT_BYTE2_FLAG_UNK5        = 0x20,
    UNIT_BYTE2_FLAG_UNK6        = 0x40,
    UNIT_BYTE2_FLAG_UNK7        = 0x80
};

#define CREATURE_MAX_SPELLS     4

enum Swing
{
    NOSWING                    = 0,
    SINGLEHANDEDSWING          = 1,
    TWOHANDEDSWING             = 2
};

enum VictimState
{
    VICTIMSTATE_UNAFFECTED     = 0,                         // seen in relation with HITINFO_MISS
    VICTIMSTATE_NORMAL         = 1,
    VICTIMSTATE_DODGE          = 2,
    VICTIMSTATE_PARRY          = 3,
    VICTIMSTATE_INTERRUPT      = 4,
    VICTIMSTATE_BLOCKS         = 5,
    VICTIMSTATE_EVADES         = 6,
    VICTIMSTATE_IS_IMMUNE      = 7,
    VICTIMSTATE_DEFLECTS       = 8
};

enum HitInfo
{
    HITINFO_NORMALSWING         = 0x00000000,
    HITINFO_UNK0                = 0x00000001,               // req correct packet structure
    HITINFO_NORMALSWING2        = 0x00000002,
    HITINFO_LEFTSWING           = 0x00000004,
    HITINFO_UNK3                = 0x00000008,
    HITINFO_MISS                = 0x00000010,
    HITINFO_ABSORB              = 0x00000020,               // plays absorb sound
    HITINFO_RESIST              = 0x00000040,               // resisted atleast some damage
#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_9_4
    HITINFO_CRITICALHIT         = 0x00000080,
#else
    HITINFO_CRITICALHIT         = 0x00000008,
#endif
    HITINFO_UNK8                = 0x00000100,               // wotlk?
    HITINFO_UNK9                = 0x00002000,               // wotlk?
    HITINFO_GLANCING            = 0x00004000,
    HITINFO_CRUSHING            = 0x00008000,
    HITINFO_NOACTION            = 0x00010000,
    HITINFO_SWINGNOHITSOUND     = 0x00080000
};

//i would like to remove this: (it is defined in item.h
enum InventorySlot
{
    NULL_BAG                   = 0,
    NULL_SLOT                  = 255
};

struct FactionTemplateEntry;
struct Modifier;
class SpellEntry;

class Aura;
class SpellAuraHolder;
class Creature;
class Spell;
class DynamicObject;
class GameObject;
class Item;
class Pet;
class PetAura;
class Totem;
class CreatureAI;

struct SpellImmune
{
    uint32 type;
    uint32 spellId;
};

typedef std::list<SpellImmune> SpellImmuneList;

enum UnitModifierType
{
    BASE_VALUE = 0,
    BASE_PCT = 1,
    TOTAL_VALUE = 2,
    TOTAL_PCT = 3,
    MODIFIER_TYPE_END = 4
};

enum WeaponDamageRange
{
    MINDAMAGE,
    MAXDAMAGE
};

struct WeaponDamageInfo
{
    float damage[2];
    SpellSchools school;
};

enum DamageTypeToSchool
{
    RESISTANCE,
    DAMAGE_DEALT,
    DAMAGE_TAKEN
};

enum AuraRemoveMode
{
    AURA_REMOVE_BY_DEFAULT,
    AURA_REMOVE_BY_STACK,                                   // at replace by similar aura
    AURA_REMOVE_BY_CANCEL,
    AURA_REMOVE_BY_DISPEL,
    AURA_REMOVE_BY_DEATH,
    AURA_REMOVE_BY_DELETE,                                  // use for speedup and prevent unexpected effects at player logout/pet unsummon (must be used _only_ after save), delete.
    AURA_REMOVE_BY_SHIELD_BREAK,                            // when absorb shield is removed by damage
    AURA_REMOVE_BY_EXPIRE,                                  // at duration end
    AURA_REMOVE_BY_CHANNEL,                                 // when removing an aura due to channel finish or cancel
    AURA_REMOVE_BY_RANGE,                                   // when removing an area aura due to being out of range
    AURA_REMOVE_BY_GROUP                                    // when removing an aura due to not being in the same group (includes pet ownership)
};

enum UnitMods
{
    UNIT_MOD_STAT_STRENGTH,                                 // UNIT_MOD_STAT_STRENGTH..UNIT_MOD_STAT_SPIRIT must be in existing order, it's accessed by index values of Stats enum.
    UNIT_MOD_STAT_AGILITY,
    UNIT_MOD_STAT_STAMINA,
    UNIT_MOD_STAT_INTELLECT,
    UNIT_MOD_STAT_SPIRIT,
    UNIT_MOD_HEALTH,
    UNIT_MOD_MANA,                                          // UNIT_MOD_MANA..UNIT_MOD_HAPPINESS must be in existing order, it's accessed by index values of Powers enum.
    UNIT_MOD_RAGE,
    UNIT_MOD_FOCUS,
    UNIT_MOD_ENERGY,
    UNIT_MOD_HAPPINESS,
    UNIT_MOD_ARMOR,                                         // UNIT_MOD_ARMOR..UNIT_MOD_RESISTANCE_ARCANE must be in existing order, it's accessed by index values of SpellSchools enum.
    UNIT_MOD_RESISTANCE_HOLY,
    UNIT_MOD_RESISTANCE_FIRE,
    UNIT_MOD_RESISTANCE_NATURE,
    UNIT_MOD_RESISTANCE_FROST,
    UNIT_MOD_RESISTANCE_SHADOW,
    UNIT_MOD_RESISTANCE_ARCANE,
    UNIT_MOD_ATTACK_POWER,
    UNIT_MOD_ATTACK_POWER_RANGED,
    UNIT_MOD_DAMAGE_MAINHAND,
    UNIT_MOD_DAMAGE_OFFHAND,
    UNIT_MOD_DAMAGE_RANGED,
    UNIT_MOD_DAMAGE_PHYSICAL,
    UNIT_MOD_END,
    // synonyms
    UNIT_MOD_STAT_START = UNIT_MOD_STAT_STRENGTH,
    UNIT_MOD_STAT_END = UNIT_MOD_STAT_SPIRIT + 1,
    UNIT_MOD_RESISTANCE_START = UNIT_MOD_ARMOR,
    UNIT_MOD_RESISTANCE_END = UNIT_MOD_RESISTANCE_ARCANE + 1,
    UNIT_MOD_POWER_START = UNIT_MOD_MANA,
    UNIT_MOD_POWER_END = UNIT_MOD_HAPPINESS + 1
};

enum BaseModGroup
{
    CRIT_PERCENTAGE,
    RANGED_CRIT_PERCENTAGE,
    OFFHAND_CRIT_PERCENTAGE,
    SHIELD_BLOCK_VALUE,
    BASEMOD_END
};

enum BaseModType
{
    FLAT_MOD,
    PCT_MOD
};

#define MOD_END (PCT_MOD+1)

enum DeathState
{
    ALIVE          = 0,                                     // show as alive
    JUST_DIED      = 1,                                     // temporary state at die, for creature auto converted to CORPSE, for player at next update call
    CORPSE         = 2,                                     // corpse state, for player spirit is still within the corpse
    DEAD           = 3,                                     // for creature despawned state (corpse despawned), for player spirit has been released (player is ghost)
    JUST_ALIVED    = 4,                                     // temporary state at resurrection, for creature auto converted to ALIVE, for player at next update call
    CORPSE_FALLING = 5                                      // corpse state in case when corpse still falling to ground
};

// internal state flags for some auras and movement generators, other.
enum UnitState
{
    // persistent state (applied by aura/etc until expire)
    UNIT_STAT_MELEE_ATTACKING = 0x00000001,                     // unit is melee attacking someone Unit::Attack
    //UNIT_STAT_ATTACK_PLAYER = 0x00000002,                     // (Deprecated) unit attack player or player's controlled unit and have contested pvpv timer setup, until timer expire, combat end and etc
    UNIT_STAT_DIED            = 0x00000004,                     // Unit::SetFeignDeath
    UNIT_STAT_STUNNED         = 0x00000008,                     // Aura::HandleAuraModStun
    UNIT_STAT_ROOT            = 0x00000010,                     // Aura::HandleAuraModRoot
    UNIT_STAT_ISOLATED        = 0x00000020,                     // area auras do not affect other players, Aura::HandleAuraModSchoolImmunity
    UNIT_STAT_POSSESSED       = 0x00000040,                     // Aura::HandleAuraModPossess

    // persistent movement generator state (all time while movement generator applied to unit (independent from top state of movegen)
    UNIT_STAT_TAXI_FLIGHT     = 0x00000080,                     // player is in flight mode (in fact interrupted at far teleport until next map telport landing)
    UNIT_STAT_DISTRACTED      = 0x00000100,                     // DistractedMovementGenerator active

    // persistent movement generator state with non-persistent mirror states for stop support
    // (can be removed temporary by stop command or another movement generator apply)
    // not use _MOVE versions for generic movegen state, it can be removed temporary for unit stop and etc
    UNIT_STAT_CONFUSED        = 0x00000200,                     // ConfusedMovementGenerator active/onstack
    //UNIT_STAT_CONFUSED_MOVE   = 0x00000400,                   // No longer in use (Nostalrius)
    UNIT_STAT_ROAMING         = 0x00000800,                     // RandomMovementGenerator/PointMovementGenerator/WaypointMovementGenerator active (now always set)
    UNIT_STAT_ROAMING_MOVE    = 0x00001000,
    UNIT_STAT_CHASE           = 0x00002000,                     // ChaseMovementGenerator active
    UNIT_STAT_CHASE_MOVE      = 0x00004000,
    UNIT_STAT_FOLLOW          = 0x00008000,                     // FollowMovementGenerator active
    UNIT_STAT_FOLLOW_MOVE     = 0x00010000,
    UNIT_STAT_FLEEING         = 0x00020000,                     // FleeMovementGenerator/TimedFleeingMovementGenerator active/onstack
    UNIT_STAT_FLEEING_MOVE    = 0x00040000,
    // MMAPS
    UNIT_STAT_IGNORE_PATHFINDING    = 0x00080000,               // do not use pathfinding in any MovementGenerator

    UNIT_STAT_PENDING_ROOT          = 0x00100000,
    UNIT_STAT_PENDING_STUNNED       = 0x00200000,
    UNIT_STAT_CLIENT_ROOT           = 0x00400000,

    // High-level states
    UNIT_STAT_NO_COMBAT_MOVEMENT = 0x01000000,
    UNIT_STAT_RUNNING            = 0x02000000,
    UNIT_STAT_IGNORE_MOVE_LOS    = 0x04000000,

    UNIT_STAT_ALLOW_INCOMPLETE_PATH = 0x08000000, // allow movement with incomplete or partial paths
    UNIT_STAT_ALLOW_LOS_ATTACK      = 0x10000000, // allow melee attacks without LoS
    // masks (only for check)

    // can't move currently
    UNIT_STAT_CAN_NOT_MOVE    = UNIT_STAT_ROOT | UNIT_STAT_STUNNED | UNIT_STAT_DIED,

    // stay by different reasons
    UNIT_STAT_NOT_MOVE        = UNIT_STAT_ROOT | UNIT_STAT_STUNNED | UNIT_STAT_DIED |
                                UNIT_STAT_DISTRACTED,

    // stay or scripted movement for effect( = in player case you can't move by client command)
    UNIT_STAT_NO_FREE_MOVE    = UNIT_STAT_ROOT | UNIT_STAT_STUNNED | UNIT_STAT_DIED |
                                UNIT_STAT_TAXI_FLIGHT |
                                UNIT_STAT_CONFUSED | UNIT_STAT_FLEEING,

    // not react at move in sight or other
    UNIT_STAT_CAN_NOT_REACT   = UNIT_STAT_STUNNED | UNIT_STAT_DIED |
                                UNIT_STAT_CONFUSED | UNIT_STAT_FLEEING,

    // AI disabled by some reason
    UNIT_STAT_LOST_CONTROL    = UNIT_STAT_FLEEING | UNIT_STAT_POSSESSED,

    // above 2 state cases
    UNIT_STAT_CAN_NOT_REACT_OR_LOST_CONTROL  = UNIT_STAT_CAN_NOT_REACT | UNIT_STAT_LOST_CONTROL,

    // masks (for check or reset)

    // for real move using movegen check and stop (except unstoppable flight)
    UNIT_STAT_MOVING          = UNIT_STAT_ROAMING_MOVE | UNIT_STAT_CHASE_MOVE | UNIT_STAT_FOLLOW_MOVE | UNIT_STAT_FLEEING_MOVE,

    UNIT_STAT_ALL_STATE       = 0xFFFFFFFF,
    UNIT_STAT_ALL_DYN_STATES  = UNIT_STAT_ALL_STATE & ~(UNIT_STAT_NO_COMBAT_MOVEMENT | UNIT_STAT_RUNNING | UNIT_STAT_IGNORE_PATHFINDING),
};

/// internal used flags for marking special auras - for example some dummy-auras
enum UnitAuraFlags
{
    UNIT_AURAFLAG_ALIVE_INVISIBLE   = 0x1,                  // aura which makes unit invisible for alive
};

enum UnitVisibility
{
    VISIBILITY_OFF                = 0,                      // absolute, not detectable, GM-like, can see all other
    VISIBILITY_ON                 = 1,
    VISIBILITY_GROUP_STEALTH      = 2,                      // detect chance, seen and can see group members
    VISIBILITY_GROUP_INVISIBILITY = 3,                      // invisibility, can see and can be seen only another invisible unit or invisible detection unit, set only if not stealthed, and in checks not used (mask used instead)
    VISIBILITY_GROUP_NO_DETECT    = 4,                      // state just at stealth apply for update Grid state. Don't remove, otherwise stealth spells will break
    VISIBILITY_RESPAWN            = 5                       // special totally not detectable visibility for force delete object at respawn command
};

// [-ZERO] Need recheck values
// Value masks for UNIT_FIELD_FLAGS
enum UnitFlags
{
    UNIT_FLAG_NONE                  = 0x00000000,
    UNIT_FLAG_UNK_0                 = 0x00000001,           // Movement checks disabled, likely paired with loss of client control packet.
    UNIT_FLAG_NON_ATTACKABLE        = 0x00000002,           // not attackable
    UNIT_FLAG_DISABLE_MOVE          = 0x00000004,
    UNIT_FLAG_PLAYER_CONTROLLED     = 0x00000008,           // players, pets, totems, guardians, companions, charms, any units associated with players
    UNIT_FLAG_PET_RENAME            = 0x00000010,           // Old pet rename: moved to UNIT_FIELD_BYTES_2,2 in TBC+
    UNIT_FLAG_PET_ABANDON           = 0x00000020,           // Old pet abandon: moved to UNIT_FIELD_BYTES_2,2 in TBC+
    UNIT_FLAG_UNK_6                 = 0x00000040,
    UNIT_FLAG_IMMUNE_TO_PLAYER      = 0x00000100,           // Target is immune to players
    UNIT_FLAG_PASSIVE               = 0x00000200,           // makes you unable to attack everything. Almost identical to our "civilian"-term. Will ignore it's surroundings and not engage in combat unless "called upon" or engaged by another unit.
    UNIT_FLAG_PVP                   = 0x00001000,
    UNIT_FLAG_SILENCED              = 0x00002000,           // silenced, 2.1.1
    UNIT_FLAG_UNK_14                = 0x00004000,
    UNIT_FLAG_USE_SWIM_ANIMATION    = 0x00008000,
    UNIT_FLAG_NON_ATTACKABLE_2      = 0x00010000,           // removes attackable icon, if on yourself, cannot assist self but can cast TARGET_UNIT_CASTER spells - added by SPELL_AURA_MOD_UNATTACKABLE
    UNIT_FLAG_PACIFIED              = 0x00020000,
    UNIT_FLAG_STUNNED               = 0x00040000,           // Unit is a subject to stun, turn and strafe movement disabled
    UNIT_FLAG_IN_COMBAT             = 0x00080000,
    UNIT_FLAG_TAXI_FLIGHT           = 0x00100000,           // Unit is on taxi, paired with a duplicate loss of client control packet (likely a legacy serverside hack). Disables any spellcasts not allowed in taxi flight client-side.
    UNIT_FLAG_CONFUSED              = 0x00400000,           // Unit is a subject to confused movement, movement checks disabled, paired with loss of client control packet.
    UNIT_FLAG_FLEEING               = 0x00800000,           // Unit is a subject to fleeing movement, movement checks disabled, paired with loss of client control packet.
    UNIT_FLAG_POSSESSED             = 0x01000000,           // Unit is under remote control by another unit, movement checks disabled, paired with loss of client control packet. New master is allowed to use melee attack and can't select this unit via mouse in the world (as if it was own character).
    UNIT_FLAG_NOT_SELECTABLE        = 0x02000000,
    UNIT_FLAG_SKINNABLE             = 0x04000000,
    UNIT_FLAG_AURAS_VISIBLE         = 0x08000000,           // magic detect
    UNIT_FLAG_SHEATHE               = 0x40000000,
    //UNIT_FLAG_UNK_31              = 0x80000000            // no effect in 1.12.1
    UNIT_FLAG_NO_KILL_REWARD        = 0x80000000,           // Unit should yield no reward (Honor/XP/Rep) on kill

    // [-ZERO] TBC enumerations [?]
    UNIT_FLAG_NOT_ATTACKABLE_1      = 0x00000080,           // ?? (UNIT_FLAG_PLAYER_CONTROLLED | UNIT_FLAG_NOT_ATTACKABLE_1) is NON_PVP_ATTACKABLE
    UNIT_FLAG_LOOTING               = 0x00000400,           // loot animation
    UNIT_FLAG_PET_IN_COMBAT         = 0x00000800,           // in combat?, 2.0.8
    UNIT_FLAG_DISARMED              = 0x00200000,           // disable melee spells casting..., "Required melee weapon" added to melee spells tooltip.

    UNIT_FLAG_UNK_28                = 0x10000000,
    UNIT_FLAG_UNK_29                = 0x20000000,           // used in Feing Death spell
};

/// Non Player Character flags
enum NPCFlags
{
    UNIT_NPC_FLAG_NONE                  = 0x00000000,
    UNIT_NPC_FLAG_GOSSIP                = 0x00000001,       // 100%
    UNIT_NPC_FLAG_QUESTGIVER            = 0x00000002,       // 100%
    UNIT_NPC_FLAG_VENDOR                = 0x00000004,       // 100%
    UNIT_NPC_FLAG_FLIGHTMASTER          = 0x00000008,       // 100%
    UNIT_NPC_FLAG_TRAINER               = 0x00000010,       // 100%
    UNIT_NPC_FLAG_SPIRITHEALER          = 0x00000020,       // guessed
    UNIT_NPC_FLAG_SPIRITGUIDE           = 0x00000040,       // guessed
    UNIT_NPC_FLAG_INNKEEPER             = 0x00000080,       // 100%
    UNIT_NPC_FLAG_BANKER                = 0x00000100,       // 100%
    UNIT_NPC_FLAG_PETITIONER            = 0x00000200,       // 100% 0xC0000 = guild petitions
    UNIT_NPC_FLAG_TABARDDESIGNER        = 0x00000400,       // 100%
    UNIT_NPC_FLAG_BATTLEMASTER          = 0x00000800,       // 100%
    UNIT_NPC_FLAG_AUCTIONEER            = 0x00001000,       // 100%
    UNIT_NPC_FLAG_STABLEMASTER          = 0x00002000,       // 100%
    UNIT_NPC_FLAG_REPAIR                = 0x00004000,       // 100%
    UNIT_NPC_FLAG_OUTDOORPVP            = 0x20000000,       // custom flag for outdoor pvp creatures || Custom flag
};

enum AutoAttackCheckResult
{
    ATTACK_RESULT_OK = 0,
    ATTACK_RESULT_NOT_IN_RANGE = 1,
    ATTACK_RESULT_BAD_FACING = 2,
    ATTACK_RESULT_CANT_ATTACK = 3,
    ATTACK_RESULT_DEAD = 4,
    ATTACK_RESULT_FRIENDLY_TARGET = 5,
};

namespace Movement
{
    class MoveSpline;
}

/**
 * Structure to keep track of diminishing returns, for more information
 * about the idea behind diminishing returns, see: http://www.wowwiki.com/Diminishing_returns
 * \see Unit::GetDiminishing
 * \see Unit::IncrDiminishing
 * \see Unit::ApplyDiminishingToDuration
 * \see Unit::ApplyDiminishingAura
 */
struct DiminishingReturn
{
    DiminishingReturn(DiminishingGroup group, uint32 t, uint32 count)
        : DRGroup(group), stack(0), hitTime(t), hitCount(count)
    {}

    /**
     * Group that this diminishing return will affect
     */
    DiminishingGroup        DRGroup:16;
    /**
     * Seems to be how many times this has been stacked, modified in
     * Unit::ApplyDiminishingAura
     */
    uint16                  stack:16;
    /**
     * Records at what time the last hit with this DiminishingGroup was done, if it's
     * higher than 15 seconds (ie: 15 000 ms) the DiminishingReturn::hitCount will be reset
     * to DiminishingLevels::DIMINISHING_LEVEL_1, which will do no difference to the duration
     * of the stun etc.
     */
    uint32                  hitTime;
    /**
     * Records how many times a spell of this DiminishingGroup has hit, this in turn
     * decides how how long the duration of the stun etc is.
     */
    uint32                  hitCount;
};

// At least some values expected fixed and used in auras field, other custom
enum MeleeHitOutcome
{
    MELEE_HIT_EVADE     = 0,
    MELEE_HIT_MISS      = 1,
    MELEE_HIT_DODGE     = 2,                                // used as misc in SPELL_AURA_IGNORE_COMBAT_RESULT
    MELEE_HIT_BLOCK     = 3,                                // used as misc in SPELL_AURA_IGNORE_COMBAT_RESULT
    MELEE_HIT_PARRY     = 4,                                // used as misc in SPELL_AURA_IGNORE_COMBAT_RESULT
    MELEE_HIT_GLANCING  = 5,
    MELEE_HIT_CRIT      = 6,
    MELEE_HIT_CRUSHING  = 7,
    MELEE_HIT_NORMAL    = 8,
    MELEE_HIT_BLOCK_CRIT= 9,
};

struct CleanDamage
{
    CleanDamage(uint32 _damage, WeaponAttackType _attackType, MeleeHitOutcome _hitOutCome, uint32 _Absorb, int32 _Resist) :
    damage(_damage), attackType(_attackType), hitOutCome(_hitOutCome), absorb(_Absorb), resist(_Resist) {}

    uint32 damage;
    WeaponAttackType attackType;
    MeleeHitOutcome hitOutCome;
    uint32 absorb;
    int32 resist;
};

struct SubDamageInfo
{
    SpellSchoolMask damageSchoolMask = SPELL_SCHOOL_MASK_NORMAL;
    uint32 damage = 0;
    uint32 absorb = 0;
    int32 resist = 0;
};

// Struct for use in Unit::CalculateMeleeDamage
// Need create structure like in SMSG_ATTACKERSTATEUPDATE opcode
struct CalcDamageInfo
{
    Unit  *attacker = nullptr;             // Attacker
    Unit  *target = nullptr;               // Target for damage
    uint32 totalDamage = 0;
    uint32 totalAbsorb = 0;
    int32 totalResist = 0;
    SubDamageInfo subDamage[MAX_ITEM_PROTO_DAMAGES] = {};
    uint32 blocked_amount = 0;
    uint32 HitInfo = HITINFO_NORMALSWING;
    uint32 TargetState = VICTIMSTATE_UNAFFECTED;

    // Helper
    WeaponAttackType attackType = BASE_ATTACK;
    uint32 procAttacker = 0;
    uint32 procVictim = 0;
    uint32 procEx = 0;
    uint32 cleanDamage = 0;                        // Used only for rage calculation
    MeleeHitOutcome hitOutCome = MELEE_HIT_EVADE;  // TODO: remove this field (need use TargetState)
};

// Spell damage info structure based on structure sending in SMSG_SPELLNONMELEEDAMAGELOG opcode
struct SpellNonMeleeDamage{
    SpellNonMeleeDamage(Unit *_attacker, Unit *_target, uint32 _SpellID, SpellSchools _school)
        : target(_target), attacker(_attacker), SpellID(_SpellID), damage(0), school(_school),
        absorb(0), resist(0), periodicLog(false), unused(false), blocked(0), HitInfo(0), spell(nullptr)
    {}

    Unit   *target;
    Unit   *attacker;
    uint32 SpellID;
    uint32 damage;
    SpellSchools school;
    uint32 absorb;
    int32 resist;
    bool   periodicLog;
    bool   unused;
    uint32 blocked;
    uint32 HitInfo;
    Spell *spell;
};

struct SpellPeriodicAuraLogInfo
{
    SpellPeriodicAuraLogInfo(Aura *_aura, uint32 _damage, uint32 _absorb, int32 _resist, float _multiplier)
        : aura(_aura), damage(_damage), absorb(_absorb), resist(_resist), multiplier(_multiplier) {}

    Aura   *aura;
    uint32 damage;
    uint32 absorb;
    int32 resist;
    float  multiplier;
};

uint32 createProcExtendMask(SpellNonMeleeDamage *damageInfo, SpellMissInfo missCondition);

enum SpellAuraProcResult
{
    SPELL_AURA_PROC_OK              = 0,                    // proc was processed, will remove charges
    SPELL_AURA_PROC_FAILED          = 1,                    // proc failed - if at least one aura failed the proc, charges won't be taken
    SPELL_AURA_PROC_CANT_TRIGGER    = 2                     // aura can't trigger - skip charges taking, move to next aura if exists
};

typedef SpellAuraProcResult(Unit::*pAuraProcHandler)(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
extern pAuraProcHandler AuraProcHandler[TOTAL_AURAS];

enum CurrentSpellTypes
{
    CURRENT_MELEE_SPELL             = 0,
    CURRENT_GENERIC_SPELL           = 1,
    CURRENT_AUTOREPEAT_SPELL        = 2,
    CURRENT_CHANNELED_SPELL         = 3
};

#define CURRENT_FIRST_NON_MELEE_SPELL 1
#define CURRENT_MAX_SPELL             4
#define UNIT_SPELL_UPDATE_TIME_BUFFER 60

struct GlobalCooldown
{
    explicit GlobalCooldown(uint32 _dur = 0, uint32 _time = 0) : duration(_dur), cast_time(_time) {}

    uint32 duration;
    uint32 cast_time;
};

typedef std::unordered_map<uint32 /*category*/, GlobalCooldown> GlobalCooldownList;

class GlobalCooldownMgr                                     // Shared by Player and CharmInfo
{
    public:
        GlobalCooldownMgr() {}

    public:
        bool HasGlobalCooldown(SpellEntry const* spellInfo) const;
        void AddGlobalCooldown(SpellEntry const* spellInfo, uint32 gcd);
        void CancelGlobalCooldown(SpellEntry const* spellInfo);

    private:
        GlobalCooldownList m_GlobalCooldowns;
};

struct SpellCooldown
{
    time_t end;
    uint32 cat;
    time_t categoryEnd;
    uint16 itemid;
};

typedef std::map<uint32, SpellCooldown> SpellCooldowns;

enum ActiveStates
{
    ACT_PASSIVE  = 0x01,                                    // 0x01 - passive
    ACT_DISABLED = 0x81,                                    // 0x80 - castable
    ACT_ENABLED  = 0xC1,                                    // 0x40 | 0x80 - auto cast + castable
    ACT_COMMAND  = 0x07,                                    // 0x01 | 0x02 | 0x04
    ACT_REACTION = 0x06,                                    // 0x02 | 0x04
    ACT_DECIDE   = 0x00                                     // custom
};

enum ReactStates
{
    REACT_PASSIVE    = 0,
    REACT_DEFENSIVE  = 1,
    REACT_AGGRESSIVE = 2
};

enum CommandStates
{
    COMMAND_STAY    = 0,
    COMMAND_FOLLOW  = 1,
    COMMAND_ATTACK  = 2,
    COMMAND_DISMISS = 3
};

#define UNIT_ACTION_BUTTON_ACTION(X) (uint32(X) & 0x00FFFFFF)
#define UNIT_ACTION_BUTTON_TYPE(X)   ((uint32(X) & 0xFF000000) >> 24)
#define MAX_UNIT_ACTION_BUTTON_ACTION_VALUE (0x00FFFFFF+1)
#define MAKE_UNIT_ACTION_BUTTON(A,T) (uint32(A) | (uint32(T) << 24))

struct UnitActionBarEntry
{
    UnitActionBarEntry() : packedData(uint32(ACT_DISABLED) << 24) {}

    uint32 packedData;

    // helper
    ActiveStates GetType() const { return ActiveStates(UNIT_ACTION_BUTTON_TYPE(packedData)); }
    uint32 GetAction() const { return UNIT_ACTION_BUTTON_ACTION(packedData); }
    bool IsActionBarForSpell() const
    {
        ActiveStates Type = GetType();
        return Type == ACT_DISABLED || Type == ACT_ENABLED || Type == ACT_PASSIVE;
    }

    void SetActionAndType(uint32 action, ActiveStates type)
    {
        packedData = MAKE_UNIT_ACTION_BUTTON(action,type);
    }

    void SetType(ActiveStates type)
    {
        packedData = MAKE_UNIT_ACTION_BUTTON(UNIT_ACTION_BUTTON_ACTION(packedData),type);
    }

    void SetAction(uint32 action)
    {
        packedData = (packedData & 0xFF000000) | UNIT_ACTION_BUTTON_ACTION(action);
    }
};

typedef UnitActionBarEntry CharmSpellEntry;

enum ActionBarIndex
{
    ACTION_BAR_INDEX_START = 0,
    ACTION_BAR_INDEX_PET_SPELL_START = 3,
    ACTION_BAR_INDEX_PET_SPELL_END = 7,
    ACTION_BAR_INDEX_END = 10,
};

#define MAX_UNIT_ACTION_BAR_INDEX (ACTION_BAR_INDEX_END-ACTION_BAR_INDEX_START)

struct CharmInfo
{
    explicit CharmInfo(Unit* unit);
    uint32 GetPetNumber() const { return m_petnumber; }
    void SetPetNumber(uint32 petnumber, bool statwindow);

    void SetCommandState(CommandStates st) { m_CommandState = st; }
    CommandStates GetCommandState() const { return m_CommandState; }
    bool HasCommandState(CommandStates state) const { return m_CommandState == state; }
    void SetReactState(ReactStates st) { m_reactState = st; }
    ReactStates GetReactState() const { return m_reactState; }
    bool HasReactState(ReactStates state) const { return m_reactState == state; }

    FactionTemplateEntry const* GetOriginalFactionTemplate() const { return m_originalFactionTemplate; }
    void SetOriginalFactionTemplate(FactionTemplateEntry const* ft) { m_originalFactionTemplate = ft; }

    void InitPossessCreateSpells();
    void InitCharmCreateSpells();
    void InitPetActionBar();
    void InitEmptyActionBar();

                                                        //return true if successful
    bool AddSpellToActionBar(uint32 spellid, ActiveStates newstate = ACT_DECIDE);
    bool RemoveSpellFromActionBar(uint32 spell_id);
    void LoadPetActionBar(const std::string& data);
    void BuildActionBar(WorldPacket* data);
    void SetSpellAutocast(uint32 spell_id, bool state);
    void SetActionBar(uint8 index, uint32 spellOrAction,ActiveStates type)
    {
        PetActionBar[index].SetActionAndType(spellOrAction,type);
    }
    UnitActionBarEntry const* GetActionBarEntry(uint8 index) const { return &(PetActionBar[index]); }

    void ToggleCreatureAutocast(uint32 spellid, bool apply);

    CharmSpellEntry* GetCharmSpell(uint8 index) { return &(m_charmspells[index]); }

    void SetIsCommandAttack(bool val);
    bool IsCommandAttack();
    void SetIsCommandFollow(bool val);
    bool IsCommandFollow();
    void SetIsAtStay(bool val);
    bool IsAtStay();
    void SetIsFollowing(bool val);
    bool IsFollowing();
    void SetIsReturning(bool val);
    bool IsReturning();
    void SaveStayPosition();
    void GetStayPosition(float &x, float &y, float &z);
private:

    Unit* m_unit;
    FactionTemplateEntry const* m_originalFactionTemplate;

    UnitActionBarEntry PetActionBar[MAX_UNIT_ACTION_BAR_INDEX];
    CharmSpellEntry m_charmspells[CREATURE_MAX_SPELLS];
    CommandStates   m_CommandState;
    ReactStates     m_reactState;
    uint32          m_petnumber;

    bool _isCommandAttack;
    bool _isCommandFollow;
    bool _isAtStay;
    bool _isFollowing;
    bool _isReturning;
    float _stayX;
    float _stayY;
    float _stayZ;
};

// used in CallForAllControlledUnits/CheckAllControlledUnits
enum ControlledUnitMask
{
    CONTROLLED_PET       = 0x01,
    CONTROLLED_MINIPET   = 0x02,
    CONTROLLED_GUARDIANS = 0x04,                            // including PROTECTOR_PET
    CONTROLLED_CHARM     = 0x08,
    CONTROLLED_TOTEMS    = 0x10,
};

// for clearing special attacks
#define REACTIVE_TIMER_START 4000

enum ReactiveType
{
    REACTIVE_DEFENSE      = 1,
    REACTIVE_HUNTER_PARRY = 2,
    REACTIVE_CRIT         = 3,
    //REACTIVE_HUNTER_CRIT  = 4,
    REACTIVE_OVERPOWER    = 5
};

#define MAX_REACTIVE 6

typedef std::set<ObjectGuid> GuardianPetList;

// delay time next attack to prevent client attack animation problems
#define ATTACK_DISPLAY_DELAY 200

// Regeneration defines
#define REGEN_TIME_FULL     2000                            // For this time difference is computed regen value

struct SpellProcEventEntry;                                 // used only privately

struct ProhibitSpellInfo
{
    SpellSchoolMask SchoolMask;
    uint32 RestingMsTime;
};

// cf Unit::AddAura
enum AddAuraFlags
{
    ADD_AURA_NO_OPTION    = 0x00,
    ADD_AURA_POSITIVE     = 0x01,
    ADD_AURA_NEGATIVE     = 0x02,
    ADD_AURA_PASSIVE      = 0x04,
    ADD_AURA_PERMANENT    = 0x08,
};

#define DEBUG_UNIT(unit, flags, ...) do { if (unit->GetDebugFlags() & flags) unit->Debug(flags, __VA_ARGS__); } while (false)
#define DEBUG_UNIT_IF(cond, unit, flags, ...) do { if (unit->GetDebugFlags() & flags && cond) unit->Debug(flags, __VA_ARGS__); } while (false)

enum UnitDebugFlags
{
    DEBUG_SPELL_COMPUTE_RESISTS = 0x01,
    DEBUG_PACKETS_RECV          = 0x02,
    DEBUG_PACKETS_SEND          = 0x04,
    DEBUG_AI                    = 0x08,
    DEBUG_DR                    = 0x10,
    DEBUG_CHEAT                 = 0x20,
    DEBUG_PROCS                 = 0x40,
    DEBUG_SPELLS_DAMAGE         = 0x80,
};

struct ProcTriggeredData
{
    ProcTriggeredData(SpellProcEventEntry const * _spellProcEvent, SpellAuraHolder* _triggeredByHolder, Unit* _target, uint32 _procFlag)
        : spellProcEvent(_spellProcEvent), triggeredByHolder(_triggeredByHolder), target(_target), procFlag(_procFlag)
        {}
    SpellProcEventEntry const *spellProcEvent;
    SpellAuraHolder* triggeredByHolder;
    Unit* target;
    uint32 procFlag;
};

typedef std::list< ProcTriggeredData > ProcTriggeredList;

enum TeleportToOptions
{
    TELE_TO_GM_MODE             = 0x01,
    TELE_TO_NOT_LEAVE_TRANSPORT = 0x02,
    TELE_TO_NOT_LEAVE_COMBAT    = 0x04,
    TELE_TO_NOT_UNSUMMON_PET    = 0x08,
    TELE_TO_SPELL               = 0x10,
    TELE_TO_FORCE_MAP_CHANGE    = 0x20,
};

#define MELEE_Z_LIMIT 36                                    // vertical range diff limit for melee attacks (no meleeing units that fly too high overhead)

enum MovementModType
{
    MOV_MOD_FLEE_FOR_ASSISTANCE = 0,
    MOV_MOD_FLEE_IN_FEAR        = 1,
    MOV_MOD_CONFUSED            = 2
};

class MANGOS_DLL_SPEC Unit : public WorldObject
{
    public:
        static Unit* GetUnit(WorldObject &obj, uint64 const &Guid);

        typedef std::set<Unit*> AttackerSet;
        typedef std::multimap< uint32, SpellAuraHolder*> SpellAuraHolderMap;
        typedef std::pair<SpellAuraHolderMap::iterator, SpellAuraHolderMap::iterator> SpellAuraHolderBounds;
        typedef std::pair<SpellAuraHolderMap::const_iterator, SpellAuraHolderMap::const_iterator> SpellAuraHolderConstBounds;
        typedef std::list<SpellAuraHolder *> SpellAuraHolderList;
        typedef std::list<Aura *> AuraList;
        typedef std::list<DiminishingReturn> Diminishing;
        typedef std::set<uint32> ComboPointHolderSet;
        typedef std::map<SpellEntry const*, ObjectGuid> SingleCastSpellTargetMap;

        virtual ~Unit ( );

        void AddToWorld() override;
        void RemoveFromWorld() override;

        void CleanupsBeforeDelete() override;               // used in ~Creature/~Player (or before mass creature delete to remove cross-references to already deleted units)

        float GetObjectBoundingRadius() const override { return m_floatValues[UNIT_FIELD_BOUNDINGRADIUS]; } // overwrite WorldObject version
        float GetCombatReach() const override { return m_floatValues[UNIT_FIELD_COMBATREACH]; } // overwrite WorldObject version

        /**
         * Gets the current DiminishingLevels for the given group
         * @param group The group that you would like to know the current diminishing return level for
         * @return The current diminishing level, up to DIMINISHING_LEVEL_IMMUNE
         */
        DiminishingLevels GetDiminishing(DiminishingGroup  group);
        /**
         * Increases the level of the DiminishingGroup by one level up until
         * DIMINISHING_LEVEL_IMMUNE where the target becomes immune to spells of
         * that DiminishingGroup
         * @param group The group to increase the level for by one
         */
        void IncrDiminishing(DiminishingGroup group);
        /**
         * Calculates how long the duration of a spell should be considering
         * diminishing returns, ie, if the Level passed in is DIMINISHING_LEVEL_IMMUNE
         * then the duration will be zeroed out. If it is DIMINISHING_LEVEL_1 then a full
         * duration will be used
         * @param group The group to affect
         * @param duration The duration to be changed, will be updated with the new duration
         * @param caster Who's casting the spell, used to decide whether anything should be calculated
         * @param Level The current level of diminishing returns for the group, decides the new duration
         * @param isReflected Whether the spell was reflected or not, used to determine if we should do any calculations at all.
         */
        void ApplyDiminishingToDuration(DiminishingGroup  group, int32 &duration,Unit* caster, DiminishingLevels Level, bool isReflected = false);
        /**
         * Applies a diminishing return to the given group if apply is true,
         * otherwise lowers the level by one (?)
         * @param group The group to affect
         * @param apply whether this aura is being added/removed
         */
        void ApplyDiminishingAura(DiminishingGroup  group, bool apply);
        /**
         * Clears all the current diminishing returns for this Unit.
         */
        void ClearDiminishings() { m_Diminishing.clear(); }

        void Update(uint32 update_diff, uint32 time) override;

        /**
         * Updates the attack time for the given WeaponAttackType
         * @param type The type of weapon that we want to update the time for
         * @param time the remaining time until we can attack with the WeaponAttackType again
         */
        void setAttackTimer(WeaponAttackType type, uint32 time) { m_attackTimer[type] = time; }
        /**
         * Resets the attack timer to the base value decided by Unit::m_modAttackSpeedPct and
         * Unit::GetAttackTime
         * @param type The weapon attack type to reset the attack timer for.
         */
        void resetAttackTimer(WeaponAttackType type = BASE_ATTACK);
        /**
         * Get's the remaining time until we can do an attack
         * @param type The weapon type to check the remaining time for
         * @return The remaining time until we can attack with this weapon type.
         */
        uint32 getAttackTimer(WeaponAttackType type) const { return m_attackTimer[type]; }
        /**
         * Checks whether the unit can do an attack. Does this by checking the attacktimer for the
         * WeaponAttackType, can probably be thought of as a cooldown for each swing/shot
         * @param type What weapon should we check for
         * @return true if the Unit::m_attackTimer is zero for the given WeaponAttackType
         */
        bool isAttackReady(WeaponAttackType type = BASE_ATTACK) const;
        /**
         * Checks if the current Unit has an offhand weapon
         * @return True if there is a offhand weapon.
         */
        bool haveOffhandWeapon() const;
        /**
         * Does an attack if any of the timers allow it and resets them, if the user
         * isn't in range or behind the target an error is sent to the client.
         * Also makes sure to not make and offhand and mainhand attack at the same
         * time. Only handles non-spells ie melee attacks.
         * @return True if an attack was made and no error happened, false otherwise
         */
        bool UpdateMeleeAttackingState();
        /**
         * Delays both main and off hand attacks by 100 ms if they are ready.
         * Called from UpdateMeleeAttackingState if attack can't happen now.
         */
        void DelayAutoAttacks();
        /**
         * Checks that need to be done before an auto attack swing happens.
         * Target's faction is only checked for players since its done elsewhere
         * for creatures and there is no need to check it on attack for them.
         */
        virtual AutoAttackCheckResult CanAutoAttackTarget(Unit const*) const;
        /**
         * Check is a given equipped weapon can be used, ie the mainhand, offhand etc.
         * @param attackType The attack type to check, ie: main/offhand/ranged
         * @return True if the weapon can be used, true except for shapeshifts and if disarmed.
         */
        bool CanUseEquippedWeapon(WeaponAttackType attackType) const
        {
            if (IsInFeralForm())
                return false;

            switch(attackType)
            {
                default:
                case BASE_ATTACK:
                    return !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED);
                case OFF_ATTACK:
                case RANGED_ATTACK:
                    return true;
            }
        }

        // Extra attacks methods
        void ResetExtraAttacks() { m_extraAttacks = 0; }
        void DropExtraAttack() { --m_extraAttacks; }
        void AddExtraAttack() { ++m_extraAttacks; }
        void SetExtraAttaks(uint32 attacks) { m_extraAttacks = attacks; }
        uint32 GetExtraAttacks() const { return m_extraAttacks; }
        bool IsExtraAttacksLocked() const { return m_extraMute; }
        void ExtraAttacksLocked(bool mute) { m_extraMute = mute; }

        void _addAttacker(Unit *pAttacker)                  // (Internal Use) must be called only from Unit::Attack(Unit*)
        {
            AttackerSet::const_iterator itr = m_attackers.find(pAttacker);
            if(itr == m_attackers.end())
                m_attackers.insert(pAttacker);
        }
        void _removeAttacker(Unit *pAttacker)               // (Internal Use) must be called only from Unit::AttackStop()
        {
            m_attackers.erase(pAttacker);
        }
        Unit * getAttackerForHelper() const                 // Return a possible enemy from this unit to help in combat
        {
            if (getVictim() != nullptr)
                return getVictim();

            if (!m_attackers.empty())
                return *(m_attackers.begin());

            return nullptr;
        }

        /**
         * Tries to attack a Unit/Player, also makes sure to stop attacking the current target
         * if we're already attacking someone.
         * @param victim The Unit to attack
         * @param meleeAttack Whether we should attack with melee or ranged/magic
         * @return True if an attack was initiated, false otherwise
         */
        bool Attack(Unit *victim, bool meleeAttack);
        /**
         * Called when we are attacked by someone in someway, might be when a fear runs out and
         * we want to notify AI to attack again or when a spell hits.
         * @param attacker Who's attacking us
         */
        void AttackedBy(Unit* attacker);
        /**
         * Stop all spells from casting except the one give by except_spellid
         * @param except_spellid This spell id will not be stopped from casting, defaults to 0
         * \see Unit::InterruptSpell
         */
        void CastStop(uint32 except_spellid = 0);
        /**
         * Stops attacking whatever we are attacking at the moment and tells the Unit we are attacking
         * that we are not doing that anymore.
         * @param targetSwitch if we are switching targets or not, defaults to false
         * @return false if we weren't attacking already, true otherwise
         * \see Unit::m_attacking
         */
        bool AttackStop(bool targetSwitch = false);
        /**
         * Removes all attackers from the Unit::m_attackers set and logs it if someone that
         * wasn't attacking it was in the list. Does this check by checking if Unit::AttackStop()
         * returned false.
         * \see Unit::AttackStop
         */
        void RemoveAllAttackers();

        // Returns the Unit::m_attackers, that stores the units that are attacking you
        AttackerSet const& getAttackers() const { return m_attackers; }

        // Returns the victim that this unit is currently attacking
        Unit* getVictim() const { return m_attacking; }

        // Stop this unit from combat, if includingCast==true, also interrupt casting
        void CombatStop(bool includingCast = false);
        void CombatStopWithPets(bool includingCast = false);
        void StopAttackFaction(uint32 faction_id);
        Unit* SelectNearestTarget(float dist) const;
        Unit* SelectRandomUnfriendlyTarget(Unit* except = nullptr, float radius = ATTACK_DISTANCE, bool inFront = false, bool isValidAttackTarget = false) const;
        Unit* SelectRandomFriendlyTarget(Unit* except = nullptr, float radius = ATTACK_DISTANCE, bool inCombat = false) const;
        Player* FindNearestHostilePlayer(float range) const;
        Player* FindNearestFriendlyPlayer(float range) const;
        Unit* FindLowestHpFriendlyUnit(float fRange, uint32 uiMinHPDiff = 1, bool bPercent = false, Unit* except = nullptr) const;
        Unit* FindFriendlyUnitMissingBuff(float range, uint32 spellid, Unit* except = nullptr) const;
        Unit* FindFriendlyUnitCC(float range) const;
        Unit* SummonCreatureAndAttack(uint32 creatureEntry, Unit* pVictim= nullptr);
        bool IsSecondaryThreatTarget();
        bool hasNegativeAuraWithInterruptFlag(uint32 flag);
        void SendMeleeAttackStop(Unit* victim);
        void SendMeleeAttackStart(Unit* pVictim);

        void addUnitState(uint32 f) { m_state |= f; }
        bool hasUnitState(uint32 f) const { return m_state & f; }
        void clearUnitState(uint32 f) { m_state &= ~f; }
        uint32 getUnitState() const { return m_state; }
        bool CanFreeMove() const
        {
            return !hasUnitState(UNIT_STAT_NO_FREE_MOVE) && !GetOwnerGuid();
        }

        uint32 getLevel() const { return GetUInt32Value(UNIT_FIELD_LEVEL); }
        virtual uint32 GetLevelForTarget(Unit const* /*target*/) const { return getLevel(); }
        void SetLevel(uint32 lvl);
        uint8 getRace() const { return GetByteValue(UNIT_FIELD_BYTES_0, 0); }
        uint32 getRaceMask() const { return getRace() ? 1 << (getRace()-1) : 0x0; }
        uint8 getClass() const { return GetByteValue(UNIT_FIELD_BYTES_0, 1); }
        uint32 getClassMask() const { return getClass() ? 1 << (getClass()-1) : 0x0; }
        uint8 getGender() const override { return GetByteValue(UNIT_FIELD_BYTES_0, 2); }

        /**
         * @brief Inits display id to player display ID, depending on race, class, gender
         */
        void InitPlayerDisplayIds();

        float GetStat(Stats stat) const { return float(GetUInt32Value(UNIT_FIELD_STAT0+stat)); }
        void SetStat(Stats stat, int32 val) { SetStatInt32Value(UNIT_FIELD_STAT0+stat, val); }

        inline int32 GetArmor() const { return GetResistance(SPELL_SCHOOL_NORMAL); }
        inline void SetArmor(int32 val) { SetStatInt32Value(UNIT_FIELD_RESISTANCES, val); }

        inline int32 GetResistance(SpellSchools school) const { return GetInt32Value(UNIT_FIELD_RESISTANCES + school); }
        inline void SetResistance(SpellSchools school, int32 val) { SetInt32Value(UNIT_FIELD_RESISTANCES + school, val); }

        uint32 GetHealth()    const { return GetUInt32Value(UNIT_FIELD_HEALTH); }
        uint32 GetMaxHealth() const { return GetUInt32Value(UNIT_FIELD_MAXHEALTH); }
        float GetHealthPercent() const { return (GetHealth()*100.0f) / GetMaxHealth(); }
        void SetHealth(   uint32 val);
        void SetMaxHealth(uint32 val);
        void SetHealthPercent(float percent);
        int32 ModifyHealth(int32 val);

        bool IsFullHealth() const { return GetHealth() == GetMaxHealth(); }
        bool HealthBelowPct(int32 pct) const { return GetHealth() * 100 < GetMaxHealth() * pct; }
        bool HealthBelowPctDamaged(int32 pct, uint32 damage) const { return (int32(GetHealth()) - damage) * 100 < GetMaxHealth() * pct; }
        bool HealthAbovePct(int32 pct) const { return GetHealth() * 100 > GetMaxHealth() * pct; }
        uint32 CountPctFromMaxHealth(int32 pct) const { return uint32(float(pct) * GetMaxHealth() / 100.0f); }
        void SetFullHealth() { SetHealth(GetMaxHealth()); }

        Powers getPowerType() const { return Powers(GetByteValue(UNIT_FIELD_BYTES_0, 3)); }
        void setPowerType(Powers power);
        void SetInitCreaturePowerType();
        uint32 GetPower(   Powers power) const { return GetUInt32Value(UNIT_FIELD_POWER1   +power); }
        uint32 GetMaxPower(Powers power) const { return GetUInt32Value(UNIT_FIELD_MAXPOWER1+power); }
        float GetPowerPercent(Powers power) const { return GetMaxPower(power) ? ((GetPower(power)*100.0f) / GetMaxPower(power)) : 100.0f; }
        void SetPower(   Powers power, uint32 val);
        void SetMaxPower(Powers power, uint32 val);
        void SetPowerPercent(Powers power, float percent);
        int32 ModifyPower(Powers power, int32 val);
        void ApplyPowerMod(Powers power, uint32 val, bool apply);
        void ApplyMaxPowerMod(Powers power, uint32 val, bool apply);

        uint32 GetAttackTime(WeaponAttackType att) const { return (uint32)(GetFloatValue(UNIT_FIELD_BASEATTACKTIME+att)/m_modAttackSpeedPct[att]); }
        void SetAttackTime(WeaponAttackType att, uint32 val, bool resetAttTime = true)
        {
            SetFloatValue(UNIT_FIELD_BASEATTACKTIME+att,val*m_modAttackSpeedPct[att]);
            if (resetAttTime)
                resetAttackTimer(att);
        }
        void ApplyAttackTimePercentMod(WeaponAttackType att,float val, bool apply, bool recalcDamage = false);
        void ApplyCastTimePercentMod(float val, bool apply);

        SheathState GetSheath() const { return SheathState(GetByteValue(UNIT_FIELD_BYTES_2, 0)); }
        virtual void SetSheath( SheathState sheathed ) { SetByteValue(UNIT_FIELD_BYTES_2, 0, sheathed); }

        // faction template id
        uint32 getFaction() const { return GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE); }
        void setFaction(uint32 faction) { SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, faction ); }
        FactionTemplateEntry const* getFactionTemplateEntry() const;

        ReputationRank GetReactionTo(Unit const* target) const;
        ReputationRank static GetFactionReactionTo(FactionTemplateEntry const* factionTemplateEntry, Unit const* target);

        bool IsHostileTo(Unit const* unit) const override;
        bool IsHostileToPlayers() const;
        bool IsFriendlyTo(Unit const* unit) const override;
        bool IsNeutralToAll() const;
        bool IsContestedGuard() const
        {
            if(FactionTemplateEntry const* entry = getFactionTemplateEntry())
                return entry->IsContestedGuardFaction();

            return false;
        }
        bool IsPvP() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP); }
        void SetPvP(bool state);
        bool IsPvPContested() const;
        void SetPvPContested(bool state);

        uint32 GetCreatureType() const;
        uint32 GetCreatureTypeMask() const
        {
            uint32 creatureType = GetCreatureType();
            return creatureType ? (1 << (creatureType - 1)) : 0;
        }

        uint8 getStandState() const { return GetByteValue(UNIT_FIELD_BYTES_1, 0); }
        bool IsSittingDown() const;
        bool IsStandingUp() const;
        virtual bool IsStandingUpForProc() const; // takes not yet applied stand state change into account (for players)
        void SetStandState(uint8 state);

        bool IsMounted() const { return (GetMountID() != 0); }
        uint32 GetMountID() const { return GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID); }
        virtual void Mount(uint32 mount, uint32 spellId = 0);
        virtual void Unmount(bool from_aura = false);

        // Tuer cette unite.
        void DoKillUnit(Unit *victim = nullptr);

        uint16 GetSkillMaxForLevel(Unit const* target = nullptr) const { return (target ? GetLevelForTarget(target) : getLevel()) * 5; }
        void DealDamageMods(Unit *pVictim, uint32 &damage, uint32* absorb);
        uint32 DealDamage(Unit *pVictim, uint32 damage, CleanDamage const* cleanDamage, DamageEffectType damagetype, SpellSchoolMask damageSchoolMask, SpellEntry const *spellProto, bool durabilityLoss, Spell* spell = nullptr);
        // Je viens de tuer quelqu'un.
        void Kill(Unit* pVictim, SpellEntry const *spellProto, bool durabilityLoss = true);
        int32 DealHeal(Unit *pVictim, uint32 addhealth, SpellEntry const *spellProto, bool critical = false);

        void PetOwnerKilledUnit(Unit* pVictim);

        void ProcDamageAndSpell(Unit *pVictim, uint32 procAttacker, uint32 procVictim, uint32 procEx, uint32 amount, WeaponAttackType attType = BASE_ATTACK, SpellEntry const *procSpell = nullptr, Spell* spell = nullptr);
        void ProcDamageAndSpellFor(bool isVictim, Unit * pTarget, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, SpellEntry const* procSpell, uint32 damage, ProcTriggeredList& triggeredList, Spell* spell = nullptr);
        void HandleTriggers(Unit *pVictim, uint32 procExtra, uint32 amount, SpellEntry const *procSpell, ProcTriggeredList const& procTriggered);

        void HandleEmote(uint32 emote_id);                  // auto-select command/state
        void HandleEmoteCommand(uint32 emote_id);
        void HandleEmoteState(uint32 emote_id);
        void AttackerStateUpdate (Unit *pVictim, WeaponAttackType attType = BASE_ATTACK, bool checkLoS = true, bool extra = false );

        float MeleeMissChanceCalc(const Unit *pVictim, WeaponAttackType attType) const;
        void CalculateMeleeDamage(Unit *pVictim, uint32 damage, CalcDamageInfo *damageInfo, WeaponAttackType attackType = BASE_ATTACK);
        void DealMeleeDamage(CalcDamageInfo *damageInfo, bool durabilityLoss);

        void CalculateSpellDamage(SpellNonMeleeDamage *damageInfo, int32 damage, SpellEntry const *spellInfo, WeaponAttackType attackType = BASE_ATTACK, Spell* spell = nullptr);
        void DealSpellDamage(SpellNonMeleeDamage *damageInfo, bool durabilityLoss);

        float  MeleeSpellMissChance(Unit *pVictim, WeaponAttackType attType, int32 skillDiff, SpellEntry const *spell, Spell* spellPtr = nullptr);
        SpellMissInfo MeleeSpellHitResult(Unit *pVictim, SpellEntry const *spell, Spell* spellPtr = nullptr);
        SpellMissInfo MagicSpellHitResult(Unit *pVictim, SpellEntry const *spell, Spell* spellPtr = nullptr);
        int32 MagicSpellHitChance(Unit *pVictim, SpellEntry const *spell, Spell* spellPtr = nullptr);
        SpellMissInfo SpellHitResult(Unit *pVictim, SpellEntry const *spell, SpellEffectIndex effIndex, bool canReflect = false, Spell* spellPtr = nullptr);
        // Nostalrius : SPELL_AURA_MOD_MECHANIC_RESISTANCE
        bool IsEffectResist(SpellEntry const* spell, int eff);

        float GetUnitDodgeChance()    const;
        float GetUnitParryChance()    const;
        float GetUnitBlockChance()    const;
        float GetUnitCriticalChance(WeaponAttackType attackType, const Unit *pVictim) const;

        virtual uint32 GetShieldBlockValue() const =0;
        uint32 GetUnitMeleeSkill(Unit const* target = nullptr) const { return (target ? GetLevelForTarget(target) : getLevel()) * 5; }
        uint32 GetDefenseSkillValue(Unit const* target = nullptr) const;
        uint32 GetWeaponSkillValue(WeaponAttackType attType, Unit const* target = nullptr) const;
        float GetPPMProcChance(uint32 WeaponSpeed, float PPM) const;

        MeleeHitOutcome RollMeleeOutcomeAgainst (const Unit *pVictim, WeaponAttackType attType) const;
        MeleeHitOutcome RollMeleeOutcomeAgainst (const Unit *pVictim, WeaponAttackType attType, int32 crit_chance, int32 miss_chance, int32 dodge_chance, int32 parry_chance, int32 block_chance, bool SpellCasted ) const;

        bool isVendor()       const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR ); }
        bool isTrainer()      const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER ); }
        bool isQuestGiver()   const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER ); }
        bool isGossip()       const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP ); }
        bool isTaxi()         const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_FLIGHTMASTER ); }
        bool isGuildMaster()  const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_PETITIONER ); }
        bool isBattleMaster() const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BATTLEMASTER ); }
        bool isBanker()       const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BANKER ); }
        bool isInnkeeper()    const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_INNKEEPER ); }
        bool isSpiritHealer() const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER ); }
        bool isSpiritGuide()  const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITGUIDE ); }
        bool isTabardDesigner()const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TABARDDESIGNER ); }
        bool isAuctioner()    const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_AUCTIONEER ); }
        bool isArmorer()      const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_REPAIR ); }
        bool isServiceProvider() const
        {
            return HasFlag( UNIT_NPC_FLAGS,
                UNIT_NPC_FLAG_VENDOR | UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_FLIGHTMASTER |
                UNIT_NPC_FLAG_PETITIONER | UNIT_NPC_FLAG_BATTLEMASTER | UNIT_NPC_FLAG_BANKER |
                UNIT_NPC_FLAG_INNKEEPER | UNIT_NPC_FLAG_SPIRITHEALER |
                UNIT_NPC_FLAG_SPIRITGUIDE | UNIT_NPC_FLAG_TABARDDESIGNER | UNIT_NPC_FLAG_AUCTIONEER );
        }
        bool isSpiritService() const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER | UNIT_NPC_FLAG_SPIRITGUIDE ); }

        bool IsTaxiFlying() const { return hasUnitState(UNIT_STAT_TAXI_FLIGHT); }

        bool isInCombat() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT); }
        void SetInCombatState(bool PvP, Unit* enemy = nullptr);
        void SetInCombatWith(Unit* enemy);
        void ClearInCombat();
        void SetInCombatWithAssisted(Unit* assisted);
        void SetInCombatWithAggressor(Unit* aggressor, bool touchOnly = false);
        inline void SetOutOfCombatWithAggressor(Unit* aggressor) { SetInCombatWithAggressor(aggressor, true); }
        void SetInCombatWithVictim(Unit* victim, bool touchOnly = false);
        inline void SetOutOfCombatWithVictim(Unit* victim) { SetInCombatWithVictim(victim, true); }
        void TogglePlayerPvPFlagOnAttackVictim(Unit const* pVictim, bool touchOnly = false);
        uint32 GetCombatTimer() const { return m_CombatTimer; }
        void SetCombatTimer(uint32 t) { m_CombatTimer = t; }

        bool HasAuraTypeByCaster(AuraType auraType, ObjectGuid casterGuid) const;
        uint32 GetFirstAuraBySpellIconAndVisual(uint32 spellIconId, uint32 spellVisual) const;

        uint64 GetAuraApplicationMask() const;
        uint64 GetNegativeAuraApplicationMask() const;

        SpellAuraHolderBounds GetSpellAuraHolderBounds(uint32 spell_id)
        {
            return m_spellAuraHolders.equal_range(spell_id);
        }
        SpellAuraHolderConstBounds GetSpellAuraHolderBounds(uint32 spell_id) const
        {
            return m_spellAuraHolders.equal_range(spell_id);
        }

        bool HasAuraType(AuraType auraType) const;
        bool HasAffectedAura(AuraType auraType, SpellEntry const* spellProto) const;
        bool HasAura(uint32 spellId, SpellEffectIndex effIndex) const;
        bool HasAura(uint32 spellId) const
        {
            return m_spellAuraHolders.find(spellId) != m_spellAuraHolders.end();
        }

        bool virtual HasSpell(uint32 /*spellID*/) const { return false; }

        bool HasStealthAura()      const { return HasAuraType(SPELL_AURA_MOD_STEALTH); }
        bool HasInvisibilityAura() const { return HasAuraType(SPELL_AURA_MOD_INVISIBILITY); }
        bool isFeared()  const { return HasAuraType(SPELL_AURA_MOD_FEAR); }
        bool isInRoots() const { return HasAuraType(SPELL_AURA_MOD_ROOT); }
        bool IsPolymorphed() const;
        bool IsImmuneToSchoolMask(uint32 schoolMask) const;

        bool isFrozen() const;

        void RemoveSpellbyDamageTaken(AuraType auraType, uint32 damage);
        void RemoveFearEffectsByDamageTaken(uint32 damage, uint32 exceptSpellId, DamageEffectType damagetype);

        bool IsValidAttackTarget(Unit const* target) const;
        bool isTargetableForAttack(bool inversAlive = false, bool isAttackerPlayer = false) const;
        bool isAttackableByAOE(bool requireDeadTarget = false, bool isCasterPlayer = false) const;
        bool isPassiveToHostile() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE); }

        virtual bool IsInWater() const;
        virtual bool IsUnderWater() const;
        bool isInAccessablePlaceFor(Creature const* c) const;

        void SendHealSpellLog(Unit *pVictim, uint32 SpellID, uint32 Damage, bool critical = false);
        void SendEnergizeSpellLog(Unit *pVictim, uint32 SpellID, uint32 Damage,Powers powertype);
        void EnergizeBySpell(Unit *pVictim, uint32 SpellID, uint32 Damage, Powers powertype);
        void SendEnvironmentalDamageLog(uint8 type, uint32 damage, uint32 absorb, int32 resist) const;
        uint32 SpellNonMeleeDamageLog(Unit *pVictim, uint32 spellID, uint32 damage);
        void CastSpell(Unit* Victim, uint32 spellId, bool triggered, Item *castItem = nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr, SpellEntry const* triggeredByParent = nullptr);
        void CastSpell(Unit* Victim,SpellEntry const *spellInfo, bool triggered, Item *castItem= nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr, SpellEntry const* triggeredByParent = nullptr);
        void CastCustomSpell(Unit* Victim, uint32 spellId, int32 const* bp0, int32 const* bp1, int32 const* bp2, bool triggered, Item *castItem= nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr);
        void CastCustomSpell(Unit* Victim,SpellEntry const *spellInfo, int32 const* bp0, int32 const* bp1, int32 const* bp2, bool triggered, Item *castItem= nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr);
        void CastSpell(float x, float y, float z, uint32 spellId, bool triggered, Item *castItem = nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr);
        void CastSpell(float x, float y, float z, SpellEntry const *spellInfo, bool triggered, Item *castItem = nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr);
        // Affiche le visuel d'un sort
        void SendSpellGo(Unit* target, uint32 spellId);

        void DeMorph();

        void SendAttackStateUpdate(CalcDamageInfo *damageInfo);
        void SendAttackStateUpdate(uint32 HitInfo, Unit *target, uint8 SwingType, SpellSchoolMask damageSchoolMask, uint32 Damage, uint32 AbsorbDamage, int32 Resist, VictimState TargetState, uint32 BlockedAmount);
        void SendSpellNonMeleeDamageLog(SpellNonMeleeDamage *log);
        void SendSpellNonMeleeDamageLog(Unit *target, uint32 spellID, uint32 damage, SpellSchoolMask damageSchoolMask, uint32 absorbedDamage, int32 resist, bool isPeriodic, uint32 blocked, bool criticalHit = false, bool split = false);
        void SendPeriodicAuraLog(SpellPeriodicAuraLogInfo *pInfo, AuraType auraTypeOverride = SPELL_AURA_NONE);
        void SendSpellMiss(Unit *target, uint32 spellID, SpellMissInfo missInfo);

        void NearTeleportTo(float x, float y, float z, float orientation, uint32 teleportOptions = TELE_TO_NOT_LEAVE_TRANSPORT | TELE_TO_NOT_LEAVE_COMBAT | TELE_TO_NOT_UNSUMMON_PET);
        void NearLandTo(float x, float y, float z, float orientation);
        void TeleportPositionRelocation(float x, float y, float z, float o);
        void MonsterMoveWithSpeed(float x, float y, float z, float o, float speed, uint32 options);
        void MonsterMove(float x, float y, float z); // Utilise vitesse de course

        // recommend use MonsterMove/MonsterMoveWithSpeed for most case that correctly work with movegens
        // if used additional args in ... part then floats must explicitly casted to double
        void SendHeartBeat(bool includingSelf = true);
        virtual void SetFly(bool enable);
        void SetWalk(bool enable, bool asDefault = true);

    private:
        // when a player controls this unit, and when change is made to this unit which requires an ack from the client to be acted (change of speed for example), this movementCounter is incremented
        uint32 m_movementCounter = 0;
        std::deque<PlayerMovementPendingChange> m_pendingMovementChanges;

    public:
        void SetRooted(bool apply);
        void SetRootedReal(bool apply);
        bool IsRooted() const { return m_movementInfo.HasMovementFlag(MOVEFLAG_ROOT); }

        void SetWaterWalking(bool apply);
        void SetWaterWalkingReal(bool apply);
        bool IsWaterWalking() const { return m_movementInfo.HasMovementFlag(MOVEFLAG_WATERWALKING); }

        void SetHover(bool apply);
        void SetHoverReal(bool apply);
        bool IsHovering() const { return m_movementInfo.HasMovementFlag(MOVEFLAG_HOVER); }

        void SetFeatherFall(bool apply);
        void SetFeatherFallReal(bool apply);
        bool IsFallingSlow() const { return m_movementInfo.HasMovementFlag(MOVEFLAG_SAFE_FALL); }

        void SetLevitate(bool apply);
        bool IsLevitating() const { return m_movementInfo.HasMovementFlag(MOVEFLAG_LEVITATING); }

        void KnockBackFrom(WorldObject* target, float horizontalSpeed, float verticalSpeed);
        void KnockBack(float angle, float horizontalSpeed, float verticalSpeed);

        // reflects direct client control (examples: a player MC another player or a creature (possess effects). etc...)
        bool IsMovedByPlayer() const;
        Player* GetPlayerMovingMe();

        uint32 GetMovementCounterAndInc() { return m_movementCounter++; }
        uint32 GetMovementCounter() const { return m_movementCounter; }

        PlayerMovementPendingChange PopPendingMovementChange();
        void PushPendingMovementChange(PlayerMovementPendingChange newChange);
        bool HasPendingMovementChange() const { return !m_pendingMovementChanges.empty(); }
        bool HasPendingMovementChange(MovementChangeType changeType) const;

        void SetSpeedRate(UnitMoveType mtype, float rate);
        void SetSpeedRateReal(UnitMoveType mtype, float rate);
        void  UpdateSpeed(UnitMoveType mtype, bool forced, float ratio = 1.0f);
        float GetSpeed(UnitMoveType mtype) const;
        float GetXZFlagBasedSpeed() const;
        float GetXZFlagBasedSpeed(uint32 moveFlags) const;
        float GetSpeedRate(UnitMoveType mtype) const { return m_speed_rate[mtype]; }

        virtual bool CanWalk() const = 0;
        virtual bool CanFly() const = 0;
        virtual bool CanSwim() const = 0;
        virtual bool CanBeDetected() const { return true; }

        bool isAlive() const { return m_deathState == ALIVE; }
        bool isDead() const { return m_deathState == DEAD || m_deathState == CORPSE; }
        DeathState getDeathState() const { return m_deathState; }
        virtual void SetDeathState(DeathState s);           // overwritten in Creature/Player/Pet

        ObjectGuid const& GetOwnerGuid() const { return  GetGuidValue(UNIT_FIELD_SUMMONEDBY); }
        void SetOwnerGuid(ObjectGuid owner) { SetGuidValue(UNIT_FIELD_SUMMONEDBY, owner); ForceValuesUpdateAtIndex(UNIT_FIELD_HEALTH); ForceValuesUpdateAtIndex(UNIT_FIELD_MAXHEALTH); }
        ObjectGuid const& GetCreatorGuid() const { return GetGuidValue(UNIT_FIELD_CREATEDBY); }
        void SetCreatorGuid(ObjectGuid creator) { SetGuidValue(UNIT_FIELD_CREATEDBY, creator); }
        ObjectGuid const& GetPetGuid() const { return GetGuidValue(UNIT_FIELD_SUMMON); }
        void SetPetGuid(ObjectGuid pet) { SetGuidValue(UNIT_FIELD_SUMMON, pet); }
        ObjectGuid const& GetCharmerGuid() const { return GetGuidValue(UNIT_FIELD_CHARMEDBY); }
        void SetCharmerGuid(ObjectGuid owner) { SetGuidValue(UNIT_FIELD_CHARMEDBY, owner); ForceValuesUpdateAtIndex(UNIT_FIELD_HEALTH); ForceValuesUpdateAtIndex(UNIT_FIELD_MAXHEALTH); }
        ObjectGuid const& GetCharmGuid() const { return GetGuidValue(UNIT_FIELD_CHARM); }
        void SetCharmGuid(ObjectGuid charm) { SetGuidValue(UNIT_FIELD_CHARM, charm); }
        ObjectGuid const& GetTargetGuid() const { return GetGuidValue(UNIT_FIELD_TARGET); }
        void SetTargetGuid(ObjectGuid targetGuid) { SetGuidValue(UNIT_FIELD_TARGET, targetGuid); }
        void ClearTarget() { SetTargetGuid(ObjectGuid()); }
        ObjectGuid const& GetChannelObjectGuid() const { return GetGuidValue(UNIT_FIELD_CHANNEL_OBJECT); }
        void SetChannelObjectGuid(ObjectGuid targetGuid) { SetGuidValue(UNIT_FIELD_CHANNEL_OBJECT, targetGuid); }

        ObjectGuid const& GetPossessorGuid() const { return m_possessorGuid; }
        void SetPossessorGuid(ObjectGuid possession) { m_possessorGuid = possession; }

        virtual Pet* GetMiniPet() const { return nullptr; }    // overwrited in Player

        ObjectGuid const& GetCharmerOrOwnerGuid() const { return GetCharmerGuid() ? GetCharmerGuid() : GetOwnerGuid(); }
        ObjectGuid const& GetCharmerOrOwnerOrOwnGuid() const
        {
            if (ObjectGuid const& guid = GetCharmerOrOwnerGuid())
                return guid;
            return GetObjectGuid();
        }
        bool isCharmedOwnedByPlayerOrPlayer() const { return GetCharmerOrOwnerOrOwnGuid().IsPlayer(); }

        Player* GetSpellModOwner() const;
        Unit* GetOwner() const;
        Pet* GetPet() const;
        Unit* GetCharmer() const;
        Player* GetPossessor() const;
        Unit* GetCharm() const;
        void Uncharm();
        void RemoveCharmAuras();
        Unit* GetCharmerOrOwner() const { return GetCharmerGuid() ? GetCharmer() : GetOwner(); }
        Unit* GetCharmerOrOwnerOrSelf()
        {
            if(Unit* u = GetCharmerOrOwner())
                return u;

            return this;
        }
        Unit const* GetCharmerOrOwnerOrSelf() const
        {
            Unit const*u = GetCharmerOrOwner();
            return u ? u : this;
        }

        bool IsCharmerOrOwnerPlayerOrPlayerItself() const;
        Player* GetCharmerOrOwnerPlayerOrPlayerItself() const;
        Player* GetAffectingPlayer() const;
        float GetLeewayBonusRange(const Unit* target) const;
        float GetLeewayBonusRadius() const;

        void SetPet(Pet* pet);
        void SetCharm(Unit* pet);

        void RestoreFaction();
        bool canAttack(Unit const* target, bool force = false) const;

        void AddGuardian(Pet* pet);
        void RemoveGuardian(Pet* pet);
        void RemoveGuardians();
        void RemoveGuardiansWithEntry(uint32 entry);
        Pet* FindGuardianWithEntry(uint32 entry);
        uint32 GetGuardianCountWithEntry(uint32 entry);

        bool isCharmed() const { return !GetCharmerGuid().IsEmpty(); }

        CharmInfo* GetCharmInfo() const { return m_charmInfo; }
        CharmInfo* InitCharmInfo(Unit* charm);

        ObjectGuid const& GetTotemGuid(TotemSlot slot) const { return m_TotemSlot[slot]; }
        Totem* GetTotem(TotemSlot slot) const;
        bool IsAllTotemSlotsUsed() const;

        void _AddTotem(TotemSlot slot, Totem* totem);       // only for call from Totem summon code
        void _RemoveTotem(Totem* totem);                    // only for call from Totem class

        template<typename Func>
        void CallForAllControlledUnits(Func const& func, uint32 controlledMask);
        template<typename Func>
        bool CheckAllControlledUnits(Func const& func, uint32 controlledMask) const;

        bool AddSpellAuraHolder(SpellAuraHolder *holder);
        void AddAuraToModList(Aura *aura);

        // removing specific aura stack
        void RemoveAura(Aura* aura, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAura(uint32 spellId, SpellEffectIndex effindex, Aura* except = nullptr);
        void RemoveSpellAuraHolder(SpellAuraHolder *holder, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveSingleAuraFromSpellAuraHolder(SpellAuraHolder *holder, SpellEffectIndex index, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveSingleAuraFromSpellAuraHolder(uint32 id, SpellEffectIndex index, ObjectGuid casterGuid, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveSingleAuraDueToItemSet(uint32 spellId, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void DeleteAuraHolder(SpellAuraHolder *holder);

        // Limit debuffs a 16
        uint32 GetNegativeAurasCount();
        // Returns true if we remove 'currentAura'
        bool RemoveAuraDueToDebuffLimit(SpellAuraHolder* currentAura);

        // removing specific aura stacks by diff reasons and selections
        void RemoveAurasDueToSpell(uint32 spellId, SpellAuraHolder* except = nullptr, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAurasDueToItemSpell(Item* castItem,uint32 spellId);
        void RemoveAurasByCasterSpell(uint32 spellId, ObjectGuid casterGuid, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAurasDueToSpellBySteal(uint32 spellId, ObjectGuid casterGuid, Unit *stealer);
        void RemoveAurasDueToSpellByCancel(uint32 spellId);

        // removing unknown aura stacks by diff reasons and selections
        void RemoveNotOwnSingleTargetAuras();
        void RemoveAurasAtMechanicImmunity(uint32 mechMask, uint32 exceptSpellId, bool non_positive = false);
        void RemoveSpellsCausingAura(AuraType auraType);
        void RemoveNonPassiveSpellsCausingAura(AuraType auraType);
        void RemoveSpellsCausingAura(AuraType auraType, SpellAuraHolder* except);
        void RemoveRankAurasDueToSpell(uint32 spellId);
        bool RemoveNoStackAurasDueToAuraHolder(SpellAuraHolder *holder);
        void RemoveAurasWithInterruptFlags(uint32 flags, uint32 except = 0);
        void RemoveAurasWithAttribute(uint32 flags);
        void RemoveAurasWithDispelType(DispelType type, ObjectGuid casterGuid = ObjectGuid());
        void RemoveAllAuras(AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAurasAtReset(AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAllNegativeAuras(AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAuraTypeOnDeath(AuraType auraType);
        void RemoveAllAurasOnDeath();

        // removing specific aura FROM stack by diff reasons and selections
        void RemoveAuraHolderFromStack(uint32 spellId, uint32 stackAmount = 1, ObjectGuid casterGuid = ObjectGuid(), AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAuraHolderDueToSpellByDispel(uint32 spellId, uint32 stackAmount, ObjectGuid casterGuid, Unit *dispeller);

        void DelaySpellAuraHolder(uint32 spellId, int32 delaytime, ObjectGuid casterGuid);

        void SetCreateStat(Stats stat, float val) { m_createStats[stat] = val; }
        void SetCreateHealth(uint32 val) { SetUInt32Value(UNIT_FIELD_BASE_HEALTH, val); }
        uint32 GetCreateHealth() const { return GetUInt32Value(UNIT_FIELD_BASE_HEALTH); }
        void SetCreateMana(uint32 val) { SetUInt32Value(UNIT_FIELD_BASE_MANA, val); }
        uint32 GetCreateMana() const { return GetUInt32Value(UNIT_FIELD_BASE_MANA); }
        uint32 GetCreatePowers(Powers power) const;
        float GetCreateStat(Stats stat) const { return m_createStats[stat]; }
        void SetCreateResistance(SpellSchools school, int32 val) { m_createResistances[school] = val; }
        int32 GetCreateResistance(SpellSchools school) const { return m_createResistances[school]; }

        void SetCurrentCastedSpell(Spell * pSpell);
        bool IsSpellProhibited(SpellEntry const* pSpell);
        bool HasProhibitedSpell(); /* use this to check if creature is silenced */
        typedef std::list<ProhibitSpellInfo> ProhibitSpellList;
        ProhibitSpellList m_prohibitSpell;

        void InterruptSpell(CurrentSpellTypes spellType, bool withDelayed = true);
        void FinishSpell(CurrentSpellTypes spellType, bool ok = true);

        // set withDelayed to true to account delayed spells as casted
        // delayed+channeled spells are always accounted as casted
        // we can skip channeled or delayed checks using flags
        bool IsNonMeleeSpellCasted(bool withDelayed = false, bool skipChanneled = false, bool skipAutorepeat = false) const;
        bool IsNextSwingSpellCasted() const;
        // for movement generators, check if current casted spell has movement interrupt flags
        bool IsNoMovementSpellCasted() const;
        
        // set withDelayed to true to interrupt delayed spells too
        // delayed+channeled spells are always interrupted
        void InterruptNonMeleeSpells(bool withDelayed, uint32 spellid = 0);
        void InterruptSpellsWithInterruptFlags(uint32 flags, uint32 except = 0);

        Spell* GetCurrentSpell(CurrentSpellTypes spellType) const { return m_currentSpells[spellType]; }
        Spell* FindCurrentSpellBySpellId(uint32 spell_id) const;

        bool CheckAndIncreaseCastCounter();
        void DecreaseCastCounter() { if (m_castCounter) --m_castCounter; }

        ObjectGuid m_ObjectSlotGuid[4];
        uint32 m_detectInvisibilityMask;
        uint32 m_invisibilityMask;

        ShapeshiftForm GetShapeshiftForm() const { return ShapeshiftForm(GetByteValue(UNIT_FIELD_BYTES_1, 2)); }
        void  SetShapeshiftForm(ShapeshiftForm form) { SetByteValue(UNIT_FIELD_BYTES_1, 2, form); }

        bool IsInFeralForm() const
        {
            ShapeshiftForm form = GetShapeshiftForm();
            return form == FORM_CAT || form == FORM_BEAR || form == FORM_DIREBEAR;
        }

        bool IsInDisallowedMountForm() const;

        float m_modMeleeHitChance;
        float m_modRangedHitChance;
        float m_modSpellHitChance;
        int32 m_baseSpellCritChance;

        float m_threatModifier[MAX_SPELL_SCHOOL];
        float m_modAttackSpeedPct[3];
        float m_modRecalcDamagePct[3];

        // Event handler
        EventProcessor m_Events;

        // stat system
        bool HandleStatModifier(UnitMods unitMod, UnitModifierType modifierType, float amount, bool apply);
        void SetModifierValue(UnitMods unitMod, UnitModifierType modifierType, float value) { m_auraModifiersGroup[unitMod][modifierType] = value; }
        float GetModifierValue(UnitMods unitMod, UnitModifierType modifierType) const;
        float GetTotalStatValue(Stats stat) const;
        int32 GetTotalResistanceValue(SpellSchools school) const;
        float GetTotalAuraModValue(UnitMods unitMod) const;
        SpellSchools GetSpellSchoolByAuraGroup(UnitMods unitMod) const;
        Stats GetStatByAuraGroup(UnitMods unitMod) const;
        Powers GetPowerTypeByAuraGroup(UnitMods unitMod) const;
        bool CanModifyStats() const { return m_canModifyStats; }
        void SetCanModifyStats(bool modifyStats) { m_canModifyStats = modifyStats; }
        virtual bool UpdateStats(Stats stat) = 0;
        virtual bool UpdateAllStats() = 0;
        virtual void UpdateResistances(uint32 school) = 0;
        virtual void UpdateArmor() = 0;
        virtual void UpdateMaxHealth() = 0;
        virtual void UpdateMaxPower(Powers power) = 0;
        virtual void UpdateManaRegen() = 0;
        virtual void UpdateAttackPowerAndDamage(bool ranged = false) = 0;
        virtual void UpdateDamagePhysical(WeaponAttackType attType) = 0;
        float GetTotalAttackPowerValue(WeaponAttackType attType) const;
        float GetWeaponDamageRange(WeaponAttackType attType, WeaponDamageRange damageRange, uint8 index = 0) const;
        SpellSchools GetWeaponDamageSchool(WeaponAttackType attType, uint8 index = 0) const { return m_weaponDamage[attType][index].school; }
        void SetBaseWeaponDamage(WeaponAttackType attType, WeaponDamageRange damageRange, float value, uint8 index = 0) { m_weaponDamage[attType][index].damage[damageRange] = value; }
        void SetWeaponDamageSchool(WeaponAttackType attType, SpellSchools school, uint8 index = 0) { m_weaponDamage[attType][index].school = school; }
        uint8 GetWeaponDamageCount(WeaponAttackType attType) const { return m_weaponDamageCount[attType]; }

        void SetInFront(Unit const* pTarget);
        void SetFacingTo(float ori);
        void SetFacingToObject(WorldObject* pObject);
        bool IsBehindTarget(Unit const* pTarget, bool strict = true) const;

        // Visibility system
        UnitVisibility GetVisibility() const { return m_Visibility; }
        void SetVisibility(UnitVisibility x);
        void UpdateVisibilityAndView() override;                // overwrite WorldObject::UpdateVisibilityAndView()

        // common function for visibility checks for player/creatures with detection code
        bool isVisibleForOrDetect(Unit const* u, WorldObject const* viewPoint, bool detect, bool inVisibleList = false, bool* alert = nullptr) const;
        bool canDetectInvisibilityOf(Unit const* u) const;
        bool canDetectStealthOf(Unit const* u, float distance, bool* alert = nullptr) const;

        // virtual functions for all world objects types
        bool isVisibleForInState(Player const* u, WorldObject const* viewPoint, bool inVisibleList) const override;
        // function for low level grid visibility checks in player/creature cases
        virtual bool IsVisibleInGridForPlayer(Player* pl) const = 0;
        bool isInvisibleForAlive() const;
        bool isVisibleForDead() const;

        SingleCastSpellTargetMap      & GetSingleCastSpellTargets()       { return m_singleCastSpellTargets; }
        SingleCastSpellTargetMap const& GetSingleCastSpellTargets() const { return m_singleCastSpellTargets; }
        SpellImmuneList m_spellImmune[MAX_SPELL_IMMUNITY];
        uint32 m_lastSanctuaryTime;

        // Threat related methods
        bool CanHaveThreatList() const;
        void AddThreat(Unit* pVictim, float threat = 0.0f, bool crit = false, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NONE, SpellEntry const *threatSpell = nullptr);
        float ApplyTotalThreatModifier(float threat, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL);
        void DeleteThreatList();
        bool SelectHostileTarget();
        void TauntApply(Unit* pVictim);
        void TauntFadeOut(Unit *taunter);
        ThreatManager& getThreatManager() { return m_ThreatManager; }
        ThreatManager const& getThreatManager() const { return m_ThreatManager; }
        void addHatedBy(HostileReference* pHostileReference) { m_HostileRefManager.insertFirst(pHostileReference); };
        void removeHatedBy(HostileReference* /*pHostileReference*/ ) { /* nothing to do yet */ }
        HostileRefManager& getHostileRefManager() { return m_HostileRefManager; }

        Aura* GetAura(uint32 spellId, SpellEffectIndex effindex);
        Aura* GetAura(AuraType type, SpellFamily family, uint64 familyFlag, ObjectGuid casterGuid = ObjectGuid());
        SpellAuraHolder* GetSpellAuraHolder (uint32 spellid) const;
        SpellAuraHolder* GetSpellAuraHolder (uint32 spellid, ObjectGuid casterGUID) const;

        SpellAuraHolderMap      & GetSpellAuraHolderMap()       { return m_spellAuraHolders; }
        SpellAuraHolderMap const& GetSpellAuraHolderMap() const { return m_spellAuraHolders; }
        AuraList const& GetAurasByType(AuraType type) const { return m_modAuras[type]; }
        void ApplyAuraProcTriggerDamage(Aura* aura, bool apply);

        int32 GetTotalAuraModifier(AuraType auratype) const;
        float GetTotalAuraMultiplier(AuraType auratype) const;
        int32 GetMaxPositiveAuraModifier(AuraType auratype) const;
        int32 GetMaxNegativeAuraModifier(AuraType auratype) const;

        int32 GetTotalAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        float GetTotalAuraMultiplierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        int32 GetMaxPositiveAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        int32 GetMaxNegativeAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;

        int32 GetTotalAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
        float GetTotalAuraMultiplierByMiscValue(AuraType auratype, int32 misc_value) const;
        int32 GetMaxPositiveAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
        int32 GetMaxNegativeAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;

        Aura* GetDummyAura(uint32 spell_id) const;

        uint32 m_AuraFlags;

        uint32 GetDisplayId() const { return GetUInt32Value(UNIT_FIELD_DISPLAYID); }
        void SetDisplayId(uint32 modelId);
        uint32 GetNativeDisplayId() const { return GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID); }
        void SetNativeDisplayId(uint32 modelId) { SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, modelId); }
        void setTransForm(uint32 spellid) { m_transform = spellid;}
        uint32 getTransForm() const { return m_transform;}
        float GetCollisionHeight() const { return m_modelCollisionHeight * m_nativeScaleOverride; }

        // at any changes to scale and/or displayId
        void UpdateModelData();
        void GetDynObjects(uint32 spellId, SpellEffectIndex effectIndex, std::vector<DynamicObject*>& dynObjsOut);
        DynamicObject* GetDynObject(uint32 spellId, SpellEffectIndex effIndex);
        DynamicObject* GetDynObject(uint32 spellId);
        void AddDynObject(DynamicObject* dynObj);
        void RemoveDynObject(uint32 spellid);
        void RemoveDynObjectWithGUID(ObjectGuid guid) { m_dynObjGUIDs.remove(guid); }
        void RemoveAllDynObjects();

        GameObject* GetGameObject(uint32 spellId) const;
        void AddGameObject(GameObject* gameObj);
        void RemoveGameObject(GameObject* gameObj, bool del);
        void RemoveGameObject(uint32 spellid, bool del);
        void RemoveAllGameObjects();

        uint32 CalculateDamage(WeaponAttackType attType, bool normalized, uint8 index = 0);
        float GetAPMultiplier(WeaponAttackType attType, bool normalized) const;
        void ModifyAuraState(AuraState flag, bool apply);
        bool HasAuraState(AuraState flag) const { return HasFlag(UNIT_FIELD_AURASTATE, 1<<(flag-1)); }
        void UnsummonAllTotems();
        bool UnsummonOldPetBeforeNewSummon(uint32 newPetEntry);
        Unit* SelectMagnetTarget(Unit *victim, Spell* spell = nullptr, SpellEffectIndex eff = EFFECT_INDEX_0);

        int32 SpellBonusWithCoeffs(SpellEntry const *spellProto, int32 total, int32 benefit, int32 ap_benefit, DamageEffectType damagetype, bool donePart, Unit *pCaster, Spell* spell = nullptr);
        int32 SpellBaseDamageBonusDone(SpellSchoolMask schoolMask);
        int32 SpellBaseDamageBonusTaken(SpellSchoolMask schoolMask);
        uint32 SpellDamageBonusDone(Unit *pVictim, SpellEntry const *spellProto, uint32 pdamage, DamageEffectType damagetype, uint32 stack = 1, Spell* spell = nullptr);
        uint32 SpellDamageBonusTaken(Unit *pCaster, SpellEntry const *spellProto, uint32 pdamage, DamageEffectType damagetype, uint32 stack = 1, Spell* spell = nullptr);
        int32 SpellBaseHealingBonusDone(SpellSchoolMask schoolMask);
        int32 SpellBaseHealingBonusTaken(SpellSchoolMask schoolMask);
        uint32 SpellHealingBonusDone(Unit *pVictim, SpellEntry const *spellProto, int32 healamount, DamageEffectType damagetype, uint32 stack = 1, Spell* spell = nullptr);
        uint32 SpellHealingBonusTaken(Unit *pCaster, SpellEntry const *spellProto, int32 healamount, DamageEffectType damagetype, uint32 stack = 1, Spell* spell = nullptr);
        uint32 MeleeDamageBonusDone(Unit *pVictim, uint32 damage, WeaponAttackType attType,
            SpellEntry const *spellProto = nullptr, DamageEffectType damagetype = DIRECT_DAMAGE, uint32 stack = 1, Spell* spell = nullptr, bool flat = true);
        uint32 MeleeDamageBonusTaken(Unit *pCaster, uint32 pdamage,WeaponAttackType attType,
            SpellEntry const *spellProto = nullptr, DamageEffectType damagetype = DIRECT_DAMAGE, uint32 stack = 1, Spell* spell = nullptr, bool flat = true);

        bool   IsSpellBlocked(Unit *pCaster, SpellEntry const *spellProto, WeaponAttackType attackType = BASE_ATTACK);
        bool   IsSpellCrit(Unit *pVictim, SpellEntry const *spellProto, SpellSchoolMask schoolMask, WeaponAttackType attackType = BASE_ATTACK, Spell* spell = nullptr);
        uint32 SpellCriticalDamageBonus(SpellEntry const *spellProto, uint32 damage, Unit *pVictim, Spell* spell = nullptr);
        uint32 SpellCriticalHealingBonus(SpellEntry const *spellProto, uint32 damage, Unit *pVictim);

        bool IsTriggeredAtSpellProcEvent(Unit *pVictim, SpellAuraHolder* holder, SpellEntry const* procSpell, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, bool isVictim, SpellProcEventEntry const*& spellProcEvent );
        // Aura proc handlers
        SpellAuraProcResult HandleDummyAuraProc(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        SpellAuraProcResult HandleHasteAuraProc(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        SpellAuraProcResult HandleProcTriggerSpellAuraProc(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        SpellAuraProcResult HandleProcTriggerDamageAuraProc(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        SpellAuraProcResult HandleOverrideClassScriptAuraProc(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        SpellAuraProcResult HandleModCastingSpeedNotStackAuraProc(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        SpellAuraProcResult HandleReflectSpellsSchoolAuraProc(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        SpellAuraProcResult HandleModPowerCostSchoolAuraProc(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        SpellAuraProcResult HandleMechanicImmuneResistanceAuraProc(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        SpellAuraProcResult HandleAddTargetTriggerAuraProc(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        SpellAuraProcResult HandleModResistanceAuraProc(Unit* pVictim, uint32 damage, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        SpellAuraProcResult HandleModDamageAuraProc(Unit* pVictim, uint32 damage, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        SpellAuraProcResult HandleNULLProc(Unit* /*pVictim*/, uint32 /*damage*/, Aura* /*triggeredByAura*/, SpellEntry const* /*procSpell*/, uint32 /*procFlag*/, uint32 /*procEx*/, uint32 /*cooldown*/)
        {
            // no proc handler for this aura type
            return SPELL_AURA_PROC_OK;
        }
        SpellAuraProcResult HandleCantTrigger(Unit* /*pVictim*/, uint32 /*damage*/, Aura* /*triggeredByAura*/, SpellEntry const* /*procSpell*/, uint32 /*procFlag*/, uint32 /*procEx*/, uint32 /*cooldown*/)
        {
            // this aura type can't proc
            return SPELL_AURA_PROC_CANT_TRIGGER;
        }

        void SetLastManaUse() { m_lastManaUseTimer = 5000; }
        bool IsUnderLastManaUseEffect() const { return m_lastManaUseTimer; }

        void ApplySpellImmune(uint32 spellId, uint32 op, uint32 type, bool apply);
        void ApplySpellDispelImmunity(const SpellEntry * spellProto, DispelType type, bool apply);
        virtual bool IsImmuneToSpell(SpellEntry const* spellInfo, bool castOnSelf);
        virtual bool IsImmuneToDamage(SpellSchoolMask meleeSchoolMask, SpellEntry const* spellInfo = nullptr);
        virtual bool IsImmuneToSpellEffect(SpellEntry const* spellInfo, SpellEffectIndex index, bool castOnSelf) const;

        float GetSpellResistChance(Unit const* victim, uint32 schoolMask, bool innateResists) const;
        uint32 CalcArmorReducedDamage(Unit* pVictim, const uint32 damage);
        void CalculateDamageAbsorbAndResist(Unit *pCaster, SpellSchoolMask schoolMask, DamageEffectType damagetype, const uint32 damage, uint32 *absorb, int32 *resist, SpellEntry const* spellProto = nullptr, Spell* spell = nullptr);
        void CalculateAbsorbResistBlock(Unit *pCaster, SpellNonMeleeDamage *damageInfo, SpellEntry const* spellProto, WeaponAttackType attType = BASE_ATTACK, Spell* spell = nullptr);
        float RollMagicResistanceMultiplierOutcomeAgainst(const Unit* pCaster, SpellSchoolMask schoolMask, DamageEffectType dmgType, SpellEntry const* spellProto) const;

        void _RemoveAllAuraMods();
        void _ApplyAllAuraMods();

        int32 CalculateSpellDamage(Unit const* target, SpellEntry const* spellProto, SpellEffectIndex effect_index, int32 const* basePoints = nullptr, Spell* spell = nullptr);

        float CalculateLevelPenalty(SpellEntry const* spellProto) const;

        void addFollower(FollowerReference* pRef) { m_FollowingRefManager.insertFirst(pRef); }
        void removeFollower(FollowerReference* /*pRef*/ ) { /* nothing to do yet */ }

        MotionMaster* GetMotionMaster() { return &i_motionMaster; }
        MotionMaster const* GetMotionMaster() const { return &i_motionMaster; }

        bool IsStopped() const { return !(hasUnitState(UNIT_STAT_MOVING)); }
        void StopMoving(bool force = false);

        void SetFleeing(bool apply, ObjectGuid casterGuid = ObjectGuid(), uint32 spellID = 0, uint32 time = 0);
        void SetFeared(bool apply, ObjectGuid casterGuid = ObjectGuid(), uint32 spellID = 0, uint32 time = 0);/*DEPRECATED METHOD*/
        void SetConfused(bool apply, ObjectGuid casterGuid = ObjectGuid(), uint32 spellID = 0);/*DEPRECATED METHOD*/
        void SetFeignDeath(bool apply, ObjectGuid casterGuid = ObjectGuid(), bool success = true);

        void InterruptSpellsCastedOnMe(bool killDelayed = false, bool interruptPositiveSpells = false);
        uint32 DespawnNearCreaturesByEntry(uint32 entry, float range);
        uint32 RespawnNearCreaturesByEntry(uint32 entry, float range);
        uint32 DespawnHostileCreaturesInRange(float range = 0.0f);
        void InterruptAttacksOnMe(float dist=0.0f, bool guard_check = false); // Interrompt toutes les "auto-attaques"
        void CombatStopInRange(float dist=0.0f); // CombatStop tous les ennemis

        void AddComboPointHolder(uint32 lowguid) { m_ComboPointHolders.insert(lowguid); }
        void RemoveComboPointHolder(uint32 lowguid) { m_ComboPointHolders.erase(lowguid); }
        void ClearComboPointHolders();

        ///----------Pet responses methods-----------------
        void SendPetCastFail(uint32 spellid, SpellCastResult msg);
        void SendPetActionFeedback (uint8 msg);
        void SendPetTalk (uint32 pettalk);
        void SendPetAIReaction();
        ///----------End of Pet responses methods----------

        void propagateSpeedChange() { GetMotionMaster()->propagateSpeedChange(); }

        // reactive attacks
        void ClearAllReactives();
        void StartReactiveTimer(ReactiveType reactive, ObjectGuid target) { m_reactiveTimer[reactive] = REACTIVE_TIMER_START; m_reactiveTarget[reactive] = target; }
        ObjectGuid const& GetReactiveTarget(ReactiveType reactive) const { return m_reactiveTarget[reactive]; }
        void UpdateReactives(uint32 p_time);

        // group updates
        void UpdateAuraForGroup(uint8 slot);
        
        bool IsLinkingEventTrigger() { return m_isCreatureLinkingTrigger; }

        // pet auras
        typedef std::set<PetAura const*> PetAuraSet;
        PetAuraSet m_petAuras;
        void AddPetAura(PetAura const* petSpell);
        void RemovePetAura(PetAura const* petSpell);

        void UpdateControl();
        uint32 m_castingSpell;
        void ModConfuseSpell(bool apply, ObjectGuid casterGuid, uint32 spellID, MovementModType modType, uint32 time=0);
        // "Un sort plus puissant est actif"
        bool HasMorePowerfullSpellActive(SpellEntry const* spellInfos);

        // Renvoit l'aura le plus important de la meme sorte que 'like', sauf except.
        Aura* GetMostImportantAuraAfter(Aura const* like, Aura const* except = nullptr);
        // debug.
        void Debug(uint32 debugType, const char* str, ...) const ATTR_PRINTF(3, 4);
        void SetDebugger(ObjectGuid playerGuid, uint32 flags)
        {
            _debuggerGuid = playerGuid;
            _debugFlags   = flags;
        }
        uint32 GetDebugFlags() const { return _debugFlags; }
        ObjectGuid GetDebuggerGuid() const { return _debuggerGuid; }
        ObjectGuid _debuggerGuid;
        uint32     _debugFlags;

        bool IsWithinMeleeRange(const Unit *obj, float dist = 0.0f) const;
        float GetMeleeReach() const;
        float GetCombatReach(bool forMeleeRange /*=true*/) const;
        float GetCombatReach(Unit const* pVictim, bool forMeleeRange = true, float flatMod = 0.0f) const;
        void GetRandomAttackPoint(const Unit* target, float &x, float &y, float &z) const;

        bool CanReachWithMeleeAttack(Unit const* pVictim, float flat_mod = 0.0f) const;
        bool CanReachWithMeleeAttackAtPosition(Unit const* pVictim, float x, float y, float z, float flat_mod = 0.0f) const;
        bool CanReachWithMeleeSpellAttack(Unit const* pVictim, float flat_mod = 0.0f) const;

        // Caster movement
        float GetMinChaseDistance(Unit* target) const;
        float GetMaxChaseDistance(Unit* target) const;
        bool HasDistanceCasterMovement() const { return (_casterChaseDistance >= 1.0f); }
        void SetCasterChaseDistance(float dist) { _casterChaseDistance = dist; }
        float _casterChaseDistance;

        void RestoreMovement();

        void AddExtraAttackOnUpdate() { m_doExtraAttacks = true; };

        // (pAttacker doit etre valide)
        virtual void OnEnterCombat(Unit* /*pAttacker*/, bool /*notInCombat*/) {}
        virtual void OnLeaveCombat() {}
        virtual void OnMoveTo(float, float, float) {}

        // Appele dans SpellEffects::EffectSummonPet et apres un rez en BG.
        ObjectGuid EffectSummonPet(uint32 spellId, uint32 petEntry, uint32 petLevel);
        void ModPossess(Unit* target, bool apply, AuraRemoveMode m_removeMode = AURA_REMOVE_BY_DEFAULT);

        // Caster ?
        bool IsCaster();
        CreatureAI* AI() const;

        // Auras
        SpellAuraHolder* AddAura(uint32 spellId, uint32 addAuraFlags = 0, Unit* pCaster = nullptr);
        // Refresh seulement si l'aura existe.
        SpellAuraHolder* RefreshAura(uint32 spellId, int32 duration);

        Movement::MoveSpline* movespline;
        // Serialize access to the movespline to prevent thread race conditions in async
        // move spline updates (one thread updates a spline, while another checks the
        // spline for end point with targeted move gen)
        ACE_Thread_Mutex asyncMovesplineLock;

        void ScheduleAINotify(uint32 delay);
        bool IsAINotifyScheduled() const { return m_AINotifyScheduled;}
        void _SetAINotifyScheduled(bool on) { m_AINotifyScheduled = on;}       // only for call from RelocationNotifyEvent code
        void OnRelocated();
        void ProcessRelocationVisibilityUpdates();
        bool m_needUpdateVisibility;

        uint32 m_lastCastedSpellID;
        virtual void SetLastCastedSpell(uint32 spell_id, bool byclient);
        virtual uint32 GetLastCastedSpell(bool byclientonly);
        uint32 m_lastAttackType;
        uint32 GetLastAttackType();
        void SetLastAttackType(uint32 attackType);
        // Nostalrius - suis-je considere comme un joueur ? (joueur ou un pet de joueur)
        bool IsLikePlayer() const;

        // Nostalrius - en fin de CM par exemple
        void TransferAttackersThreatTo(Unit* unit);
        void RemoveAttackersThreat(Unit* owner);

        bool IsInPartyWith(Unit const* unit) const;
        bool IsInRaidWith(Unit const* unit) const;
        bool HasBreakableByDamageCrowdControlAura(Unit* excludeCasterChannel= nullptr) const;
        bool HasBreakableByDamageAuraType(AuraType type, uint32 excludeAura) const;

        // Cooldown management
        SpellCooldowns const& GetSpellCooldownMap() const { return m_spellCooldowns; }
        static uint32 const infinityCooldownDelay = MONTH;  // used for set "infinity cooldowns" for spells and check
        static uint32 const infinityCooldownDelayCheck = MONTH/2;

        bool HasSpellCategoryCooldown(uint32 category) const;
        bool HasSpellCooldown(uint32 spell_id) const
        {
            SpellCooldowns::const_iterator itr = m_spellCooldowns.find(spell_id);
            return itr != m_spellCooldowns.end() && itr->second.end > time(nullptr);
        }
        time_t GetSpellCooldownDelay(uint32 spell_id) const
        {
            SpellCooldowns::const_iterator itr = m_spellCooldowns.find(spell_id);
            time_t t = time(nullptr);
            return itr != m_spellCooldowns.end() && itr->second.end > t ? itr->second.end - t : 0;
        }
        void CooldownEvent(SpellEntry const *spellInfo, uint32 itemId = 0, Spell* spell = nullptr);
        void AddSpellAndCategoryCooldowns(SpellEntry const* spellInfo, uint32 itemId, Spell* spell = nullptr, bool infinityCooldown = false );
        void AddSpellCooldown(uint32 spell_id, uint32 itemid, time_t endTime, time_t categoryEndTime = 0, uint32 cat = 0);
        virtual void ProhibitSpellSchool(SpellSchoolMask idSchoolMask, uint32 unTimeMs);
        void RemoveSpellCooldown(uint32 spell_id, bool update = false);
        void RemoveSpellCategoryCooldown(uint32 cat, bool update = false);
        void WritePetSpellsCooldown(WorldPacket& data);

        GlobalCooldownMgr& GetGlobalCooldownMgr() { return m_GlobalCooldownMgr; }

        void RemoveAllSpellCooldown();

        void setTransformScale(float scale);
        void resetTransformScale();
        float getNativeScale() const;
        void setNativeScale(float scale);

    protected:
        explicit Unit ();

        void _UpdateSpells(uint32 time);

        void _UpdateAutoRepeatSpell();
        bool m_AutoRepeatFirstCast;

        uint32 m_attackTimer[MAX_ATTACK];

        float m_createStats[MAX_STATS];
        int32 m_createResistances[MAX_SPELL_SCHOOL];

        AttackerSet m_attackers;
        Unit* m_attacking;

        DeathState m_deathState;

        SpellAuraHolderMap m_spellAuraHolders;
        SpellAuraHolderMap::iterator m_spellAuraHoldersUpdateIterator; // != end() in Unit::m_spellAuraHolders update and point to next element
        AuraList m_deletedAuras;                                       // auras removed while in ApplyModifier and waiting deleted
        SpellAuraHolderList m_deletedHolders;

        SingleCastSpellTargetMap m_singleCastSpellTargets;  // casted by unit single per-caster auras

        typedef std::list<ObjectGuid> DynObjectGUIDs;
        DynObjectGUIDs m_dynObjGUIDs;

        typedef std::list<GameObject*> GameObjectList;
        GameObjectList m_gameObj;
        bool m_isSorted;
        uint32 m_transform;
        float m_modelCollisionHeight;

        AuraList m_modAuras[TOTAL_AURAS];
        float m_auraModifiersGroup[UNIT_MOD_END][MODIFIER_TYPE_END];
        WeaponDamageInfo m_weaponDamage[MAX_ATTACK][MAX_ITEM_PROTO_DAMAGES];
        uint8 m_weaponDamageCount[MAX_ATTACK];
        bool m_canModifyStats;
        //std::list< spellEffectPair > AuraSpells[TOTAL_AURAS];  // TODO: use this if ok for mem

        float m_speed_rate[MAX_MOVE_TYPE];

        CharmInfo *m_charmInfo;
        ObjectGuid m_possessorGuid; // Guid of unit possessing this one

        virtual SpellSchoolMask GetMeleeDamageSchoolMask() const;

        MotionMaster i_motionMaster;

        uint32 m_reactiveTimer[MAX_REACTIVE];
        ObjectGuid m_reactiveTarget[MAX_REACTIVE];
        int32 m_regenTimer;
        uint32 m_lastManaUseTimer;
        uint32 m_spellUpdateTimeBuffer;

        SpellCooldowns m_spellCooldowns;
        GlobalCooldownMgr m_GlobalCooldownMgr;

        bool m_isCreatureLinkingTrigger;
        bool m_isSpawningLinked;

    public:
        void DisableSpline();
        void UnitDamaged(ObjectGuid from, uint32 damage) { _damageTakenHistory[from] += damage; _lastDamageTaken = 0; }
        void SetMeleeZLimit(float newZLimit) { m_meleeZLimit = newZLimit; }
        float GetMeleeZLimit() const { return m_meleeZLimit; }
        void SetMeleeZReach(float newZReach) { m_meleeZReach = newZReach; }
        float GetMeleeZReach() const { return m_meleeZReach; }

    protected:
        typedef std::map<ObjectGuid /*attackerGuid*/, uint32 /*damage*/ > DamageTakenHistoryMap;
        DamageTakenHistoryMap   _damageTakenHistory;
        uint32                  _lastDamageTaken;

        float m_nativeScale = 1.0f;
        float m_nativeScaleOverride = 1.0f;

    private:
        void CleanupDeletedAuras();
        void UpdateSplineMovement(uint32 t_diff);

        Unit* _GetTotem(TotemSlot slot) const;              // for templated function without include need
        Pet* _GetPet(ObjectGuid guid) const;                // for templated function without include need

        uint32 m_state;                                     // Even derived shouldn't modify
        uint32 m_CombatTimer;

        // extra attacks vars
        uint32 m_extraAttacks;
        bool m_extraMute;
        bool m_doExtraAttacks;

        Spell* m_currentSpells[CURRENT_MAX_SPELL];
        uint32 m_castCounter;                               // count casts chain of triggered spells for prevent infinity cast crashes

        UnitVisibility m_Visibility;
        Position m_last_notified_position;
        bool m_AINotifyScheduled;

        Diminishing m_Diminishing;
        // Manage all Units threatening us
        ThreatManager m_ThreatManager;

        // Manage all Units that are threatened by us
        HostileRefManager m_HostileRefManager;
        FollowerRefManager m_FollowingRefManager;

        ComboPointHolderSet m_ComboPointHolders;

        GuardianPetList m_guardianPets;

        ObjectGuid m_TotemSlot[MAX_TOTEM_SLOT];

        float m_meleeZLimit;
        float m_meleeZReach;

        // Error traps for some wrong args using
        // this will catch and prevent build for any cases when all optional args skipped and instead triggered used non boolean type
        // no bodies expected for this declarations
        template <typename TR>
        void CastSpell(Unit* Victim, uint32 spell, TR triggered);
        template <typename TR>
        void CastSpell(Unit* Victim, SpellEntry const* spell, TR triggered);
        template <typename TR>
        void CastCustomSpell(Unit* Victim, uint32 spell, int32 const* bp0, int32 const* bp1, int32 const* bp2, TR triggered);
        template <typename SP, typename TR>
        void CastCustomSpell(Unit* Victim, SpellEntry const* spell, int32 const* bp0, int32 const* bp1, int32 const* bp2, TR triggered);
        template <typename TR>
        void CastSpell(float x, float y, float z, uint32 spell, TR triggered);
        template <typename TR>
        void CastSpell(float x, float y, float z, SpellEntry const* spell, TR triggered);
};

template<typename Func>
void Unit::CallForAllControlledUnits(Func const& func, uint32 controlledMask)
{
    if (controlledMask & CONTROLLED_PET)
        if (Pet* pet = GetPet())
            func(pet);

    if (controlledMask & CONTROLLED_MINIPET)
        if (Pet* mini = GetMiniPet())
            func(mini);

    if (controlledMask & CONTROLLED_GUARDIANS)
    {
        for(GuardianPetList::const_iterator itr = m_guardianPets.begin(); itr != m_guardianPets.end();)
            if (Pet* guardian = _GetPet(*(itr++)))
                func(guardian);
    }

    if (controlledMask & CONTROLLED_TOTEMS)
    {
        for (int i = 0; i < MAX_TOTEM_SLOT; ++i)
            if (Unit *totem = _GetTotem(TotemSlot(i)))
                func(totem);
    }

    if (controlledMask & CONTROLLED_CHARM)
        if (Unit* charm = GetCharm())
            func(charm);
}


template<typename Func>
bool Unit::CheckAllControlledUnits(Func const& func, uint32 controlledMask) const
{
    if (controlledMask & CONTROLLED_PET)
        if (Pet const* pet = GetPet())
            if (func(pet))
                return true;

    if (controlledMask & CONTROLLED_MINIPET)
        if (Pet* mini = GetMiniPet())
            if (func(mini))
                return true;

    if (controlledMask & CONTROLLED_GUARDIANS)
    {
        for(GuardianPetList::const_iterator itr = m_guardianPets.begin(); itr != m_guardianPets.end();)
            if (Pet const* guardian = _GetPet(*(itr++)))
                if (func(guardian))
                    return true;

    }

    if (controlledMask & CONTROLLED_TOTEMS)
    {
        for (int i = 0; i < MAX_TOTEM_SLOT; ++i)
            if (Unit const* totem = _GetTotem(TotemSlot(i)))
                if (func(totem))
                    return true;
    }

    if (controlledMask & CONTROLLED_CHARM)
        if (Unit const* charm = GetCharm())
            if (func(charm))
                return true;

    return false;
}

#endif
