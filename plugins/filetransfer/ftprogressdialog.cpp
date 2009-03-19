#include "ftprogressdialog.h"
#include "ui_ftprogressdialog.h"

FTProgressDialog::FTProgressDialog(QObject *s, const QString &ftId, bool in, Contact *c, const QString &filename, int bytes, QWidget *parent) :
    QDialog(parent),
	m_ui(new Ui::FTProgressDialog), source(s), id(ftId), incoming(in), contact(c), sizeBytes(bytes)
{
    m_ui->setupUi(this);

	m_ui->lblState->setText(incoming ? tr("INCOMING") : tr("OUTGOING"));
	m_ui->lblContact->setText(contact->contact_id);
	m_ui->lblFileName->setText(filename);
	m_ui->lblSize->setText(QString("%1 kb").arg(bytes/1024));

	m_ui->lblMsg->setText(tr("Waiting for acceptance..."));

	setAttribute(Qt::WA_DeleteOnClose, true);
}

FTProgressDialog::~FTProgressDialog()
{
    delete m_ui;
}

void FTProgressDialog::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void FTProgressDialog::setState(State s) {
	state = s;
	QString msg;
	switch(state) {
		case ST_ACCEPTED:
			msg = tr("Accepted. Waiting for data..."); break;
		case ST_CANCELLED:
			msg = tr("Cancelled.");
			m_ui->btnDone->setText(tr("Close"));
			break;
		case ST_INPROGRESS:
			msg = incoming ? tr("Receiving...") : tr("Sending...");
			break;
		case ST_COMPLETED:
			msg = tr("Completed.");
			m_ui->btnDone->setText(tr("Close"));
			break;
	}
	m_ui->lblMsg->setText(msg);
}

void FTProgressDialog::setProgress(int prog) {
	m_ui->prbProgress->setValue((int)(prog / (float)sizeBytes + 0.5));
	if(prog == sizeBytes)
		setState(ST_COMPLETED);
	else
		setState(ST_INPROGRESS);
}

void FTProgressDialog::on_btnDone_clicked()
{
	if(state == ST_CANCELLED)
		setState(ST_COMPLETED);
	else
		setState(ST_CANCELLED);
	emit cancelled(source, id, contact);
}
