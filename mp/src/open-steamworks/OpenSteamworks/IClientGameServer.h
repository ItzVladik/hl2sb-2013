//==========================  Open Steamworks  ================================
//
// This file is part of the Open Steamworks project. All individuals associated
// with this project do not claim ownership of the contents
// 
// The code, comments, and all related files, projects, resources,
// redistributables included with this project are Copyright Valve Corporation.
// Additionally, Valve, the Valve logo, Half-Life, the Half-Life logo, the
// Lambda logo, Steam, the Steam logo, Team Fortress, the Team Fortress logo,
// Opposing Force, Day of Defeat, the Day of Defeat logo, Counter-Strike, the
// Counter-Strike logo, Source, the Source logo, and Counter-Strike Condition
// Zero are trademarks and or registered trademarks of Valve Corporation.
// All other trademarks are property of their respective owners.
//
//=============================================================================

#ifndef ICLIENTGAMESERVER_H
#define ICLIENTGAMESERVER_H
#ifdef _WIN32
#pragma once
#endif

#include "SteamTypes.h"
#include "GameServerCommon.h"
#include "UserCommon.h"


typedef enum EGameConnectSteamResponse
{
	k_EGameConnectSteamResponse_WaitingForResponse = 0,
	k_EGameConnectSteamResponse_AuthorizedToPlay = 1,
	k_EGameConnectSteamResponse_Denied = 2,
	k_EGameConnectSteamResponse_ExceededReasonableTime_StillWaiting = 3,
} EGameConnectSteamResponse;

struct ConnectedUserInfo_t
{
	int m_cubConnectedUserInfo;
	int m_nCountOfGuestUsers;
	CSteamID m_SteamID;
	uint32 m_unIPPublic;
	uint32 m_nFrags;
	double m_flConnectTime;
	EGameConnectSteamResponse m_eGameConnectSteamResponse;
	EDenyReason m_eDenyReason;
};

abstract_class UNSAFE_INTERFACE IClientGameServer
{
public:
	// returns the HSteamUser this interface represents
	virtual HSteamUser GetHSteamUser() = 0;

	virtual CSteamID GetSteamID() = 0;

	// steam account management functions
	virtual void LogOn() = 0;
	virtual void LogOff() = 0;
	virtual bool BLoggedOn() = 0;

	virtual bool BSecure() = 0;
	virtual ELogonState GetLogonState() = 0;
	virtual bool BConnected() = 0;

	virtual int RaiseConnectionPriority( EConnectionPriority eConnectionPriority ) = 0;
	virtual void ResetConnectionPriority( int hRaiseConnectionPriorityPrev ) = 0;

	virtual void SetCellID( CellID_t cellID ) = 0;

	virtual void TrackSteamUsageEvent( ESteamUsageEvent eSteamUsageEvent, const uint8 *pubKV, uint32 cubKV ) = 0;

	// Handles receiving a new connection from a Steam user.  This call will ask the Steam
	// servers to validate the users identity, app ownership, and VAC status.  If the Steam servers 
	// are off-line, then it will validate the cached ticket itself which will validate app ownership 
	// and identity.  The AuthBlob here should be acquired on the game client using SteamUser()->InitiateGameConnection()
	// and must then be sent up to the game server for authentication.
	//
	// Return Value: returns true if the users ticket passes basic checks. pSteamIDUser will contain the Steam ID of this user. pSteamIDUser must NOT be NULL
	// If the call succeeds then you should expect a GSClientApprove_t or GSClientDeny_t callback which will tell you whether authentication
	// for the user has succeeded or failed (the steamid in the callback will match the one returned by this call)
	virtual bool SendUserConnectAndAuthenticate( uint32 unIPClient, const void *pvAuthBlob, uint32 cubAuthBlobSize, CSteamID *pSteamIDUser ) = 0;

	// Creates a fake user (ie, a bot) which will be listed as playing on the server, but skips validation.  
	// 
	// Return Value: Returns a SteamID for the user to be tracked with, you should call HandleUserDisconnect()
	// when this user leaves the server just like you would for a real user.
	virtual CSteamID CreateUnauthenticatedUserConnection() = 0;

	// Should be called whenever a user leaves our game server, this lets Steam internally
	// track which users are currently on which servers for the purposes of preventing a single
	// account being logged into multiple servers, showing who is currently on a server, etc.
	virtual void SendUserDisconnect( CSteamID steamIDUser ) = 0;

	// You shouldn't need to call this as it is called internally by SteamGameServer_Init() and can only be called once.
	//
	// To update the data in this call which may change during the servers lifetime see UpdateServerStatus() below.
	//
	// Input:	nGameAppID - The Steam assigned AppID for the game
	//			unServerFlags - Any applicable combination of flags (see k_unServerFlag____ constants below)
	//			unGameIP - The IP Address the server is listening for client connections on (might be INADDR_ANY)
	//			unGamePort - The port which the server is listening for client connections on
	//			unSpectatorPort - the port on which spectators can join to observe the server, 0 if spectating is not supported
	//			usQueryPort - The port which the ISteamMasterServerUpdater API should use in order to listen for matchmaking requests
	//			pchGameDir - A unique string identifier for your game
	//			pchVersion - The current version of the server as a string like 1.0.0.0
	//			bLanMode - Is this a LAN only server?
	//			
	// bugbug jmccaskey - figure out how to remove this from the API and only expose via SteamGameServer_Init... or make this actually used,
	// and stop calling it in SteamGameServer_Init()?
	virtual bool BSetServerType( uint32 unServerFlags, uint32 unGameIP, uint16 unGamePort, uint16 unSpectatorPort, uint16 usQueryPort, const char *pchGameDir, const char *pchVersion, bool bLANMode ) = 0;


	// Update the data to be displayed in the server browser and matchmaking interfaces for a user
	// currently connected to the server.  For regular users you must call this after you receive a
	// GSUserValidationSuccess callback.
	// 
	// Return Value: true if successful, false if failure (ie, steamIDUser wasn't for an active player)
	virtual bool BUpdateUserData( CSteamID steamIDUser, const char *pchPlayerName, uint32 uScore ) = 0;

	// Updates server status values which shows up in the server browser and matchmaking APIs
	virtual void UpdateStatus( int cPlayers, int cPlayersMax, int cBotPlayers, const char *pchServerName, const char *pSpectatorServerName, const char *pchMapName ) = 0;

	// This can be called if spectator goes away or comes back (passing 0 means there is no spectator server now).
	virtual void UpdateSpectatorPort( uint16 unSpectatorPort ) = 0;

	virtual void SetGameTags( const char *pchGameTags ) = 0; 

	// Sets a string defining the "gamedata" for this server, this is optional, but if it is set
	// it allows users to filter in the matchmaking/server-browser interfaces based on the value
	// don't set this unless it actually changes, its only uploaded to the master once (when
	// acknowledged)
	virtual void SetGameData( const char *pchGameData) = 0; 

	virtual void SetCountOfSimultaneousGuestUsersPerSteamAccount( int nCount ) = 0;

	virtual bool EnumerateConnectedUsers( int iterator, ConnectedUserInfo_t *pConnectedUserInfo ) = 0;

	// Ask for the gameplay stats for the server. Results returned in a callback
	virtual void GetGameplayStats() = 0;

	virtual SteamAPICall_t GetServerReputation() = 0;

	// Ask if a user in in the specified group, results returns async by GSUserGroupStatus_t
	// returns false if we're not connected to the steam servers and thus cannot ask
	virtual bool RequestUserGroupStatus( CSteamID steamIDUser, CSteamID steamIDGroup ) = 0;

	// Returns the public IP of the server according to Steam, useful when the server is 
	// behind NAT and you want to advertise its IP in a lobby for other clients to directly
	// connect to
	virtual uint32 GetPublicIP() = 0;

	// Retrieve ticket to be sent to the entity who wishes to authenticate you. 
	// pcbTicket retrieves the length of the actual ticket.
	virtual HAuthTicket GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket ) = 0;

	// Authenticate ticket from entity steamID to be sure it is valid and isnt reused
	// Registers for callbacks if the entity goes offline or cancels the ticket ( see ValidateAuthTicketResponse_t callback and EAuthSessionResponse )
	virtual EBeginAuthSessionResult BeginAuthSession( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID ) = 0;

	// Stop tracking started by BeginAuthSession - called when no longer playing game with this entity
	virtual void EndAuthSession( CSteamID steamID ) = 0;

	// Cancel auth ticket from GetAuthSessionTicket, called when no longer playing game with the entity you gave the ticket to
	virtual void CancelAuthTicket( HAuthTicket hAuthTicket ) = 0;

	// After receiving a user's authentication data, and passing it to SendUserConnectAndAuthenticate, use this function
	// to determine if the user owns downloadable content specified by the provided AppID.
	//virtual EUserHasLicenseForAppResult UserHasLicenseForApp( CSteamID steamID, AppId_t appID ) = 0;
	virtual int32 IsUserSubscribedAppInTicket( CSteamID steamID, uint32 appID ) = 0;

	// Ask if a user has a specific achievement for this game, will get a callback on reply
	virtual bool _GetUserAchievementStatus( CSteamID steamID, const char *pchAchievementName ) = 0;

	virtual void _GSSetSpawnCount( uint32 ucSpawn ) = 0;
	virtual bool _GSGetSteam2GetEncryptionKeyToSendToNewClient( void *pvEncryptionKey, uint32 *pcbEncryptionKey, uint32 cbMaxEncryptionKey ) = 0;

	virtual bool _GSSendSteam2UserConnect( uint32 unUserID, const void *pvRawKey, uint32 unKeyLen, uint32 unIPPublic, uint16 usPort, const void *pvCookie, uint32 cubCookie ) = 0;
	virtual bool _GSSendSteam3UserConnect( CSteamID steamID, uint32 unIPPublic, const void *pvCookie, uint32 cubCookie ) = 0;

	virtual bool _GSSendUserConnect( uint32 unUserID, uint32 unIPPublic, uint16 usPort, const void *pvCookie, uint32 cubCookie ) = 0;
	virtual bool _GSRemoveUserConnect( uint32 unUserID ) = 0;

	// Updates server status values which shows up in the server browser and matchmaking APIs
	virtual bool _GSUpdateStatus( int cPlayers, int cPlayersMax, int cBotPlayers, const char *pchServerName, const char *pSpectatorServerName, const char *pchMapName ) = 0;

	virtual bool _GSCreateUnauthenticatedUser( CSteamID *pSteamID ) = 0;

	virtual bool _GSSendUserDisconnect( CSteamID, uint32 unUserID ) = 0;
};


#endif // ICLIENTGAMESERVER_H
