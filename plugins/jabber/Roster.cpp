#include "Roster.h"
#include <QDebug>

QString RosterGroup::delimiter = "\\";

RosterItem *Resource::getItem() const {
  return static_cast<RosterItem *>(getParent());
}

QString Resource::full_jid() const {
  return getItem()->getJID() + "/" + name;
}

RosterItem::RosterItem(Contact *contact, const QString &n, SubscriptionType sub, RosterGroup *g)
  :RosterTreeNonLeafNode(n, g), subscription(sub),
  userChatState(CS_INACTIVE), contactChatState(CS_INACTIVE), contact(contact)

{
  if(!n.isEmpty()) contact->set_property("name", n);
  QStringList gn = g->getClistName();
  if(gn.size()) contact->set_property("group", gn);
}

void RosterItem::setName(const QString &n) {
  RosterTreeNonLeafNode::setName(n);
  contact->set_property("name", n);
}

void RosterItem::setParent(RosterTreeNonLeafNode *p) {
  RosterTreeNonLeafNode::setParent(p);
  if(p && getGroup()->getClistName().size())
    contact->set_property("group", getGroup()->getClistName());
  else
    contact->remove_property("group");
}

RosterGroup *RosterItem::getGroup() const {
  return static_cast<RosterGroup *>(getParent());
}

QString RosterItem::get_active_resource_id() const {
  Resource *r = get_active_resource(true);
  if(r) return r->full_jid();
  return getJID();
}

Resource *RosterItem::get_active_resource(bool onlyRecent) const {
  if(childCount() == 0) return 0;
  else if(childCount() == 1) return static_cast<Resource *>(child(0));
  else {

    if(onlyRecent) {
	  QDateTime recent = QDateTime::currentDateTime().addSecs(-60 * 60); // only last hour
      QVectorIterator<RosterTreeNode *> i(children);
      Resource *r = 0, *active = 0;
      while(i.hasNext()) {
        r = static_cast<Resource *>(i.next());
        if(r->getPresence() != PT_UNAVAILABLE && r->getLastActivity() > recent) {
          active = r;
          recent = r->getLastActivity();
        }
      }

      return active;
    } else {
      int prio, max_prio = -128;
      QVectorIterator<RosterTreeNode *> i(children);
      Resource *r = 0, *max_r = 0;
      while(i.hasNext()) {
        r = static_cast<Resource *>(i.next());
        prio = r->getPresence() == PT_UNAVAILABLE ? -128 : r->getPriority();
        if(prio >= max_prio) {
          if(prio == max_prio && max_r) {
            if(r->getLastActivity() > max_r->getLastActivity()) {
              max_r = r;
            }
          } else {
            max_prio = prio;
            max_r = r;
          }
        }
      }

      return max_r;
    }
  }
}

void RosterItem::setAllResourcePresence(PresenceType pres, const QString &msg) {
  QVectorIterator<RosterTreeNode *> i(children);
  Resource *r = 0;
  while(i.hasNext()) {
    r = static_cast<Resource *>(i.next());
    r->setPresence(pres, msg);
  }
}

bool RosterItem::is_offline() const {
  QVectorIterator<RosterTreeNode *> i(children);
  Resource *r;
  while(i.hasNext()) {
    r = static_cast<Resource *>(i.next());
    if(r->getPresence() != PT_UNAVAILABLE)
      return false;
  }
  return true;
}


RosterGroup *RosterGroup::get_group(const QStringList &group, bool create) {
  if(group.isEmpty())
    return this;

  QStringList gr = group;
  QString subgroup = gr.takeFirst();

  QVectorIterator<RosterTreeNode *> i(children);
  RosterGroup *g = 0;
  RosterTreeNode *c = 0;
  while(i.hasNext()) {
    c = i.next();
    if(c->type() == RTNT_GROUP && c->getName() == subgroup) {
      g = static_cast<RosterGroup *>(c);
      break;
    }
  }
  if(!g && create) {
    g = new RosterGroup(subgroup, this);
    addChild(g);
  }

  return g ? g->get_group(gr, create) : 0;
}

RosterItem *RosterGroup::get_item(const QString &jid) const {
  RosterTreeNode *c = 0;
  QVectorIterator<RosterTreeNode *> i(children);
  while(i.hasNext()) {
    c = i.next();
    if(c->type() == RTNT_ITEM) {
      RosterItem *item = static_cast<RosterItem *>(c);
      if(item->getJID() == jid)
        return item;
    } else if(c->type() == RTNT_GROUP) {
      RosterItem *i = static_cast<RosterGroup *>(c)->get_item(jid);
      if(i) return i;
    }
  }
  return 0;
}

Resource *Roster::get_resource(const QString &full_jid, bool create) {
  QString jid, resource;
  int index = full_jid.indexOf("/");
  if(index == -1) {
    jid = full_jid;
    resource = "default";
  } else {
    jid = full_jid.left(index);
    resource = full_jid.mid(index + 1);
  }
  //qDebug() << "jid:" << jid << "resource:" << resource;
  RosterItem *item = get_item(jid);
  if(!item) {
    qDebug() << "no item for jid" + jid;
    return 0;
  }

  Resource *n = static_cast<Resource *>(item->child(resource));
  if(!n && create) {
    n = new Resource(resource, item);
    item->addChild(n);
  }

  return n;
}
