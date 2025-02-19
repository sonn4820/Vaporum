#pragma once
#include "Game/GameCommon.hpp"

class Entity;
class Player;
class Prop;
class Map;

class Game
{
public:
	Game();
	~Game();

	void Startup();
	void Update(float deltaSeconds);
	void Render() const;
	void Shutdown();

	bool IsNetworkGame() const;
	void BackToMenu();

	static bool Command_LoadMap(EventArgs& args);

	static bool Command_PlayerReady(EventArgs& args);
	static bool Command_StartTurn(EventArgs& args);
	static bool Command_EndTurn(EventArgs& args);
	static bool Command_SetFocusedHex(EventArgs& args);
	static bool Command_SelectFocusedUnit(EventArgs& args);
	static bool Command_SelectPreviousUnit(EventArgs& args);
	static bool Command_SelectNextUnit(EventArgs& args);
	static bool Command_Move(EventArgs& args);
	static bool Command_Stay(EventArgs& args);
	static bool Command_HoldFire(EventArgs& args);
	static bool Command_Attack(EventArgs& args);
	static bool Command_Cancel(EventArgs& args);
	static bool Command_PlayerQuit(EventArgs& args);
public:
	Camera m_screenCamera;
	Clock* m_clock = nullptr;

	ParticleSystem* m_particleSystem = nullptr;

	Map* m_map = nullptr;

	Texture* m_logoImage = nullptr;

	Texture* m_BisonLogoImage = nullptr;
	Texture* m_GrizzlyLogoImage = nullptr;
	Texture* m_HadrianLogoImage = nullptr;
	Texture* m_OctopusLogoImage = nullptr;
	Texture* m_PolarLogoImage = nullptr;

	Texture* m_ui_Left = nullptr;
	Texture* m_ui_Right = nullptr;
	Texture* m_ui_LMB = nullptr;
	Texture* m_ui_RMB = nullptr;
	Texture* m_ui_Y = nullptr;

	std::vector<Texture*> m_hitEffectTextures;
	std::vector<Texture*> m_shotEffectTextures;
	std::vector<Texture*> m_explodeEffectTextures;

	// MENU
	Canvas* m_menuCanvas = nullptr;
	Button* m_introFirstClick = nullptr;
	Text* m_introText = nullptr;
	Text* m_introTitle = nullptr;
	Panel* m_mainMenuPanel = nullptr;
	Text* m_menuTitle = nullptr;
	Panel* m_menuLine = nullptr;
	Button* m_menuLocalPlay = nullptr;
	Button* m_menuNetworkPlay = nullptr;
	Button* m_menuQuit = nullptr;

	Panel* m_waitingPanel = nullptr;
	Text* m_waitingText = nullptr;
	// PAUSE
	Canvas* m_pauseCanvas = nullptr;
	Panel* m_pauseMenuPanel = nullptr;
	Text* m_pauseTitle = nullptr;
	Panel* m_pauseLine = nullptr;
	Button* m_pauseResume = nullptr;
	Button* m_pauseQuit = nullptr;
	// IN GAME
	Canvas* m_gameCanvas = nullptr;
	Panel* m_gameButtonListPanel = nullptr;
	Sprite* m_RMBSprite = nullptr;
	Text* m_RMBText= nullptr;
	Sprite* m_LMBSprite = nullptr;
	Text* m_LMBText = nullptr;
	Sprite* m_LeftSprite = nullptr;
	Text* m_LeftText = nullptr;
	Sprite* m_RightSprite = nullptr;
	Text* m_RightText = nullptr;
	Sprite* m_YSprite = nullptr;
	Text* m_YText = nullptr;

	Panel* m_p1Panel = nullptr;
	Text* m_p1PanelText = nullptr;
	Panel* m_p2Panel = nullptr;
	Text* m_p2PanelText = nullptr;

	Panel* m_p1TankInfoPanel = nullptr;
	Text* m_p1TankInfoTitle = nullptr;
	Sprite* m_p1TankInfoSprite = nullptr;
	Text* m_p1TankInfoTexts = nullptr;
	Text* m_p1TankInfoStats = nullptr;

	Panel* m_p2TankInfoPanel = nullptr;
	Text* m_p2TankInfoTitle = nullptr;
	Sprite* m_p2TankInfoSprite = nullptr;
	Text* m_p2TankInfoTexts = nullptr;
	Text* m_p2TankInfoStats = nullptr;

	Panel* m_dialoguePanel = nullptr;
	Text* m_dialogueTitle = nullptr;
	Text* m_dialogueDetail = nullptr;

	Button* m_backToMenuFromGame = nullptr;

	bool m_isMenu = true;
	bool m_isPaused = false;
	bool m_isOnlineGame = false;

private:
	void PlayLocalGame();
	void PlayNetworkGame();
	void RenderScreenWorld() const;
	void FirstClick();

	void MainMenu_Init();
	void PauseMenu_Init();
	void GameplayUI_Init();
};