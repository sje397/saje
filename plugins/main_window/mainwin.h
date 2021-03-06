#ifndef MAINWIN_H
#define MAINWIN_H

#include <QMainWindow>
#include "ui_mainwin.h"
#include <icons_i.h>
#include <menus_i.h>
#include <QPointer>
#include <QSystemTrayIcon>
#include <QPoint>
#include "mainwinoptions.h"

class MainWin : public QMainWindow
{
	Q_OBJECT

public:
	MainWin(CoreI *core, QWidget *parent = 0);
	virtual ~MainWin();

	void add_window(QWidget *w);
	void manage_window_position(QWidget *w);
	void restoreHiddenState();

	void modules_loaded();
	void set_options(MainWinOptions::Settings settings);

public slots:
	void ensureOnScreen(int);
	void toggleHidden();
	void quit();

protected:
	void mouseMoveEvent(QMouseEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void resizeEvent(QResizeEvent *e);
	void paintEvent(QPaintEvent *e);
	void closeEvent(QCloseEvent *e);
	void showEvent(QShowEvent *e);

	bool eventFilter(QObject *target, QEvent *e);

	CoreI *core_i;
	QPointer<IconsI> icons_i;
	QPointer<MenusI> menus_i;

	QMenu *winMenu;

	QAction *sepAction;
	QSystemTrayIcon *systray;

	bool mousePressed;
	QPoint cursorOffset;
	bool closeToTray, hideFrame, toolWindow, roundCorners, onTop;
	void updateFlags();

	bool isObscured();

protected slots:
	void systrayActivated(QSystemTrayIcon::ActivationReason reason);

private:
	Ui::MainWinClass ui;
};

#endif // MAINWIN_H
