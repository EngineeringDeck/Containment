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
	virtual std::optional<QSize> IdealDimensions() const;
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
	virtual QString WindowTitle() const=0;
signals:
	void Launched();
	void Captured(QWindow *window);
	void FindingWindow();
	void Failed();
public slots:
	virtual void DisplaySettingsRestored() { }
protected slots:
	virtual void FindWindow();
};

class PixelJunk_Eden : public Game
{
	Q_OBJECT
public:
	PixelJunk_Eden();
	void Launch() override;
	void Capture() override;
protected:
	QString WindowTitle() const override;
public slots:
	void DisplaySettingsRestored() override;
};

class RhemSE : public Game
{
	Q_OBJECT
public:
	RhemSE();
	std::optional<QSize> IdealDimensions() const override;
protected:
	void Launch() override;
	void Capture() override;
};

class RhemISE : public RhemSE
{
	Q_OBJECT
public:
	RhemISE();
protected:
	QString WindowTitle() const override;
};

class RhemIISE : public RhemSE
{
	Q_OBJECT
public:
	RhemIISE();
protected:
	QString WindowTitle() const override;
};

class RhemIIISE : public RhemSE
{
	Q_OBJECT
public:
	RhemIIISE();
protected:
	QString WindowTitle() const override;
};

class RhemIVSE : public RhemSE
{
	Q_OBJECT
public:
	RhemIVSE();
protected:
	QString WindowTitle() const override;
};