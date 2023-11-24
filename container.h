#pragma once

#include <QMainWindow>
#include <QVBoxLayout>
#include <QFrame>
#include <QGroupBox>
#include <QButtonGroup>
#include <QPushButton>
#include <QLineEdit>
#include <QStackedWidget>
#include <Windows.h>
#include "game.h"

class Container : public QMainWindow
{
Q_OBJECT
public:
	Container(QWidget *parent=nullptr);
	~Container();
protected:
	void showEvent(QShowEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	DEVMODEA mode;
private:
	bool userInput;
	std::vector<QString> steamIDs;
	QRect screenGeometry;
	QFrame *launchPanel;
	QGroupBox *gameListColumn;
	QVBoxLayout *gameListColumnLayout;
	QButtonGroup *gameSelection;
	QStackedWidget *viewport;
	QFrame *launchColumn;
	QLineEdit *steamLibraryLocation;
	QPushButton *launchButton;
	std::shared_ptr<Game> selectedGame;
	std::unordered_map<QString,std::shared_ptr<Game>> availableGames;
	QMetaObject::Connection displayChanged;
	QRect originalGeometry;
	void BuildUI();
	void AddEntry(std::shared_ptr<Game> game);
	void Center();
	QWidget* LineSizePolicy(QWidget *widget);
	void ValidText(QWidget *widget,bool valid);
signals:
	void DisplayRestored();
private slots:
	void SteamLibraryLocationUpdated(const QString &location);
	void SelectedGameChanged(bool checked);
	void LaunchSelectedGame();
	void CaptureSelectedGame();
	void EmbedSelectedGameWindow(QWindow *window);
	void DisplayChanged(const QRect &geometry);
};
