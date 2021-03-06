#ifndef SPLITTERWIN_H
#define SPLITTERWIN_H

#include <QSplitter>
#include "ui_splitterwin.h"
#include <contact_info_i.h>
#include <events_i.h>
#include <QList>
#include <QWebPage>
#include <QUrl>
#include <QTimer>
#include <QHideEvent>
#include <QShowEvent>
#include <QCloseEvent>

class SplitterWin : public QSplitter { 
	Q_OBJECT

public:
	SplitterWin(Contact *contact, EventsI *events_i, QWidget *parent = 0);
	~SplitterWin();

	void addEvents(QList<Message> &events);
public slots:
	void msgEvent(Message &m);
	void msgSend(const QString &msg);

	void setLogStyleSheet(const QString &styleSheet);

	void update_title();

	void setContactChatState(ChatStateType state);

	void setSendChatState(bool f);

signals:
	void closed(Contact *contact);

protected:
	QString format_text(Message &m);
	void update_log();
	QString getNick();
	QString getContent();
	void addToLog(Message &m);

	void showEvent(QShowEvent *e);
	void hideEvent(QHideEvent *e);
	void closeEvent(QCloseEvent *e);

	void setUserChatState(ChatStateType state);
	//void linkUrls(QString &s);

protected slots:
	void scrollLogToBottom();
	void openLink(const QUrl &url);
	
	// chat state
	void active();
	void composing();
	void paused();
	void inactive();
	void gone();

private:
	Ui::SplitterWinClass ui;
	Contact *contact;
	QString timestamp(QDateTime &dt);
	QPointer<EventsI> events_i;
	bool showDate, showTime, showNick;

	QList<Message> content;
	QString style;

	ChatStateType chatState, contactChatState;
	QTimer pauseTimer;
	QTimer inactiveTimer;

	bool sendChatState;
};

#endif // SPLITTERWIN_H
