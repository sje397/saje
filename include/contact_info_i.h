#ifndef _I_CONTACT_INFO_H
#define _I_CONTACT_INFO_H

#include "accounts_i.h"
#include "events_i.h"

#define INAME_CONTACTINFO		"ContactInfoInterface"

#define UUID_CONTACT_CHANGED		"{3602C4D5-2589-4be5-AC4D-A2DCA5A4F8F0}"
#define UUID_MSG					"{99D59C72-F5C6-4a20-84CF-1BAA6612E945}"
#define UUID_USER_CHAT_STATE		"{AC8F9710-54B5-40af-9CA1-779AB4ED59DB}"
#define UUID_CONTACT_CHAT_STATE		"{C00E8853-CF13-4a7d-AF60-2684FF4D8FE9}"

class Contact {
public:
	Contact(Account *a, const QString &id): account(a), contact_id(id), status(ST_OFFLINE) {}

	Account *account;
	QString contact_id;

	GlobalStatus status;
	QVariant get_property(const QString &key) {if(properties.contains(key)) return properties[key]; return QVariant();}
	void set_property(const QString &key, const QVariant &v) {
		if((!properties.contains(key) || properties[key] != v) && changed_properties.indexOf(key) == -1)
			changed_properties.append(key);
		properties[key] = v;
	}
	QStringList get_changed_properties() {return changed_properties;}
	void clear_changed_properties() {changed_properties.clear();}
	void remove_property(const QString &key) {
		if(properties.contains(key)) {
			properties.remove(key);
			if(changed_properties.indexOf(key) == -1)
				changed_properties.append(key);
		}
	}
	bool has_property(const QString &key) {
		return properties.contains(key);
	}
private:
	QMap<QString, QVariant> properties; // should be property cache?
	QStringList changed_properties;
};

class ContactChanged: public EventsI::Event {
public:
	ContactChanged(Contact *c, QObject *source = 0): EventsI::Event(UUID_CONTACT_CHANGED, source), contact(c), removed(false) {}
	Contact *contact;
	bool removed;
};

class Message: public EventsI::Event {
public:
	class MessageData {
	public:
		MessageData(): incomming(false), read(false) {}
		MessageData(const QString msg, bool in): message(msg), incomming(in), read(!in) {}
		QString message;
		bool incomming, read;
	};

	Message(QObject *source = 0): EventsI::Event(UUID_MSG, source) {};
	Message(const QString &msg, bool incomming, int i, Contact *c, QObject *source = 0): EventsI::Event(UUID_MSG, source), data(msg, incomming), id(i), contact(c) {}
	Message(const Message &m): EventsI::Event(UUID_MSG, m.source), data(m.data), contact(m.contact), id(m.id) {}
	MessageData data;
	Contact *contact;
	int id;
};

typedef enum {CS_ACTIVE, CS_COMPOSING, CS_PAUSED, CS_INACTIVE, CS_GONE}  ChatStateType;

class UserChatState: public EventsI::Event {
public:

	UserChatState(Contact *c, ChatStateType t, QObject *source = 0): 
		EventsI::Event(UUID_USER_CHAT_STATE, source), contact(c), type(t) {}
	Contact *contact;
	ChatStateType type;
};

class ContactChatState: public EventsI::Event {
public:
	ContactChatState(Contact *c, ChatStateType t, QObject *source = 0): 
		EventsI::Event(UUID_CONTACT_CHAT_STATE, source), contact(c), type(t) {}
	Contact *contact;
	ChatStateType type;
};

class ContactInfoI: public PluginI, public EventsI::EventListener {
	Q_OBJECT
	Q_INTERFACES(PluginI)
public:

	const QString get_interface_name() const {return INAME_CONTACTINFO;}

	virtual Contact *get_contact(Account *acc, const QString &contact_id) = 0;
	virtual bool delete_contact(Contact *contact) = 0;
};

#endif
