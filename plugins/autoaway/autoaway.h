#ifndef __AUTOAWAY_H
#define __AUTOAWAY_H

#include <options_i.h>
#include <accounts_i.h>
#include <autoaway_i.h>
#include <events_i.h>
#include <message_window_i.h>
#include <clist_i.h>
#include "autoawayoptions.h"
#include <QTimer>
#include <QPoint>
#include <QDateTime>
#include <QMap>

class AutoAway: public AutoAwayI, EventsI::EventListener {
	Q_OBJECT

public:
	AutoAway();
	~AutoAway();

	bool load(CoreI *core);
	bool modules_loaded();
	bool pre_shutdown();
	bool unload();
	const PluginInfo &get_plugin_info();

        bool event_fired(EventsI::Event &e);

protected slots:
	void options_applied();
	void checkIdle();

	void goIdle();
	void returnFromIdle();

protected:
	CoreI *core_i;
	QPointer<OptionsI> options_i;
	QPointer<AccountsI> accounts_i;
	QPointer<EventsI> events_i;

	AutoAwayOptions *opt;
	AutoAwayOptions::Settings current_settings;
	QTimer timer;

	QPoint last_mouse_pos;
	QDateTime idle_time;
	bool idle;

	QMap<Account *, GlobalStatus> saved_status;
};

#endif // AUTOAWAY

