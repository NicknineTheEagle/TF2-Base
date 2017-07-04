//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Draws CSPort's death notices
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "c_playerresource.h"
#include "iclientmode.h"
#include <vgui_controls/Controls.h>
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>
#include "c_baseplayer.h"
#include "c_team.h"

#include "hud_basedeathnotice.h"

#include "tf_shareddefs.h"
#include "clientmode_tf.h"
#include "c_tf_player.h"
#include "c_tf_playerresource.h"
#include "tf_hud_freezepanel.h"
#include "engine/IEngineSound.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Must match resource/tf_objects.txt!!!
const char *szLocalizedObjectNames[OBJ_LAST] =
{
	"#TF_Object_Dispenser",
	"#TF_Object_Tele_Entrance",
	"#TF_Object_Tele_Exit",
	"#TF_Object_Sentry",
	"#TF_object_sapper"			
};

class CTFHudDeathNotice : public CHudBaseDeathNotice
{
	DECLARE_CLASS_SIMPLE( CTFHudDeathNotice, CHudBaseDeathNotice );
public:
	CTFHudDeathNotice( const char *pElementName ) : CHudBaseDeathNotice( pElementName ) {};
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );
	virtual bool IsVisible( void );

	void PlayRivalrySounds( int iKillerIndex, int iVictimIndex, int iType  );

protected:	
	virtual void OnGameEvent( IGameEvent *event, int iDeathNoticeMsg );
	virtual Color GetTeamColor( int iTeamNumber, bool bLocalPlayerInvolved /* = false */ );

private:
	void AddAdditionalMsg( int iKillerID, int iVictimID, const char *pMsgKey );

	CHudTexture		*m_iconDomination;

	CPanelAnimationVar( Color, m_clrBlueText, "TeamBlue", "153 204 255 255" );
	CPanelAnimationVar( Color, m_clrRedText, "TeamRed", "255 64 64 255" );

};

DECLARE_HUDELEMENT( CTFHudDeathNotice );

void CTFHudDeathNotice::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	m_iconDomination = gHUD.GetIcon( "leaderboard_dominated" );
}

bool CTFHudDeathNotice::IsVisible( void )
{
	if ( IsTakingAFreezecamScreenshot() )
		return false;

	return BaseClass::IsVisible();
}

void CTFHudDeathNotice::PlayRivalrySounds( int iKillerIndex, int iVictimIndex, int iType )
{
	int iLocalPlayerIndex = GetLocalPlayerIndex();

	//We're not involved in this kill
	if ( iKillerIndex != iLocalPlayerIndex && iVictimIndex != iLocalPlayerIndex )
		return;

	const char *pszSoundName = NULL;

	if ( iType == TF_DEATH_DOMINATION )
	{
		if ( iKillerIndex == iLocalPlayerIndex )
		{
			pszSoundName = "Game.Domination";
		}
		else if ( iVictimIndex == iLocalPlayerIndex )
		{
			pszSoundName = "Game.Nemesis";
		}
	}
	else if ( iType == TF_DEATH_REVENGE )
	{
		pszSoundName = "Game.Revenge";
	}

	CLocalPlayerFilter filter;
	C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, pszSoundName );
}

//-----------------------------------------------------------------------------
// Purpose: Called when a game event happens and a death notice is about to be 
//			displayed.  This method can examine the event and death notice and
//			make game-specific tweaks to it before it is displayed
//-----------------------------------------------------------------------------
void CTFHudDeathNotice::OnGameEvent(IGameEvent *event, int iDeathNoticeMsg)
{
	const char *pszEventName = event->GetName();

	if ( FStrEq( pszEventName, "player_death" ) || FStrEq( pszEventName, "object_destroyed" ) )
	{
		bool bIsObjectDestroyed = FStrEq( pszEventName, "object_destroyed" );
		int iCustomDamage = event->GetInt( "customkill" );
		int iLocalPlayerIndex = GetLocalPlayerIndex();

		// if there was an assister, put both the killer's and assister's names in the death message
		int iAssisterID = engine->GetPlayerForUserID( event->GetInt( "assister" ) );
		const char *assister_name = ( iAssisterID > 0 ? g_PR->GetPlayerName( iAssisterID ) : NULL );
		if ( assister_name )
		{
			char szKillerBuf[MAX_PLAYER_NAME_LENGTH*2];
			Q_snprintf(szKillerBuf, ARRAYSIZE(szKillerBuf), "%s + %s", m_DeathNotices[iDeathNoticeMsg].Killer.szName, assister_name);
			Q_strncpy(m_DeathNotices[iDeathNoticeMsg].Killer.szName, szKillerBuf, ARRAYSIZE(m_DeathNotices[iDeathNoticeMsg].Killer.szName));
			if ( iLocalPlayerIndex == iAssisterID )
			{
				m_DeathNotices[iDeathNoticeMsg].bLocalPlayerInvolved = true;
			}
		}

		if ( !bIsObjectDestroyed )
		{
			// if this death involved a player dominating another player or getting revenge on another player, add an additional message
			// mentioning that
			int iKillerID = engine->GetPlayerForUserID( event->GetInt( "attacker" ) );
			int iVictimID = engine->GetPlayerForUserID( event->GetInt( "userid" ) );
		
			if ( event->GetInt( "dominated" ) > 0 )
			{
				AddAdditionalMsg( iKillerID, iVictimID, "#Msg_Dominating" );
				PlayRivalrySounds( iKillerID, iVictimID, TF_DEATH_DOMINATION );
			}
			if ( event->GetInt( "assister_dominated" ) > 0 && ( iAssisterID > 0 ) )
			{
				AddAdditionalMsg( iAssisterID, iVictimID, "#Msg_Dominating" );
				PlayRivalrySounds( iAssisterID, iVictimID, TF_DEATH_DOMINATION );
			}
			if ( event->GetInt( "revenge" ) > 0 ) 
			{
				AddAdditionalMsg( iKillerID, iVictimID, "#Msg_Revenge" );
				PlayRivalrySounds( iKillerID, iVictimID, TF_DEATH_REVENGE );
			}
			if ( event->GetInt( "assister_revenge" ) > 0 && ( iAssisterID > 0 ) ) 
			{
				AddAdditionalMsg( iAssisterID, iVictimID, "#Msg_Revenge" );
				PlayRivalrySounds( iAssisterID, iVictimID, TF_DEATH_REVENGE );
			}
		}
		else
		{
			// if this is an object destroyed message, set the victim name to "<object type> (<owner>)"
			int iObjectType = event->GetInt( "objecttype" );
			if ( iObjectType >= 0 && iObjectType < OBJ_LAST )
			{
				// get the localized name for the object
				char szLocalizedObjectName[MAX_PLAYER_NAME_LENGTH];
				szLocalizedObjectName[ 0 ] = 0;
				const wchar_t *wszLocalizedObjectName = g_pVGuiLocalize->Find( szLocalizedObjectNames[iObjectType] );
				if ( wszLocalizedObjectName )
				{
					g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalizedObjectName, szLocalizedObjectName, ARRAYSIZE( szLocalizedObjectName ) );
				}
				else
				{
					Warning( "Couldn't find localized object name for '%s'\n", szLocalizedObjectNames[iObjectType] );
					Q_strncpy( szLocalizedObjectName, szLocalizedObjectNames[iObjectType], sizeof( szLocalizedObjectName ) );
				}

				// compose the string
				if (m_DeathNotices[iDeathNoticeMsg].Victim.szName[0])
				{
					char szVictimBuf[MAX_PLAYER_NAME_LENGTH*2];
					Q_snprintf(szVictimBuf, ARRAYSIZE(szVictimBuf), "%s (%s)", szLocalizedObjectName, m_DeathNotices[iDeathNoticeMsg].Victim.szName);
					Q_strncpy(m_DeathNotices[iDeathNoticeMsg].Victim.szName, szVictimBuf, ARRAYSIZE(m_DeathNotices[iDeathNoticeMsg].Victim.szName));
				}
				else
				{
					Q_strncpy(m_DeathNotices[iDeathNoticeMsg].Victim.szName, szLocalizedObjectName, ARRAYSIZE(m_DeathNotices[iDeathNoticeMsg].Victim.szName));
				}
				
			}
			else
			{
				Assert( false ); // invalid object type
			}
		}

		const wchar_t *pMsg = NULL;
		switch ( iCustomDamage )
		{
		case TF_DMG_CUSTOM_BACKSTAB:
			Q_strncpy(m_DeathNotices[iDeathNoticeMsg].szIcon, "d_backstab", ARRAYSIZE(m_DeathNotices[iDeathNoticeMsg].szIcon));
			break;
		case TF_DMG_CUSTOM_HEADSHOT:
			Q_strncpy(m_DeathNotices[iDeathNoticeMsg].szIcon, "d_headshot", ARRAYSIZE(m_DeathNotices[iDeathNoticeMsg].szIcon));
			break;
		case TF_DMG_CUSTOM_BURNING:
			// special-case if custom kill is burning; if the attacker is dead we can't get weapon information, so force flamethrower as weapon
			Q_strncpy(m_DeathNotices[iDeathNoticeMsg].szIcon, "d_flamethrower", ARRAYSIZE(m_DeathNotices[iDeathNoticeMsg].szIcon));
			m_DeathNotices[iDeathNoticeMsg].wzInfoText[0] = 0;
			break;
		case TF_DMG_CUSTOM_SUICIDE:
			{
				// display a different message if this was suicide, or assisted suicide (suicide w/recent damage, kill awarded to damager)
				bool bAssistedSuicide = event->GetInt( "userid" ) != event->GetInt( "attacker" );
				pMsg = g_pVGuiLocalize->Find( bAssistedSuicide ? "#DeathMsg_AssistedSuicide" : "#DeathMsg_Suicide" );
				if ( pMsg )
				{
					V_wcsncpy(m_DeathNotices[iDeathNoticeMsg].wzInfoText, pMsg, sizeof(m_DeathNotices[iDeathNoticeMsg].wzInfoText));
				}			
				break;
			}
		default:
			break;
		}
	} 
	else if ( FStrEq( "teamplay_point_captured", pszEventName ) || FStrEq( "teamplay_capture_blocked", pszEventName ) || 
		FStrEq( "teamplay_flag_event", pszEventName ) )
	{
		bool bDefense = ( FStrEq( "teamplay_capture_blocked", pszEventName ) || ( FStrEq( "teamplay_flag_event", pszEventName ) &&
			TF_FLAGEVENT_DEFEND == event->GetInt( "eventtype" ) ) );

		const char *szCaptureIcons[] = { "d_redcapture", "d_bluecapture" };
		const char *szDefenseIcons[] = { "d_reddefend", "d_bluedefend" };
		
		int iTeam = m_DeathNotices[iDeathNoticeMsg].Killer.iTeam;
		Assert( iTeam >= FIRST_GAME_TEAM );
		Assert( iTeam < FIRST_GAME_TEAM + TF_TEAM_COUNT );
		if ( iTeam < FIRST_GAME_TEAM || iTeam >= FIRST_GAME_TEAM + TF_TEAM_COUNT )
			return;

		int iIndex = m_DeathNotices[iDeathNoticeMsg].Killer.iTeam - FIRST_GAME_TEAM;
		Assert( iIndex < ARRAYSIZE( szCaptureIcons ) );

		Q_strncpy(m_DeathNotices[iDeathNoticeMsg].szIcon, bDefense ? szDefenseIcons[iIndex] : szCaptureIcons[iIndex], ARRAYSIZE(m_DeathNotices[iDeathNoticeMsg].szIcon));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Adds an additional death message
//-----------------------------------------------------------------------------
void CTFHudDeathNotice::AddAdditionalMsg( int iKillerID, int iVictimID, const char *pMsgKey )
{
	DeathNoticeItem &msg2 = m_DeathNotices[AddDeathNoticeItem()];
	Q_strncpy( msg2.Killer.szName, g_PR->GetPlayerName( iKillerID ), ARRAYSIZE( msg2.Killer.szName ) );
	Q_strncpy( msg2.Victim.szName, g_PR->GetPlayerName( iVictimID ), ARRAYSIZE( msg2.Victim.szName ) );
	const wchar_t *wzMsg =  g_pVGuiLocalize->Find( pMsgKey );
	if ( wzMsg )
	{
		V_wcsncpy( msg2.wzInfoText, wzMsg, sizeof( msg2.wzInfoText ) );
	}
	msg2.iconDeath = m_iconDomination;
	int iLocalPlayerIndex = GetLocalPlayerIndex();
	if ( iLocalPlayerIndex == iVictimID || iLocalPlayerIndex == iKillerID )
	{
		msg2.bLocalPlayerInvolved = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: returns the color to draw text in for this team.  
//-----------------------------------------------------------------------------
Color CTFHudDeathNotice::GetTeamColor( int iTeamNumber, bool bLocalPlayerInvolved /* = false */ )
{
	switch ( iTeamNumber )
	{
	case TF_TEAM_BLUE:
		return m_clrBlueText;
		break;
	case TF_TEAM_RED:
		return m_clrRedText;
		break;
	case TEAM_UNASSIGNED:		
		return Color( 255, 255, 255, 255 );
		break;
	default:
		AssertOnce( false );	// invalid team
		return Color( 255, 255, 255, 255 );
		break;
	}
}
