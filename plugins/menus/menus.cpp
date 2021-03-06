#include "menus.h"
#include <QtPlugin>
#include <QDebug>

PluginInfo info = {
  0x200,
  "Menus",
  "Scott Ellis",
  "mail@scottellis.com.au",
  "http://www.scottellis.com.au",
  "Menus",
  0x00000001
};

Menus::Menus()
{

}

Menus::~Menus()
{

}

bool Menus::load(CoreI *core) {
  core_i = core;
  if((events_i = (EventsI *)core_i->get_interface(INAME_EVENTS)) == 0) return false;
  if((icons_i = (IconsI *)core_i->get_interface(INAME_ICONS)) == 0) return false;

  return true;
}

bool Menus::modules_loaded() {
  OptionsI *options_i = (OptionsI *)core_i->get_interface(INAME_OPTIONS);
  if(options_i) {
    opt = new MenusOptions();
    connect(opt, SIGNAL(applied()), this, SLOT(options_applied()));
    options_i->add_page("Menus", opt);
  }
  return true;
}

bool Menus::pre_shutdown() {
  return true;
}

bool Menus::unload() {
  qDeleteAll(menus.values());

  return true;
}

const PluginInfo &Menus::get_plugin_info() {
  return info;
}

/////////////////////////////

void Menus::options_applied() {
}

QAction *Menus::add_menu_action(const QString &menu_id, const QString &label, const QString &icon, QAction *prev) {
  QStringList path = menu_id.split("/");
  QString p, parent;
  if(path.size()) {
    QAction *action = new QAction(icons_i ? icons_i->get_icon(icon) : QIcon(), label, 0);
    while(path.size()) {
      parent = p;
      if(p.size()) p += "/";
      p += path.first();

      if(!menus.contains(p)) {
        menus[p] = new QMenu(path.first());
        if(parent.size()) {
          menus[parent]->insertMenu(menus[parent]->actions().value(0), menus[p]);
        }
      }

      path.takeFirst();
    }

    menus[menu_id]->insertAction(prev ? prev : 0, action);
    return action;
  }
  return 0;
}

QAction *Menus::add_menu_separator(const QString &menu_id, QAction *prev) {
  if(!menus.contains(menu_id))
    menus[menu_id] = new QMenu();
  return menus[menu_id]->insertSeparator(prev);
}

void Menus::show_menu(const QString &id, const QMap<QString, QVariant> &properties, const QPoint &p) {
  if(menus.contains(id)) {
    ShowMenu sgm(id, this);
    sgm.properties = properties;
    sgm.properties["name"] = id;

    events_i->fire_event(sgm);
    QPoint pos = p;
    if(p.isNull()) pos = QCursor::pos();
    menus[id]->exec(pos);
  }
}

QMenu *Menus::get_menu(const QString &id) {
  if(!menus.contains(id))
    menus[id] = new QMenu();

  return menus[id];
}

/////////////////////////////

Q_EXPORT_PLUGIN2(menus, Menus)

