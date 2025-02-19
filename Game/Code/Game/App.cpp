#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/Map.hpp"


App* g_theApp = nullptr;
RandomNumberGenerator* g_theRNG = nullptr;
Renderer* g_theRenderer = nullptr;
AudioSystem* g_theAudio = nullptr;
Window* g_theWindow = nullptr;
UISystem* g_UI = nullptr;

App::App()
{

}

App::~App()
{

}

void App::Startup(const char* commandLine)
{
	InitializeGameConfig("Data/GameConfig.xml");

	Clock::s_theSystemClock = new Clock();

	EventSystemConfig eventConfig;
	g_theEventSystem = new EventSystem(eventConfig);
	SubscribeEventCallbackFunction("quit", App::Event_Quit);
	SubscribeEventCallbackFunction("LoadGameConfig", App::Event_LoadGameConfig);

	std::string cmdLineString = std::string(commandLine);
	Strings cmdSplits = SplitStringOnDelimiter(cmdLineString, " ");
	if (cmdSplits.size() == 2)
	{
		EventArgs args;
		Strings keyValuePair = SplitStringOnDelimiter(cmdSplits[1], '=', true);

		if (keyValuePair.size() == 2)
		{
			args.SetValue(keyValuePair[0], keyValuePair[1]);
		}
		FireEvent(cmdSplits[0], args);
	}

	g_theRNG = new RandomNumberGenerator();
	InputConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_windowTitle = g_gameConfigBlackboard.GetValue("windowTitle", "Vaporum");
	windowConfig.m_isFullscreen = g_gameConfigBlackboard.GetValue("windowFullscreen", true);
	windowConfig.m_size = g_gameConfigBlackboard.GetValue("windowSize", IntVec2(1200, 600));
	windowConfig.m_pos = g_gameConfigBlackboard.GetValue("windowPosition", IntVec2(100, 50));
	g_theWindow = new Window(windowConfig);

	RendererConfig renderConfig;
	renderConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(renderConfig);

	AudioConfig audioConfig;
	g_theAudio = new AudioSystem(audioConfig);

	DevConsoleConfig consoleConfig;
	consoleConfig.m_renderer = g_theRenderer;
	consoleConfig.m_fontFilePath = "Data/Fonts/RobotoMonoSemiBold64";
	consoleConfig.m_camera = new Camera();
	g_theDevConsole = new DevConsole(consoleConfig);

	NetworkConfig networkConfig;
	networkConfig.m_modeString = g_gameConfigBlackboard.GetValue("netMode", "");
	networkConfig.m_hostAddressString = g_gameConfigBlackboard.GetValue("netHostAddress", "");
	networkConfig.m_sendBufferSize = g_gameConfigBlackboard.GetValue("netSendBufferSize", 2048);
	networkConfig.m_recvBufferSize = g_gameConfigBlackboard.GetValue("netRecvBufferSize", 2048);
	g_theNetwork = new NetWorkSystem(networkConfig);

	m_game = new Game();

	DebugRenderConfig debugrenderConfig;
	debugrenderConfig.m_renderer = g_theRenderer;

	Clock::s_theSystemClock->TickSystemClock();
	g_theEventSystem->Startup();
	g_theInput->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	DebugRenderSystemStartUp(debugrenderConfig);
	g_theAudio->Startup();
	g_theDevConsole->Startup();
	g_theNetwork->Startup();

	BitmapFont* font32 = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/RobotoMonoSemiBold32");
	BitmapFont* font64 = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/RobotoMonoSemiBold64");
	BitmapFont* font128 = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/RobotoMonoSemiBold128");

	UIConfig uiConfig;
	uiConfig.m_window = g_theWindow;
	uiConfig.m_renderer = g_theRenderer;
	uiConfig.m_inputSystem = g_theInput;
	uiConfig.m_font = font128;
	g_UI = new UISystem(uiConfig);
	g_UI->AddFont(font32);
	g_UI->AddFont(font64);
	g_UI->Startup();

	m_game->Startup();

	ConsoleTutorial();
}

bool App::Event_Quit(EventArgs& args)
{
	UNUSED(args);

	EventArgs arg;
	arg.SetValue("command", Stringf("\"PlayerQuit id=%i\"", g_theApp->m_game->m_map->GetApplicationPlayerID()));
	g_theNetwork->RemoteCommand(arg);

	g_theApp->HandleQuitRequested();
	return false;
}

bool App::Event_LoadGameConfig(EventArgs& args)
{
	XmlDocument file;
	std::string filePath = args.GetValue("File", "");
	XmlError result = file.LoadFile(filePath.c_str());
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "FILE IS NOT LOADED");

	XmlElement* element = file.FirstChildElement();

	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*element, true);
	return false;
}

void App::Shutdown()
{
	m_game->Shutdown();
	g_UI->Shutdown();
	g_theNetwork->Shutdown();
	g_theDevConsole->Shutdown();
	g_theAudio->Shutdown();
	DebugRenderSystemShutdown();
	g_theRenderer->Shutdown();
	g_theWindow->Startup();
	g_theInput->Shutdown();
	g_theEventSystem->Shutdown();

	delete g_theRNG;
	delete m_game;
	m_game = nullptr;
	delete g_theNetwork;
	g_theNetwork = nullptr;
	delete g_theDevConsole;
	g_theDevConsole = nullptr;
	delete g_theAudio;
	g_theAudio = nullptr;
	delete g_theRenderer;
	g_theRenderer = nullptr;
	delete g_theWindow;
	g_theWindow = nullptr;
	delete g_theInput;
	g_theInput = nullptr;
	delete g_theEventSystem;
	g_theEventSystem = nullptr;

}

void App::Run()
{
	while (!m_isQuitting)
	{
		RunFrame();
	}
}

void App::RunFrame()
{
	BeginFrame();
	Update(m_game->m_clock->GetDeltaSeconds());
	Render();
	EndFrame();
}

bool App::HandleQuitRequested()
{
	m_isQuitting = true;
	return 0;
}

void App::BeginFrame()
{
	Clock::s_theSystemClock->TickSystemClock();
	g_theEventSystem->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	DebugRenderBeginFrame();
	g_theAudio->BeginFrame();
	g_theDevConsole->BeginFrame();
	g_theNetwork->BeginFrame();
	g_UI->BeginFrame();
}

void App::Update(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_TIDLE))
	{
		g_theDevConsole->ToggleOpen();
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_game->Shutdown();
		delete m_game;
		m_game = new Game();
		m_game->Startup();
	}
	if (g_theInput->IsKeyDown('T'))
	{
		m_game->m_clock->SetTimeScale(0.1f);
	}
	if (g_theInput->WasKeyJustReleased('T'))
	{
		m_game->m_clock->SetTimeScale(1.f);
	}


	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_game->m_clock->StepSingleFrame();
	}

	m_game->Update(deltaSeconds);
}

void App::Render() const
{
	m_game->Render();
	AABB2 screenBound(0.f, 0.f, (float)g_theWindow->GetClientDimensions().x, (float)g_theWindow->GetClientDimensions().y);
	g_theDevConsole->Render(screenBound, g_theRenderer);
}

void App::EndFrame()
{
	g_theEventSystem->EndFrame();
	g_theInput->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();
	DebugRenderEndFrame();
	g_theAudio->EndFrame();
	g_theDevConsole->EndFrame();
	g_theNetwork->EndFrame();
	g_UI->EndFrame();
}

void App::ConsoleTutorial()
{
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Type help to see the list of events");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "DEBUG KEY: B to toggle Debug");
}

void App::InitializeGameConfig(const char* filePath)
{
	XmlDocument file;
	XmlError result = file.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "FILE IS NOT LOADED");

	XmlElement* element = file.FirstChildElement();

	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*element);
}