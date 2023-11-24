#include <QScreen>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QRadioButton>
#include <QMouseEvent>
#include <Windows.h>
#include "container.h"

#include <QMdiSubwindow>

using namespace Qt::Literals::StringLiterals;

DEVMODEW originalDisplayMode;

Container::Container(QWidget *parent) : QMainWindow(parent),
	userInput(false),
	screenGeometry(screen()->geometry()),
	launchPanel(new QFrame(this)),
	gameListColumn(new QGroupBox(this)),
	gameListColumnLayout(nullptr),
	gameSelection(new QButtonGroup(this)),
	viewport(new QStackedWidget(this)),
	launchColumn(new QFrame(this)),
	steamLibraryLocation(new QLineEdit(this)),
	launchButton(nullptr),
	selectedGame(nullptr)
{
	setCentralWidget(viewport);
	BuildUI();

	originalDisplayMode.dmSize=sizeof(DEVMODE);
	originalDisplayMode.dmDriverExtra=0;
	EnumDisplaySettingsW(NULL,ENUM_CURRENT_SETTINGS,&originalDisplayMode);
	originalDisplayMode.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

	connect(launchButton,&QPushButton::pressed,this,&Container::LaunchSelectedGame);
	connect(steamLibraryLocation,&QLineEdit::textChanged,this,&Container::SteamLibraryLocationUpdated);
	connect(screen(),&QScreen::geometryChanged,this,&Container::DisplayChanged);
}

Container::~Container()
{
}

void Container::showEvent(QShowEvent *event)
{
	resize(1920,1080);
}

void Container::resizeEvent(QResizeEvent *event)
{
	if (!userInput) Center();
}

void Container::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) userInput=true;
}

void Container::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) userInput=false;
}

void Container::BuildUI()
{
	viewport->addWidget(launchPanel);
	QHBoxLayout *panelLayout=new QHBoxLayout(launchPanel);
	panelLayout->setContentsMargins(0,0,0,0);
	launchPanel->setLayout(panelLayout);

	gameListColumnLayout=new QVBoxLayout(gameListColumn);
	gameListColumnLayout->addStretch();
	gameListColumn->setLayout(gameListColumnLayout);
	QScrollArea *scroll=new QScrollArea(this);
	scroll->setWidgetResizable(true);
	scroll->setWidget(gameListColumn);
	panelLayout->addWidget(scroll);

	QVBoxLayout *launchLayout=new QVBoxLayout(launchColumn);
	launchColumn->setLayout(launchLayout);
	panelLayout->addWidget(launchColumn);
	QGroupBox *steamGroup=new QGroupBox("Steam",this);
	launchLayout->addWidget(steamGroup);

	QVBoxLayout *steamLayout=new QVBoxLayout(steamGroup);
	steamGroup->setLayout(steamLayout);
	steamLayout->addWidget(LineSizePolicy(new QLabel("Library Location")));
	steamLayout->addWidget(LineSizePolicy(steamLibraryLocation));

	launchButton=new QPushButton("Launch",launchColumn);
	launchLayout->addWidget(launchButton);

	launchLayout->addStretch();
}

void Container::AddEntry(std::shared_ptr<Game> game)
{
	if (!availableGames.contains(game->Name()))
	{
		connect(game.get(),&Game::Launched,this,&Container::CaptureSelectedGame,Qt::UniqueConnection);
		connect(game.get(),&Game::Captured,this,&Container::EmbedSelectedGameWindow,Qt::UniqueConnection);
		connect(this,&Container::DisplayRestored,game.get(),&Game::DisplaySettingsRestored);
		availableGames.insert({game->Name(),game});
		QRadioButton *button=new QRadioButton(game->Name(),gameListColumn);
		connect(button,&QRadioButton::clicked,this,&Container::SelectedGameChanged);
		gameSelection->addButton(button);
		gameListColumnLayout->insertWidget(gameListColumnLayout->count()-1,LineSizePolicy(button));
	}
}

void Container::Center()
{
	//move(screenGeometry.center()-frameGeometry().center());
	//move(50,50);
}

QWidget* Container::LineSizePolicy(QWidget *widget)
{
	widget->setSizePolicy({QSizePolicy::Expanding,QSizePolicy::Fixed});
	return widget;
}

void Container::ValidText(QWidget *widget,bool valid)
{
	if (valid)
		widget->setStyleSheet("color: palette(window-text);");
	else
		widget->setStyleSheet("color: red;");
}

void Container::SteamLibraryLocationUpdated(const QString &location)
{
	QDir folder{location};
	if (folder.exists())
	{
		QFileInfoList entries=folder.entryInfoList({"*.acf"});
		if (!entries.isEmpty())
		{
			QStringList steamIDs;
			QString launcherPath;
			for (const QFileInfo entry : entries)
			{
				QFile file{entry.absoluteFilePath()};
				if (file.open(QIODevice::ReadOnly))
				{
					QString contents=file.readAll();
					QRegularExpression expression(R"#((?<=\"appid\"\t\t\")\d+(?=\"))#");
					QRegularExpressionMatch match=expression.match(contents);
					if (match.hasMatch())
					{
						if (launcherPath.isNull())
						{
							QRegularExpression expression(R"#((?<=\"LauncherPath\"\t\t\").+(?=\"))#");
							QRegularExpressionMatch match=expression.match(contents);
							if (!match.hasMatch()) continue;
							launcherPath=match.captured();
						}
						steamIDs.append(match.captured());
					}
				}
			}
			if (!steamIDs.isEmpty())
			{
				std::unordered_map<QString,std::shared_ptr<Game>> games;
				for (std::shared_ptr<Game> game : Game::games)
				{
					QString id=game->SteamID();
					if (id.isNull()) continue;
					games.insert({id,game});
				}
				for (const QString &id : steamIDs)
				{
					if (auto candidate=games.find(id); candidate != games.end())
					{
						std::shared_ptr<Game> game=candidate->second;
						game->SteamLauncherPath(launcherPath);
						AddEntry(game);
					}
				}
			}
			ValidText(steamLibraryLocation,true);
			return;
		}
	}
	ValidText(steamLibraryLocation,false);
}

void Container::SelectedGameChanged(bool checked)
{
	if (!checked) return;
	QAbstractButton *button=gameSelection->checkedButton();
	if (button) selectedGame=availableGames.at(button->text());
}

void Container::LaunchSelectedGame()
{
	originalGeometry=geometry();
	if (selectedGame) selectedGame->Launch();
}

void Container::CaptureSelectedGame()
{
	selectedGame->Capture();
}

void Container::EmbedSelectedGameWindow(QWindow *window)
{
	qDebug() << window->surfaceType();
	QWidget *containerWidget=QWidget::createWindowContainer(window,viewport);
	if (!containerWidget) return; // TODO: alert the user here
	viewport->addWidget(containerWidget);
	viewport->setCurrentWidget(containerWidget);
	setGeometry(originalGeometry);
}

void Container::DisplayChanged(const QRect &geometry)
{
	if (geometry.width() != originalDisplayMode.dmPelsWidth || geometry.height() != originalDisplayMode.dmPelsHeight)
	{
		ChangeDisplaySettingsW(&originalDisplayMode,0);
	}
	else
	{
		emit DisplayRestored();
		setGeometry(originalGeometry);
	}
}