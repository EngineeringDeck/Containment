#include <Windows.h>
#include "game.h"

using namespace Qt::Literals::StringLiterals;

const std::vector<std::shared_ptr<Game>> Game::games={
	std::make_shared<PixelJunk_Eden>(),
	std::make_shared<RhemIIISE>()
};

Game::Game() : QObject(nullptr), captureDelay(0), window(nullptr)
{
	connect(&process,&QProcess::started,this,&Game::FindingWindow,Qt::QueuedConnection);
	connect(this,&Game::FindingWindow,this,&Game::FindWindow,Qt::QueuedConnection);
}

const QString& Game::Name() const
{
	return name;
}

const QString& Game::SteamID() const
{
	return steamID;
}

void Game::SteamLauncherPath(const QString &path)
{
	process.setProgram(path);
	process.setArguments({"steam://rungameid/"+steamID});
}

void Game::CaptureDelay(int delay)
{
	captureDelay=delay;
}

std::optional<QSize> Game::IdealDimensions()
{
	return std::nullopt;
}

void Game::FindWindow()
{
	HWND handle=nullptr;
	handle=FindWindowExW(NULL,NULL,NULL,reinterpret_cast<const wchar_t*>(WindowTitle().utf16()));
	if (!handle)
	{
		emit FindingWindow();
		return;
	}
	window=QWindow::fromWinId(reinterpret_cast<long long>(handle));
	if (window)
		emit Launched();
	else
		emit Failed();
}

PixelJunk_Eden::PixelJunk_Eden()
{
	captureDelay=2000;
	name=u"PixelJunk Eden"_s;
	steamID=u"105800"_s;
}

QString PixelJunk_Eden::WindowTitle() const
{
	return name;
}

void PixelJunk_Eden::Launch()
{
	process.start();
}

void PixelJunk_Eden::Capture()
{
	// grabbing the window immediately disrupts gameplay
	// also not all of the child windows have been created
	// I'm guessing there is a need to give it time to initialize the OpenGL context
	QTimer::singleShot(captureDelay,[this]() {
		// there must be some forking going on
		// the process that starts the game is not the one that owns game window
		// so find the process for the original window...
		/*DWORD processID=0;
		if (!GetWindowThreadProcessId(reinterpret_cast<HWND>(window->winId()),&processID))
		{
			emit Failed();
			return;
		}

		// ...then find all of the windows that belong to the process
		HWND candidateWindowHandle=nullptr;
		do
		{
			candidateWindowHandle=FindWindowEx(NULL,candidateWindowHandle,NULL,NULL);
			DWORD candidateWindowProcessID=0;
			if (!GetWindowThreadProcessId(candidateWindowHandle,&candidateWindowProcessID)) continue;
			if (candidateWindowProcessID == processID)
			{
				// the game creates a series of small windows < 120 pixels, so it's safe to assume
				// the window we're looking for is going to be bigger than all of them, so look for
				// the widest window
				QWindow *candidateWindow=QWindow::fromWinId(reinterpret_cast<long long>(candidateWindowHandle));
				if (!candidateWindow) continue;
				if (candidateWindow->width() > window->width()) window=candidateWindow;// for some reason QWindow's size calculation is more accurate than GetWindowRect
			}
		}
		while (candidateWindowHandle);*/
		emit Captured(window);
	});
}

void PixelJunk_Eden::DisplaySettingsRestored()
{
	/*emit Captured(window);
	capturing=false;*/
}


RhemIIISE::RhemIIISE()
{
	captureDelay=1000;
	name=u"RHEM III SE"_s;
	steamID=u"1527930"_s;
}

QString RhemIIISE::WindowTitle() const
{
	return u"RhemIIISE"_s;
}

std::optional<QSize> RhemIIISE::IdealDimensions()
{
	return {{800,600}};
}

void RhemIIISE::Launch()
{
	process.start();
}

void RhemIIISE::Capture()
{
	// in a consistently weird design decision, Rhem places a giant 20,000 x 20,000 black window over the desktop
	// then puts the game over it, so shrink that window and attempt to destroy it, then capture the game
	window->resize(1,1);
	window->close();
	QTimer::singleShot(captureDelay,[this]() {
		HWND handle=nullptr;
		handle=FindWindowExW(NULL,NULL,NULL,reinterpret_cast<const wchar_t*>(WindowTitle().utf16()));
		if (!handle)
		{
			emit Failed();
			return;
		}
		window=QWindow::fromWinId(reinterpret_cast<long long>(handle));
		if (window)
			emit Captured(window);
		else
			emit Failed();
	});
}
