#pragma once

#include <QString>
#include <QProcess>
#include <QWindow>
#include <QTimer>

class Game : public QObject
{
	Q_OBJECT
public:
	Game();
	const QString& Name() const;
	const QString& SteamID() const;
	void SteamLauncherPath(const QString &path);
	void CaptureDelay(int delay);
	virtual void Launch()=0;
	virtual void Capture()=0;
	static const std::vector<std::shared_ptr<Game>> games;
	static QString steamLocation;
protected:
	bool steamLaunchEnabled;
	int captureDelay;
	int locateWindowAttempts;
	QString name;
	QString steamID;
	QString steamLauncherPath;
	QProcess process;
	QWindow *window;
signals:
	void Launched();
	void Captured(QWindow *window);
	void FindingWindow();
	void Failed();
public slots:
	virtual void DisplaySettingsRestored() { }
protected slots:
	void FindWindow();
};

class PixelJunk_Eden : public Game
{
	Q_OBJECT
public:
	PixelJunk_Eden();
	void Launch() override;
	void Capture() override;
protected:
	bool capturing;
public slots:
	void DisplaySettingsRestored() override;
};