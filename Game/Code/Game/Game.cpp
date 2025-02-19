#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/Prop.hpp"
#include "Game/Entity.hpp"
#include "Game/Map.hpp"
#include "Game/Unit.hpp"

bool Game::Command_LoadMap(EventArgs& args)
{
	if (!args.IsKeyNameValid("name"))
	{
		g_theDevConsole->AddLine(DevConsole::ERROR, "ERROR: Arguments are missing or incorrect, please use timescale Name=<name>");
		return false;
	}
	std::string mapName = args.GetValue("name", " ");

	MapDefinition* mapDef = MapDefinition::GetByName(mapName);

	if (!mapDef)
	{
		g_theDevConsole->AddLine(DevConsole::ERROR, Stringf("ERROR: Couldn't load map name: %s", mapName.c_str()));
		return false;
	}

	g_theDevConsole->AddLine(DevConsole::SUCCESS, Stringf("SUCCESS: Loaded map: %s", mapName.c_str()));
	g_theApp->m_game->m_map->LoadMapDef(mapDef);
	return true;
}


bool Game::Command_PlayerReady(EventArgs& args)
{
	UNUSED(args);
	g_theApp->m_game->m_map->SetPlayerReady(args.GetValue("id", 1));
	return true;
}

bool Game::Command_StartTurn(EventArgs& args)
{
	UNUSED(args);
	g_theApp->m_game->m_map->StartTurn();
	return true;
}

bool Game::Command_EndTurn(EventArgs& args)
{
	UNUSED(args);
	g_theApp->m_game->m_map->EndTurn();
	return true;
}

bool Game::Command_SetFocusedHex(EventArgs& args)
{
	IntVec2 coords = args.GetValue("coords", IntVec2(-1, -1));
	g_theApp->m_game->m_map->m_currentFocusedCoord = coords;
	
	return true;
}

bool Game::Command_SelectFocusedUnit(EventArgs& args)
{
	IntVec2 coords = args.GetValue("coords", IntVec2(-1, -1));
	Tile* tile = g_theApp->m_game->m_map->GetTile(coords);
	g_theApp->m_game->m_map->Select(tile);
	return true;
}

bool Game::Command_SelectPreviousUnit(EventArgs& args)
{
	UNUSED(args);
	return true;
}

bool Game::Command_SelectNextUnit(EventArgs& args)
{
	UNUSED(args);
	return true;
}

bool Game::Command_Move(EventArgs& args)
{
	IntVec2 coords = args.GetValue("coords", IntVec2(-1, -1));
	g_theApp->m_game->m_map->Move(coords);
	return true;
}

bool Game::Command_Stay(EventArgs& args)
{
	UNUSED(args);
	IntVec2 currentCoord = g_theApp->m_game->m_map->m_currentSelectedUnit->m_currentCoord;
	g_theApp->m_game->m_map->Move(currentCoord);
	return true;
}

bool Game::Command_HoldFire(EventArgs& args)
{
	UNUSED(args);
	g_theApp->m_game->m_map->HoldFire();
	return true;
}

bool Game::Command_Attack(EventArgs& args)
{
	IntVec2 coords = args.GetValue("coords", IntVec2(-1, -1));
	Tile* tile = g_theApp->m_game->m_map->GetTile(coords);
	g_theApp->m_game->m_map->Attack(tile);
	return true;
}

bool Game::Command_Cancel(EventArgs& args)
{
	UNUSED(args);
	g_theApp->m_game->m_map->Cancel();
	return true;
}

bool Game::Command_PlayerQuit(EventArgs& args)
{
	g_theApp->m_game->m_map->SetPlayerQuit(args.GetValue("id", 1));
	return true;
}

Game::Game()
{
}
//..............................
Game::~Game()
{

}
//..............................
void Game::Startup()
{
	UnitDefinitions::InitializeUnitDefs("Data/Definitions/UnitDefinitions.xml");
	TileDefinition::InitializeTileDefs("Data/Definitions/TileDefinitions.xml");
	MapDefinition::InitializeMapDefs("Data/Definitions/MapDefinitions.xml");
	g_theEventSystem->SubscribeEventCallbackFunction("LoadMap", Game::Command_LoadMap);
	g_theEventSystem->SubscribeEventCallbackFunction("PlayerReady", Game::Command_PlayerReady);
	g_theEventSystem->SubscribeEventCallbackFunction("StartTurn", Game::Command_StartTurn);
	g_theEventSystem->SubscribeEventCallbackFunction("Move", Game::Command_Move);
	g_theEventSystem->SubscribeEventCallbackFunction("Stay", Game::Command_Stay);
	g_theEventSystem->SubscribeEventCallbackFunction("HoldFire", Game::Command_HoldFire);
	g_theEventSystem->SubscribeEventCallbackFunction("Attack", Game::Command_Attack);
	g_theEventSystem->SubscribeEventCallbackFunction("Cancel", Game::Command_Cancel);
	g_theEventSystem->SubscribeEventCallbackFunction("EndTurn", Game::Command_EndTurn);
	g_theEventSystem->SubscribeEventCallbackFunction("SetFocusedHex", Game::Command_SetFocusedHex);
	g_theEventSystem->SubscribeEventCallbackFunction("SelectFocusedUnit", Game::Command_SelectFocusedUnit);
	g_theEventSystem->SubscribeEventCallbackFunction("SelectPreviousUnit", Game::Command_SelectPreviousUnit);
	g_theEventSystem->SubscribeEventCallbackFunction("SelectNextUnit", Game::Command_SelectNextUnit);
	g_theEventSystem->SubscribeEventCallbackFunction("PlayerQuit", Game::Command_PlayerQuit);

	m_clock = new Clock(*Clock::s_theSystemClock);

	IntVec2 screenSize = Window::GetMainWindowInstance()->GetClientDimensions();
	m_screenCamera.SetOrthographicView(Vec2(0, 0), Vec2((float)screenSize.x, (float)screenSize.y));

	m_menuCanvas = new Canvas(g_UI, &m_screenCamera);
	m_pauseCanvas = new Canvas(g_UI, &m_screenCamera);
	m_gameCanvas = new Canvas(g_UI, &m_screenCamera);

	m_map = new Map(this);
	m_map->Startup();
	m_map->LoadMapDef(MapDefinition::GetByName("Grid12x12"));

	m_logoImage = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Logo.png");

	m_BisonLogoImage = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Tanks/Bison.png");
	m_GrizzlyLogoImage = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Tanks/Grizzly.png");
	m_HadrianLogoImage = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Tanks/Hadrian.png");
	m_OctopusLogoImage = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Tanks/Octopus.png");
	m_PolarLogoImage = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Tanks/Polar.png");

	m_ui_Left = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Icons/Left.png");
	m_ui_Right = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Icons/Right.png");
	m_ui_LMB = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Icons/LMB.png");
	m_ui_RMB = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Icons/RMB.png");
	m_ui_Y = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Icons/Y.png");

	m_hitEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Fire01.png"));
	m_hitEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Fire02.png"));

	m_shotEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Muzzle01.png"));
	m_shotEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Muzzle02.png"));
	m_shotEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Muzzle03.png"));
	m_shotEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Muzzle04.png"));
	m_shotEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Muzzle05.png"));

	m_explodeEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Smoke01.png"));
	m_explodeEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Smoke02.png"));
	m_explodeEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Smoke03.png"));
	m_explodeEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Smoke04.png"));
	m_explodeEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Smoke05.png"));
	m_explodeEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Smoke06.png"));
	m_explodeEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Smoke07.png"));
	m_explodeEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Smoke08.png"));
	m_explodeEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Smoke09.png"));
	m_explodeEffectTextures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Particles/Smoke10.png"));

	m_particleSystem = new ParticleSystem(g_theRenderer);
	auto shotEffect = new Emitter("TankShot", Vec3::ZERO);
	shotEffect->m_textures = m_shotEffectTextures;
	auto hitEffect = new Emitter("Hit", Vec3::ZERO);
	hitEffect->m_textures = m_hitEffectTextures;
	auto explodeEffect = new Emitter("Explosion", Vec3::ZERO);
	explodeEffect->m_textures = m_explodeEffectTextures;

	m_particleSystem->AddEmitter(shotEffect);
	m_particleSystem->AddEmitter(hitEffect);
	m_particleSystem->AddEmitter(explodeEffect);

	MainMenu_Init();
	PauseMenu_Init();
	GameplayUI_Init();
}
//..............................
void Game::Shutdown()
{
	UnitDefinitions::ClearDefinition();
	TileDefinition::ClearDefinition();
	MapDefinition::ClearDefinition();

	delete m_map;
	m_map = nullptr;

	delete m_menuCanvas;
	m_menuCanvas = nullptr;

	delete m_pauseCanvas;
	m_pauseCanvas = nullptr;

	delete m_gameCanvas;
	m_gameCanvas = nullptr;

	delete m_particleSystem;
	m_particleSystem = nullptr;
}

bool Game::IsNetworkGame() const
{
	return g_theNetwork->IsEnable() && m_isOnlineGame;
}

//----------------------------------------------------------------------------------------------------------------------------------------
// UPDATE

void Game::Update(float deltaSeconds)
{
	if (m_isOnlineGame || g_theNetwork->IsEnable())
	{
		if (m_waitingPanel->IsActive())
		{
			if (g_theInput->WasKeyJustPressed(KEYCODE_ESCAPE))
			{
				m_map->SetPlayerQuit(m_map->GetApplicationPlayerID());

				EventArgs args;
				args.SetValue("command", Stringf("\"PlayerQuit id=%i\"", m_map->GetApplicationPlayerID()));
				g_theNetwork->RemoteCommand(args);
				m_waitingPanel->SetActive(false);
				m_mainMenuPanel->SetActive(true);

				return;
			}
		}
		if (m_map->IsReadyToBeginNetworkGame() && m_isMenu)
		{
			m_isMenu = false;
			m_waitingPanel->SetActive(false);
			m_map->SetPlayerConnected();
			m_map->m_isGameEnded = false;
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESCAPE))
	{
		if (m_isMenu)
		{
			FireEvent("quit");
		}
		else
		{
			if (m_map->IsEveryoneInTheGame() && !m_map->m_isGameEnded)
			{
				if (m_isPaused)
				{
					BackToMenu();
				}
				else
				{
					if (!m_map->m_isPendingEndTurn)
					{
						m_isPaused = true;
					}
				}
			}
		}
	}

	if (m_isMenu)
	{
		m_menuCanvas->Update(deltaSeconds);
	}
	else
	{
		m_particleSystem->Update(deltaSeconds);

		if (g_theInput->WasKeyJustPressed('P'))
		{
			m_isPaused = true;
		}

		if (m_isPaused)
		{
			m_pauseCanvas->Update(deltaSeconds);
		}
		else
		{
			m_map->Update(deltaSeconds);

			m_gameCanvas->Update(deltaSeconds);
		}
	}
}

void Game::Render() const
{
	IntVec2 screenSize = Window::GetMainWindowInstance()->GetClientDimensions();
	float aspectTextWidth = screenSize.x / 1920.f;
	float aspectTextHeight = screenSize.y / 1080.f;
	Vec2 logoExtendedSize = Vec2(250 * aspectTextWidth, 250 * aspectTextHeight);

	if (m_isMenu)
	{
		g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
		g_theRenderer->BeginCamera(m_screenCamera);
		g_theRenderer->SetDepthStencilMode(DepthMode::DISABLED);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		g_theRenderer->SetSamplerMode(SampleMode::POINT_CLAMP);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindShader(nullptr);

		Vec2 center = m_screenCamera.GetOrthographicTopRight() * 0.5f;
		std::vector<Vertex_PCU> logoVert;
		AddVertsForAABB2D(logoVert, AABB2(center - logoExtendedSize, center + logoExtendedSize));

		g_theRenderer->BindTexture(m_logoImage);
		g_theRenderer->DrawVertexArray((int)logoVert.size(), logoVert.data());

		m_menuCanvas->Render();


		g_theRenderer->EndCamera(m_screenCamera);
	}
	else
	{
		g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));

		if (m_isPaused)
		{
			g_theRenderer->BeginCamera(m_screenCamera);
			g_theRenderer->SetDepthStencilMode(DepthMode::DISABLED);
			g_theRenderer->SetBlendMode(BlendMode::ALPHA);
			g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
			g_theRenderer->SetSamplerMode(SampleMode::POINT_CLAMP);
			g_theRenderer->SetModelConstants();
			g_theRenderer->BindShader(nullptr);

			Vec2 center = m_screenCamera.GetOrthographicTopRight() * 0.5f;
			std::vector<Vertex_PCU> logoVert;
			AddVertsForAABB2D(logoVert, AABB2(center - logoExtendedSize, center + logoExtendedSize));

			g_theRenderer->BindTexture(m_logoImage);
			g_theRenderer->DrawVertexArray((int)logoVert.size(), logoVert.data());

			m_pauseCanvas->Render();
		}
		else
		{
			m_map->Render();

			m_particleSystem->Render(m_map->m_camera);

			RenderScreenWorld();

			g_theRenderer->SetDepthStencilMode(DepthMode::DISABLED);
			g_theRenderer->BindShader(nullptr);
			m_gameCanvas->Render();

	

		}
		DebugRenderWorld(*m_map->m_camera);
		DebugRenderScreen(m_screenCamera);
	}
}

void Game::PlayLocalGame()
{
	m_isMenu = false;
	m_isOnlineGame = false;
	m_backToMenuFromGame->SetActive(false);
	m_dialoguePanel->SetActive(false);
	m_map->m_isGameEnded = false;
}

void Game::PlayNetworkGame()
{
	m_isOnlineGame = true;
	m_backToMenuFromGame->SetActive(false);
	m_dialoguePanel->SetActive(false);

	m_map->SetPlayerReady(m_map->GetApplicationPlayerID());
	m_map->m_currentPlayerIDTurn = 1;

	EventArgs args;
	args.SetValue("command", Stringf("\"PlayerReady id=%i\"", m_map->GetApplicationPlayerID()));
	g_theNetwork->RemoteCommand(args);

	m_mainMenuPanel->SetActive(false);
	m_waitingPanel->SetActive(true);
}

void Game::RenderScreenWorld() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	g_theRenderer->SetDepthStencilMode(DepthMode::DISABLED);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theRenderer->SetSamplerMode(SampleMode::POINT_CLAMP);

	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::FirstClick()
{
	m_introFirstClick->SetActive(false);
	m_mainMenuPanel->SetActive(true);
}

void Game::BackToMenu()
{
	m_backToMenuFromGame->SetActive(false);
	m_isMenu = true;
	m_isPaused = false;
	m_mainMenuPanel->SetActive(true);
	m_map->ResetUnitsData();
	m_map->m_currentPlayerIDTurn = 1;

	if (!m_map->m_isGameEnded)
	{
		m_map->SetPlayerQuit(m_map->GetApplicationPlayerID());

		EventArgs args;
		args.SetValue("command", Stringf("\"PlayerQuit id=%i\"", m_map->GetApplicationPlayerID()));
		g_theNetwork->RemoteCommand(args);
	}

}

void Game::MainMenu_Init()
{
	IntVec2 screenSize = Window::GetMainWindowInstance()->GetClientDimensions();
	float aspectTextWidth = screenSize.x / 1920.f;
	float aspectTextHeight = screenSize.y / 1080.f;

	// INTRO
	m_introFirstClick = new Button(m_menuCanvas, AABB2(Vec2::ZERO, Vec2(screenSize)), TextSetting(), Rgba8::COLOR_TRANSPARENT, Rgba8::COLOR_TRANSPARENT, Rgba8::COLOR_TRANSPARENT);
	m_introFirstClick->OnClickEvent([this]() {	FirstClick(); });
	TextSetting introTextSetting = TextSetting("Press ENTER or click to Start");
	introTextSetting.m_color = Rgba8::COLOR_WHITE;
	introTextSetting.m_height = 256.f * aspectTextWidth;
	m_introText = new Text(m_menuCanvas, Vec2(screenSize) * Vec2(0.5f, 0.2f), introTextSetting, m_introFirstClick);

	TextSetting titleTextSetting = TextSetting("Vaporum");
	titleTextSetting.m_color = Rgba8::COLOR_WHITE;
	titleTextSetting.m_height = 512.f * aspectTextWidth;
	m_introTitle = new Text(m_menuCanvas, Vec2(screenSize) * Vec2(0.5f, 0.85f), titleTextSetting, m_introFirstClick);

	// MAIN MENU
	m_mainMenuPanel = new Panel(m_menuCanvas, AABB2(Vec2::ZERO, Vec2(screenSize)), Rgba8::COLOR_TRANSPARENT, false);
	m_mainMenuPanel->SetActive(false);

	TextSetting titleTS = TextSetting("Main Menu");
	titleTS.m_color = Rgba8::COLOR_WHITE;
	titleTS.m_height = 256.f * aspectTextHeight;
	titleTS.m_orientation = 90.f;
	m_menuTitle = new Text(m_menuCanvas, Vec2(screenSize) * Vec2(0.35f, 0.3f), titleTS, m_mainMenuPanel);

	float lineThickness = 10.f;
	m_menuLine = new Panel(m_menuCanvas, AABB2(Vec2(m_menuTitle->GetPosition().x, 0), Vec2(Vec2(m_menuTitle->GetPosition().x + lineThickness, (float)screenSize.y))), Rgba8::COLOR_WHITE, nullptr, false, Rgba8::COLOR_TRANSPARENT, m_mainMenuPanel);

	float BLx_Button = m_menuLine->GetPosition().x + 10.f;
	m_menuTitle->SetTextOffSet(Vec2(screenSize) * Vec2(0.2f, -0.1f));

	if (g_theNetwork->IsEnable())
	{
		TextSetting networkPlayTS = TextSetting("Network Play");
		networkPlayTS.m_color = Rgba8::COLOR_WHITE;
		networkPlayTS.m_height = 64.f * aspectTextWidth;
		m_menuNetworkPlay = new Button(m_menuCanvas, AABB2(BLx_Button, Vec2(screenSize).y * 0.5f, BLx_Button + 650.f, Vec2(screenSize).y * 0.59f), networkPlayTS, Rgba8::COLOR_DARK_GRAY, Rgba8::COLOR_DARK_GREEN, Rgba8::COLOR_WHITE, true, Rgba8::COLOR_BLACK, m_mainMenuPanel);
		m_menuNetworkPlay->m_textColorHover = Rgba8::COLOR_WHITE;
		m_menuNetworkPlay->OnClickEvent([this]() {PlayNetworkGame(); });
	}

	TextSetting localPlayTS = TextSetting("Local Play");
	localPlayTS.m_color = Rgba8::COLOR_WHITE;
	localPlayTS.m_height = 64.f * aspectTextWidth;
	m_menuLocalPlay = new Button(m_menuCanvas, AABB2(BLx_Button, Vec2(screenSize).y * 0.4f, BLx_Button + 650.f, Vec2(screenSize).y * 0.49f), localPlayTS, Rgba8::COLOR_DARK_GRAY, Rgba8::COLOR_DARK_GREEN, Rgba8::COLOR_WHITE, true, Rgba8::COLOR_BLACK, m_mainMenuPanel);
	m_menuLocalPlay->m_textColorHover = Rgba8::COLOR_WHITE;
	m_menuLocalPlay->OnClickEvent([this]() {PlayLocalGame(); });

	TextSetting quitTS = TextSetting("Quit");
	quitTS.m_color = Rgba8::COLOR_WHITE;
	quitTS.m_height = 64.f * aspectTextWidth;
	m_menuQuit = new Button(m_menuCanvas, AABB2(BLx_Button, Vec2(screenSize).y * 0.3f, BLx_Button + 650.f, Vec2(screenSize).y * 0.39f), quitTS, Rgba8::COLOR_DARK_GRAY, Rgba8::COLOR_DARK_GREEN, Rgba8::COLOR_WHITE, true, Rgba8::COLOR_BLACK, m_mainMenuPanel);
	m_menuQuit->m_textColorHover = Rgba8::COLOR_WHITE;
	m_menuQuit->OnClickEvent([this]() {FireEvent("quit"); });

	m_waitingPanel = new Panel(m_menuCanvas, AABB2(Vec2(0, 0), Vec2(screenSize)), Rgba8::COLOR_BLACK, nullptr, false, Rgba8::COLOR_TRANSPARENT, nullptr);
	m_waitingPanel->SetActive(false);
	TextSetting waitingTS = TextSetting("Waiting for the other player...");
	waitingTS.m_color = Rgba8::COLOR_WHITE;
	waitingTS.m_height = 128.f * aspectTextWidth;
	m_waitingText = new Text(m_menuCanvas, Vec2(screenSize) * Vec2(0.5f, 0.5f), waitingTS, m_waitingPanel);
}

void Game::PauseMenu_Init()
{
	IntVec2 screenSize = Window::GetMainWindowInstance()->GetClientDimensions();
	float aspectTextWidth = screenSize.x / 1920.f;
	float aspectTextHeight = screenSize.y / 1080.f;

	m_pauseMenuPanel = new Panel(m_pauseCanvas, AABB2(Vec2::ZERO, Vec2(screenSize)), Rgba8::COLOR_TRANSPARENT, false);

	TextSetting tilteTS = TextSetting("Pause Menu");
	tilteTS.m_color = Rgba8::COLOR_WHITE;
	tilteTS.m_height = 256.f * aspectTextHeight;
	tilteTS.m_orientation = 90.f;
	m_pauseTitle = new Text(m_pauseCanvas, Vec2(screenSize) * Vec2(0.35f, 0.3f), tilteTS, m_pauseMenuPanel);

	float lineThickness = 10.f;
	m_pauseLine = new Panel(m_pauseCanvas, AABB2(Vec2(m_pauseTitle->GetPosition().x, 0), Vec2(Vec2(m_pauseTitle->GetPosition().x + lineThickness, (float)screenSize.y))), Rgba8::COLOR_WHITE, nullptr, false, Rgba8::COLOR_TRANSPARENT, m_pauseMenuPanel);

	float BLx_Button = m_pauseLine->GetPosition().x + 10.f;
	m_pauseTitle->SetTextOffSet(Vec2(screenSize) * Vec2(0.2f, -0.2f));

	TextSetting resumeTS = TextSetting("Resume");
	resumeTS.m_color = Rgba8::COLOR_WHITE;
	resumeTS.m_height = 64.f * aspectTextWidth;
	m_pauseResume = new Button(m_pauseCanvas, AABB2(BLx_Button, Vec2(screenSize).y * 0.5f, BLx_Button + 650.f, Vec2(screenSize).y * 0.59f), resumeTS, Rgba8::COLOR_DARK_GRAY, Rgba8::COLOR_DARK_GREEN, Rgba8::COLOR_WHITE, true, Rgba8::COLOR_BLACK, m_pauseMenuPanel);
	m_pauseResume->m_textColorHover = Rgba8::COLOR_WHITE;
	m_pauseResume->OnClickEvent([this]() {m_isPaused = false; });

	TextSetting quitTS = TextSetting("Main Menu");
	quitTS.m_color = Rgba8::COLOR_WHITE;
	quitTS.m_height = 64.f * aspectTextWidth;
	m_pauseQuit = new Button(m_pauseCanvas, AABB2(BLx_Button, Vec2(screenSize).y * 0.4f, BLx_Button + 650.f, Vec2(screenSize).y * 0.49f), quitTS, Rgba8::COLOR_DARK_GRAY, Rgba8::COLOR_DARK_GREEN, Rgba8::COLOR_WHITE, true, Rgba8::COLOR_BLACK, m_pauseMenuPanel);
	m_pauseQuit->m_textColorHover = Rgba8::COLOR_WHITE;
	m_pauseQuit->OnClickEvent([this]() {BackToMenu(); });
}

void Game::GameplayUI_Init()
{
	Vec2 screenSize = Vec2(Window::GetMainWindowInstance()->GetClientDimensions());
	float aspectTextWidth = screenSize.x / 1920.f;
	float aspectTextHeight = screenSize.y / 1080.f;

	m_gameButtonListPanel = new Panel(m_gameCanvas, AABB2(Vec2(screenSize.x * 0.2f, 0), Vec2(screenSize.x * 0.8f, screenSize.y * 0.15f)), Rgba8::COLOR_BLACK, nullptr, true, Rgba8::COLOR_WHITE);
	Vec2 p = m_gameButtonListPanel->GetPosition();
	float w = m_gameButtonListPanel->GetWidth();
	float h = m_gameButtonListPanel->GetHeight();
	m_LeftSprite = new Sprite(m_gameCanvas, AABB2(p + Vec2(w * -0.45f, h * -0.2f), 25.f * aspectTextHeight, 25.f * aspectTextWidth), m_ui_Left, m_gameButtonListPanel);
	m_LeftText = new Text(m_gameCanvas, m_LeftSprite->GetPosition() + Vec2(w * 0.04f, h * -0.025f), TextSetting("Previous", 64.f * aspectTextWidth, Rgba8::COLOR_WHITE, 0, Vec2(0, 0.5f)), m_LeftSprite);

	m_RightSprite = new Sprite(m_gameCanvas, AABB2(p + Vec2(w * -0.15f, h * -0.2f), 25.f * aspectTextHeight, 25.f * aspectTextWidth), m_ui_Right, m_gameButtonListPanel);
	m_RightText = new Text(m_gameCanvas, m_RightSprite->GetPosition() + Vec2(w * 0.04f, h * -0.025f), TextSetting("Next", 64.f * aspectTextWidth, Rgba8::COLOR_WHITE, 0, Vec2(0, 0.5f)), m_RightSprite);

	m_YSprite = new Sprite(m_gameCanvas, AABB2(p + Vec2(w * 0.25f, h * -0.2f), 25.f * aspectTextHeight, 25.f * aspectTextWidth), m_ui_Y, m_gameButtonListPanel);
	m_YText = new Text(m_gameCanvas, m_YSprite->GetPosition() + Vec2(w * 0.04f, h * -0.025f), TextSetting("End Turn", 64.f * aspectTextWidth, Rgba8::COLOR_WHITE, 0, Vec2(0, 0.5f)), m_YSprite);

	m_LMBSprite = new Sprite(m_gameCanvas, AABB2(p + Vec2(w * -0.45f, h * 0.2f), 25.f * aspectTextHeight, 25.f * aspectTextWidth), m_ui_LMB, m_gameButtonListPanel);
	m_LMBSprite->SetActive(false);
	m_LMBText = new Text(m_gameCanvas, m_LMBSprite->GetPosition() + Vec2(w * 0.04f, h * -0.025f), TextSetting("Select", 64.f * aspectTextWidth, Rgba8::COLOR_WHITE, 0, Vec2(0, 0.5f)), m_LMBSprite);

	m_RMBSprite = new Sprite(m_gameCanvas, AABB2(p + Vec2(w * -0.15f, h * 0.2f), 25.f * aspectTextHeight, 25.f * aspectTextWidth), m_ui_RMB, m_gameButtonListPanel);
	m_RMBSprite->SetActive(false);
	m_RMBText = new Text(m_gameCanvas, m_RMBSprite->GetPosition() + Vec2(w * 0.04f, h * -0.025f), TextSetting("Deselect", 64.f * aspectTextWidth, Rgba8::COLOR_WHITE, 0, Vec2(0, 0.5f)), m_RMBSprite);

	m_p1Panel = new Panel(m_gameCanvas, AABB2(Vec2(0, screenSize.y * 0.85f), Vec2(screenSize.x * 0.27f, screenSize.y)), Rgba8::COLOR_BLACK, nullptr, true, Rgba8::COLOR_WHITE);
	m_p1Panel->SetActive(false);
	TextSetting p1Text = TextSetting("Player 1's turn");
	p1Text.m_color = Rgba8::COLOR_GREEN;
	p1Text.m_height = 100.f * aspectTextWidth;
	m_p1PanelText = new Text(m_gameCanvas, m_p1Panel->GetPosition(), p1Text, m_p1Panel);

	m_p2Panel = new Panel(m_gameCanvas, AABB2(Vec2(screenSize.x * 0.73f, screenSize.y * 0.85f), Vec2(screenSize.x, screenSize.y)), Rgba8::COLOR_BLACK, nullptr, true, Rgba8::COLOR_WHITE);
	m_p2Panel->SetActive(false);
	TextSetting p2Text = TextSetting("Player 2's turn");
	p2Text.m_color = Rgba8::COLOR_GREEN;
	p2Text.m_height = 100.f * aspectTextWidth;
	m_p2PanelText = new Text(m_gameCanvas, m_p2Panel->GetPosition(), p2Text, m_p2Panel);

	m_p1TankInfoPanel = new Panel(m_gameCanvas, AABB2(Vec2(screenSize.x * 0.02f, screenSize.y * 0.02f), Vec2(screenSize.x * 0.18f, screenSize.y * 0.45f)), Rgba8::COLOR_BLACK, nullptr, true, Rgba8::COLOR_WHITE);
	m_p1TankInfoPanel->SetActive(false);
	Vec2 pi1 = m_p1TankInfoPanel->GetPosition();
	float wi1 = m_p1TankInfoPanel->GetWidth();
	float hi1 = m_p1TankInfoPanel->GetHeight();
	m_p1TankInfoSprite = new Sprite(m_gameCanvas, AABB2(pi1 + Vec2(0, hi1 * 0.15f), wi1 * 0.39f, wi1 * 0.39f), m_HadrianLogoImage, m_p1TankInfoPanel);
	m_p1TankInfoTitle = new Text(m_gameCanvas, pi1 + Vec2(0, hi1 * 0.42f), TextSetting("Tank Title", 128.f * aspectTextWidth, Rgba8::COLOR_WHITE), m_p1TankInfoPanel);
	m_p1TankInfoTexts = new Text(m_gameCanvas, pi1 + Vec2(wi1 * -0.46f, hi1 * -0.12f), TextSetting("Attack\nDefense\nRange\nMove\nHealth", 50.f * aspectTextWidth, Rgba8::COLOR_WHITE, 0, Vec2(0, 0.5f), 17.f * aspectTextHeight), m_p1TankInfoPanel);
	m_p1TankInfoStats = new Text(m_gameCanvas, pi1 + Vec2(wi1 * 0.46f, hi1 * -0.12f), TextSetting("9\n9\n9-9\n9\n9", 50.f * aspectTextWidth, Rgba8::COLOR_WHITE, 0, Vec2(1, 0.5f), 17.f * aspectTextHeight), m_p1TankInfoPanel);

	m_p2TankInfoPanel = new Panel(m_gameCanvas, AABB2(Vec2(screenSize.x * 0.82f, screenSize.y * 0.02f), Vec2(screenSize.x * 0.98f, screenSize.y * 0.45f)), Rgba8::COLOR_BLACK, nullptr, true, Rgba8::COLOR_WHITE);
	m_p2TankInfoPanel->SetActive(false);
	Vec2 pi2 = m_p2TankInfoPanel->GetPosition();
	float wi2 = m_p2TankInfoPanel->GetWidth();
	float hi2 = m_p2TankInfoPanel->GetHeight();
	m_p2TankInfoSprite = new Sprite(m_gameCanvas, AABB2(pi2 + Vec2(0, hi2 * 0.15f), wi2 * 0.39f, wi2 * 0.39f), m_HadrianLogoImage, m_p2TankInfoPanel);
	m_p2TankInfoTitle = new Text(m_gameCanvas, pi2 + Vec2(0, hi2 * 0.42f), TextSetting("Tank Title", 128.f * aspectTextWidth, Rgba8::COLOR_WHITE), m_p2TankInfoPanel);
	m_p2TankInfoTexts = new Text(m_gameCanvas, pi2 + Vec2(wi2 * -0.46f, hi2 * -0.12f), TextSetting("Attack\nDefense\nRange\nMove\nHealth", 50.f * aspectTextWidth, Rgba8::COLOR_WHITE, 0, Vec2(0, 0.5f), 17.f * aspectTextHeight), m_p2TankInfoPanel);
	m_p2TankInfoStats = new Text(m_gameCanvas, pi2 + Vec2(wi2 * 0.46f, hi2 * -0.12f), TextSetting("9\n9\n9-9\n9\n9", 50.f * aspectTextWidth, Rgba8::COLOR_WHITE, 0, Vec2(1, 0.5f), 17.f * aspectTextHeight), m_p2TankInfoPanel);

	m_dialoguePanel = new Panel(m_gameCanvas, AABB2(Vec2(screenSize.x * 0.35f, screenSize.y * 0.35f), Vec2(screenSize.x * 0.65f, screenSize.y * 0.65f)), Rgba8::COLOR_BLACK, nullptr, true, Rgba8::COLOR_WHITE);
	m_dialoguePanel->SetActive(false);
	Vec2 pd = m_dialoguePanel->GetPosition();
	float hd = m_dialoguePanel->GetHeight();

	TextSetting dtitleTS("Player 0's Turn", 128.f * aspectTextWidth, Rgba8::COLOR_WHITE, 0, Vec2(0.5f, 0.5f));
	m_dialogueTitle = new Text(m_gameCanvas, pd + Vec2(0, hd * 0.35f), dtitleTS, m_dialoguePanel);
	TextSetting ddetailTS("Press ENTER or Click to continue\nPress ESCAPE to cancel", 50.f * aspectTextWidth, Rgba8::COLOR_WHITE, 0, Vec2(0.5f, 0.5f), 17.f);
	m_dialogueDetail = new Text(m_gameCanvas, pd + Vec2(0, hd * -0.25f), ddetailTS, m_dialoguePanel);

	m_backToMenuFromGame = new Button(m_gameCanvas, AABB2(Vec2(0, 0), Vec2(screenSize)), TextSetting(), Rgba8::COLOR_TRANSPARENT, Rgba8::COLOR_TRANSPARENT, Rgba8::COLOR_TRANSPARENT, false, Rgba8::COLOR_TRANSPARENT, nullptr);
	m_backToMenuFromGame->SetActive(false);
	m_backToMenuFromGame->OnClickEvent([this]() {BackToMenu(); });
}