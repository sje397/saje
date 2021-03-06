#include "optionswin.h"
#include <QPushButton>
#include <QTreeWidgetItem>
#include <QDebug>
#include <QSettings>
#include <QShowEvent>

OptionsWin::OptionsWin(QWidget *parent)
	: QDialog(parent), numPages(0)
{
	ui.setupUi(this);

	ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
	ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(apply()));
	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(hide()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(cancel()));
	connect(ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));

	connect(ui.treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(treeSelectionChanged()));
}

OptionsWin::~OptionsWin()
{

}

void OptionsWin::set_button_states() {
	bool all_valid = (pages_invalid.size() == 0);
	bool changed = (pages_changed.size() > 0);

	ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(all_valid);
	ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(all_valid && changed);
}

void OptionsWin::showEvent(QShowEvent *e) {
	if(!e->spontaneous()) {
		QMapIterator<QTreeWidgetItem *, OptionsPageI *> i(pages);
		while(i.hasNext()) {
			i.next();
			i.value()->reset();
		}
		pages_invalid.clear();

		ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
		ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	}
	return QDialog::showEvent(e);
}

QTreeWidgetItem *findGroup(QTreeWidgetItem *parent, const QString &name) {
	for(int i = 0; i < parent->childCount(); i++) {
		if(parent->child(i)->text(0) == name)
			return parent->child(i);
	}
	return 0;
}

bool OptionsWin::add_page(const QString &category, OptionsPageI *w) {
	QSettings settings;
	QTreeWidgetItem *i, *parent = ui.treeWidget->invisibleRootItem();
	QStringList subgroups = category.split("/");
	QString full_gn;
	while((i = findGroup(parent, subgroups.at(0))) != 0) {
		parent = i;
		full_gn += "/" + subgroups.at(0);
		subgroups.removeAt(0);
	}
	while(subgroups.size()) {
		i = new QTreeWidgetItem(parent, QStringList() << subgroups.at(0));
		full_gn += "/" + subgroups.at(0);
		subgroups.removeAt(0);
		i->setExpanded(settings.value("Options/group_expand" + full_gn, true).toBool());
		parent = i;
	}

	pages[i] = w;
	categories[w] = category;
	connect(w, SIGNAL(changed(bool)), this, SLOT(changed(bool)));
	ui.stack->addWidget(w);

	if(numPages == 0 || category == settings.value("Options/selectedPage", "Accounts").toString()) {
		ui.stack->setCurrentWidget(w);
		ui.treeWidget->setCurrentItem(i);
	}
	ui.treeWidget->resizeColumnToContents(0);
	ui.treeWidget->sortByColumn(0, Qt::AscendingOrder);

	numPages++;

	return true;
}

void OptionsWin::treeSelectionChanged() {
	QTreeWidgetItem *i = ui.treeWidget->currentItem();
	if(pages.contains(i)) {
		ui.stack->setCurrentWidget(pages[i]);
		if(numPages > 0) {
			QSettings settings;
			settings.setValue("Options/selectedPage", categories[pages[i]]);
		}
	} else {
		ui.stack->setCurrentIndex(0);
	}
}

void OptionsWin::apply() {
	if(pages_changed.size()) {
                QListIterator<OptionsPageI *> i(pages_changed);
                while(i.hasNext())
                        i.next()->apply();
		pages_changed.clear();
	}
	ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
}

void OptionsWin::cancel() {
	hide();
}

void OptionsWin::changed(bool valid) {
        pages_changed.append((OptionsPageI *)ui.stack->currentWidget());
        int i = pages_invalid.indexOf((OptionsPageI *)ui.stack->currentWidget());
	if(valid && i >= 0) pages_invalid.removeAt(i);
        if(!valid && i == -1) pages_invalid.append((OptionsPageI *)ui.stack->currentWidget());
	set_button_states();
}

void OptionsWin::showPage(const QString &page) {
	QList<QTreeWidgetItem *> items = ui.treeWidget->findItems(page, Qt::MatchExactly, 0);
	if(items.size())
		ui.treeWidget->setCurrentItem(items.first());
	showNormal();
}
