#include "history.h"
#include <QtPlugin>
#include <QDebug>
#include <QSqlError>
#include <QCryptographicHash>

#define DB_FILE_NAME		"message_history.db"

double timestamp_encode(const QDateTime &d) {
	return d.toTime_t() + d.time().msec() / 1000.0;
}

QDateTime timestamp_decode(const double val) {
	uint secs = (uint)val;
	int msecs = (int)((val - secs) * 1000 + 0.5);
	return QDateTime::fromTime_t(secs).addMSecs(msecs);
}

PluginInfo info = {
	0x280,
	"History",
	"Scott Ellis",
	"mail@scottellis.com.au",
	"http://www.scottellis.com.au",
	"History",
	0x00000001
};

History::History()
{

}

History::~History()
{

}

bool History::load(CoreI *core) {
	core_i = core;
	if((events_i = (EventsI *)core_i->get_interface(INAME_EVENTS)) == 0) return false;
	if((contact_info_i = (ContactInfoI*)core_i->get_interface(INAME_CONTACTINFO)) == 0) return false;
	if((accounts_i = (AccountsI*)core_i->get_interface(INAME_ACCOUNTS)) == 0) return false;

	events_i->add_event_filter(this, 0x100, UUID_MSG);

	db = QSqlDatabase::addDatabase("QSQLITE", "History");
	db.setDatabaseName(core_i->get_config_dir() + "/" + DB_FILE_NAME);
	if(!db.open()) return false;

	QSqlQuery q(db);

	if(!q.exec("CREATE TABLE message_history ("
		"  contact_hash_id varchar(256),"
		"  timestamp number,"
		"  incomming boolean,"
		"  msg_read boolean,"
		"  message text);"))
	{
		qWarning() << "History db error:" << q.lastError().text();
	}

	return true;
}

bool History::modules_loaded() {
	QSqlQuery unread(db);
	if(!unread.exec("SELECT contact_hash_id, message, timestamp FROM message_history WHERE incomming='true' AND msg_read='false';"))
		qWarning() << "History read unread failed:" << unread.lastError().text();

	while(unread.next()) {
		Contact *contact = contact_info_i->get_contact(unread.value(0).toString());
		if(contact) {
			ContactChanged cc(contact, this);
			events_i->fire_event(cc);
			Message m(contact, unread.value(1).toString(), true, 0, this);
			m.timestamp = timestamp_decode(unread.value(2).toDouble());
			events_i->fire_event(m);
		}
	}
	return true;
}

bool History::pre_shutdown() {
	events_i->remove_event_listener(this, UUID_MSG);
	return true;
}

bool History::unload() {
	if(db.isOpen())
		db.close();
	return true;
}

const PluginInfo &History::get_plugin_info() {
	return info;
}

/////////////////////////////

bool History::event_fired(EventsI::Event &e) {
	if(e.uuid == UUID_MSG) {
		Message &m = static_cast<Message &>(e);
		double t = timestamp_encode(m.timestamp);
		if(m.source != this) {
			if(m.contact->has_property("DisableHistory") && m.contact->get_property("DisableHistory").toBool() == true)
				return true;

			QSqlQuery writeMessageQuery(db);
			writeMessageQuery.prepare("INSERT INTO message_history VALUES(?, ?, ?, ?, ?);");

			writeMessageQuery.addBindValue(m.contact->hash_id);
			writeMessageQuery.addBindValue(t);
			writeMessageQuery.addBindValue(m.type == EventsI::ET_INCOMING);
			writeMessageQuery.addBindValue(m.read);
			writeMessageQuery.addBindValue(m.text);

			if(!writeMessageQuery.exec()) {
				qWarning() << "History write failed:" << writeMessageQuery.lastError().text();
			}

		}

		if(!m.read) {
			if(m.contact->has_property("PendingMsg")) {
				m.contact->set_property("PendingMsg", m.contact->get_property("PendingMsg").toList() << t);
			} else {
				m.contact->mark_transient("PendingMsg");
				m.contact->set_property("PendingMsg", QList<QVariant>() << t);
				ContactChanged cc(m.contact, this);
				events_i->fire_event(cc);
			}
		}
	}
	return true;
}

QList<Message> History::read_history(QSqlQuery &query, bool mark_read, const QString &queryText) {
	if(!query.exec()) {
		qWarning() << "History read failed:" << query.lastError().text();
		qWarning() << "Query was: " << queryText;
	}

	QList<Message> ret;
	Contact *contact;
	double t;
	while(query.next()) {
		contact = contact_info_i->get_contact(query.value(0).toString());
		if(contact) {
			Message m(contact, query.value(1).toString(), query.value(2).toBool(), 0, this);
			m.read = query.value(3).toBool();
			t = query.value(4).toDouble();
			m.timestamp = timestamp_decode(t);
			if(!m.read && mark_read)
				mark_as_read(contact, t);
			ret << m;
		}
	}

	qSort(ret);
	return ret;
}

void History::refire_latest_events(Contact *contact, QDateTime earliest, bool mark_read) {
	// hangs for long history - earliest unread is a BAD query
	//QDateTime earliestUnread = earliest_unread(contact);
	//if(earliestUnread < earliest)
	//	earliest = earliestUnread;

  //qDebug() << "refire_latest_events (time)";

	QSqlQuery readQuery(db);
	QString queryText = "SELECT contact_hash_id, message, incomming, msg_read, timestamp FROM message_history WHERE contact_hash_id=:hash AND timestamp>=:timestamp ORDER BY timestamp ASC;";
	readQuery.prepare(queryText);

	readQuery.bindValue(":hash", contact->hash_id);
	readQuery.bindValue(":timestamp", timestamp_encode(earliest));

	QList<Message> event_list = read_history(readQuery, mark_read, queryText);
	foreach(Message m, event_list)
		events_i->fire_event(m);
}

void History::refire_latest_events(Contact *contact, int count, bool mark_read) {
	//qDebug() << "refire_latest_events (count)";

	QSqlQuery readQuery(db);
	QString queryText = "SELECT contact_hash_id, message, incomming, msg_read, timestamp FROM message_history WHERE contact_hash_id=:hash ORDER BY timestamp DESC LIMIT :count;";
	readQuery.prepare(queryText);

	readQuery.bindValue(":hash", contact->hash_id);
	readQuery.bindValue(":count", count);

	QList<Message> event_list = read_history(readQuery, mark_read, queryText);
	foreach(Message m, event_list)
		events_i->fire_event(m);
}

void History::refire_unread_events(Contact *contact, bool mark_read) {
	qDebug() << "refire_unread_events";

	QSqlQuery readQuery(db);
	QString queryText = "SELECT contact_hash_id, message, incomming, msg_read, timestamp FROM message_history WHERE msg_read='false' AND contact_hash_id=:hash ORDER BY timestamp ASC;";
	readQuery.prepare(queryText);

	readQuery.bindValue(":hash", contact->hash_id);

	QList<Message> event_list = read_history(readQuery, mark_read, queryText);
	foreach(Message m, event_list)
		events_i->fire_event(m);
}


void History::refire_latest_events(QList<Contact *> contacts, QDateTime earliest, bool mark_read) {
	//qDebug() << "refire_latest_events (contacts, time)";
	QString contactsQueryPart;
	foreach(Contact *contact, contacts) {
		if(contactsQueryPart.size())
			contactsQueryPart += " OR ";
		contactsQueryPart += "contact_hash_id='" + contact->hash_id + "'";
	}

	QSqlQuery readQuery(db);
	QString queryText = "SELECT contact_hash_id, message, incomming, msg_read, timestamp FROM message_history WHERE "
					   + contactsQueryPart + " AND timestamp>=:timestamp ORDER BY timestamp ASC;";
	readQuery.prepare(queryText);

	readQuery.bindValue(":timestamp", timestamp_encode(earliest));

	QList<Message> event_list = read_history(readQuery, mark_read);
	foreach(Message m, event_list)
		events_i->fire_event(m);
}

void History::refire_latest_events(QList<Contact *> contacts, int count, bool mark_read) {
	//qDebug() << "refire_latest_events (contacts, count)";

	QString contactsQueryPart;
	foreach(Contact *contact, contacts) {
		if(contactsQueryPart.size())
			contactsQueryPart += " OR ";
		contactsQueryPart += "contact_hash_id='" + contact->hash_id + "'";
	}

	QSqlQuery readQuery(db);
	QString queryText = "SELECT contact_hash_id, message, incomming, msg_read, timestamp FROM message_history WHERE "
						+ contactsQueryPart + " ORDER BY timestamp DESC LIMIT :count;";
	qDebug() << "Query text: " << queryText;
	readQuery.prepare(queryText);

	readQuery.bindValue(":count", count);

	QList<Message> event_list = read_history(readQuery, mark_read);
	foreach(Message m, event_list)
		events_i->fire_event(m);
}

void History::mark_as_read(Contact *contact, QDateTime timestamp) {
	mark_as_read(contact, timestamp_encode(timestamp));
}

void History::mark_as_read(Contact *contact, double timestamp) {
	QSqlQuery mrq(db);
	QString queryText = "UPDATE message_history SET msg_read='true' WHERE contact_hash_id='" + contact->hash_id + "'"
		+ " AND timestamp=:timestamp;";

	mrq.prepare(queryText);
	mrq.bindValue(":timestamp", timestamp);

	if(!mrq.exec())
		qWarning() << "History mark as read failed:" << mrq.lastError().text();
	else
		if(contact->has_property("PendingMsg")) {
			QList<QVariant> times = contact->get_property("PendingMsg").toList();
			int index;
			if((index = times.indexOf(timestamp)) != -1)
				times.removeAt(index);
			if(times.size() == 0) {
				contact->remove_property("PendingMsg");
				ContactChanged cc(contact, this);
				events_i->fire_event(cc);
			} else
				contact->set_property("PendingMsg", times);
		}
}

void History::mark_all_as_read(Contact *contact) {
	QSqlQuery mrq(db);
	QString queryText = "UPDATE message_history SET msg_read='true' WHERE contact_hash_id='" + contact->hash_id + "'"
		+ " AND msg_read='false';";

	if(!mrq.exec(queryText))
		qWarning() << "History mark all as read failed:" << mrq.lastError().text();
	else
		if(contact->has_property("PendingMsg")) {
			contact->remove_property("PendingMsg");
			ContactChanged cc(contact, this);
			events_i->fire_event(cc);
		}
}

void History::wipe_history(Contact *contact) {
	QSqlQuery wq(db);
	QString queryText = "DELETE FROM message_history,contact_hash WHERE contact_hash_id='" + contact->hash_id + "';";
	if(!wq.exec(queryText))
		qWarning() << "History wipe failed:" << wq.lastError().text();
}

// BAD query
QDateTime History::earliest_unread(Contact *contact) {
	QSqlQuery wq(db);
	QString queryText = "SELECT min(timestamp) FROM message_history WHERE msg_read='false' AND contact_hash_id='" + contact->hash_id + "';";
	if(!wq.exec(queryText) || wq.size() == 0)
		return QDateTime::currentDateTime();
	return timestamp_decode(wq.value(1).toDouble());
}

void History::enable_history(Contact *contact, bool enable) {
	if(enable) contact->remove_property("DisableHistory");
	else contact->set_property("DisableHistory", true);

	ContactChanged cc(contact, this);
	events_i->fire_event(cc);
}

/////////////////////////////

Q_EXPORT_PLUGIN2(history, History)

