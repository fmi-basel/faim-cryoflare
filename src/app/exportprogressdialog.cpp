//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2020 by the CryoFLARE Authors
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3.0 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License
// along with CryoFLARE.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include "exportprogressdialog.h"
#include "ui_exportprogressdialog.h"

class MessageSyntaxHighlighter: public QSyntaxHighlighter{
public:
    MessageSyntaxHighlighter(QTextDocument *doc):
        QSyntaxHighlighter(doc),
        verbosity_(0),
        formats_()
    {
        QTextCharFormat info;
        formats_.append(info);
        QTextCharFormat error;
        error.setFontWeight(QFont::Bold);
        error.setForeground(QColor("#ffaaaa"));
        formats_.append(error);
    }
    void setVerbosity(int v){
        verbosity_=v;
        rehighlight();
    }
    void highlightBlock(const QString &text){
        if(verbosity_<=currentBlock().userState()){
            currentBlock().setVisible(true);
            setFormat(0,text.size(),formats_[currentBlock().userState()]);
        }else{
            currentBlock().setVisible(false);
        }
    }
protected:
    int verbosity_;
    QList<QTextCharFormat> formats_;
} ;

ExportProgressDialog::ExportProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportProgressDialog),
    highlighter_()
{
    ui->setupUi(this);
    highlighter_=new MessageSyntaxHighlighter(ui->details->document());
    ui->details->setReadOnly(true);
    ui->details->setMaximumBlockCount(100000);
    setVerbose(Qt::Unchecked);
    connect(ui->verbose,&QCheckBox::stateChanged,this,&ExportProgressDialog::setVerbose);
    setModal(true);
    hide();
}

ExportProgressDialog::~ExportProgressDialog()
{
    delete ui;
}

void ExportProgressDialog::start(const QString &title, int num)
{
    setWindowTitle(title);
    ui->progress->reset();
    ui->progress->setMaximum(num);
    ui->finish->hide();
    ui->cancel->show();
    ui->details->clear();
    open();
}
void ExportProgressDialog::update(const QList<ExportMessage> &messages, int num_left)
{
    ui->progress->setValue(ui->progress->maximum()-num_left);
    update(messages);
}

void ExportProgressDialog::update(const QList<ExportMessage> &messages)
{
    if(messages.empty()){
        return;
    }
    //QStringList new_messages;
    foreach( ExportMessage m,messages){
        ui->details->appendHtml(QString("%1 (%2): %3: %4").arg(m.timestamp.toString(Qt::ISODateWithMs)).arg(m.counter_).arg(m.id).arg(m.text));
        if(m.type==ExportMessage::ERROR){
            ui->details->document()->lastBlock().setUserState(1);
        }else{
            ui->details->document()->lastBlock().setUserState(0);
        }
    }
}

void ExportProgressDialog::finish()
{
    ui->finish->show();
    ui->cancel->hide();
}

void ExportProgressDialog::setVerbose(int v)
{
    if(v==Qt::Checked){
        highlighter_->setVerbosity(0);
    }else{
        highlighter_->setVerbosity(1);
    }
}
