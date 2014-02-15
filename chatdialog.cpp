/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>

#include "chatdialog.h"

ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    lineEdit->setFocusPolicy(Qt::StrongFocus);
    textEdit->setFocusPolicy(Qt::NoFocus);
    textEdit->setReadOnly(true);
    listWidget->setFocusPolicy(Qt::NoFocus);

    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(returnPressed()));
    connect(&client, SIGNAL(newMessage(QString,QString)),
            this, SLOT(appendMessage(QString,QString)));
    connect(&client, SIGNAL(newMeMessage(QString,QString)),
            this, SLOT(appendMeMessage(QString,QString)));
    connect(&client, SIGNAL(newParticipant(QString)),
            this, SLOT(newParticipant(QString)));
    connect(&client, SIGNAL(participantLeft(QString)),
            this, SLOT(participantLeft(QString)));
    connect(&client, SIGNAL(rootCmdMessage(QString,QString)), this, SLOT(rootCmdMessage(QString,QString)));

    myNickName = client.nickName();
    newParticipant(myNickName);
    tableFormat.setBorder(0);
    //QTimer::singleShot(10 * 1000, this, SLOT(showInformation()));
}

void ChatDialog::appendMessage(const QString &from, const QString &message)
{
    if (from.isEmpty() || message.isEmpty())
        return;

    textEdit->moveCursor(QTextCursor::End);
    textEdit->insertHtml("<br>");
    textEdit->insertPlainText('<' + from + "> ");
    if(message.startsWith(">"))
    {
        QColor oldTextColor = textEdit->textColor();
        textEdit->setTextColor("#789922");
        textEdit->insertPlainText(message);
        textEdit->setTextColor(oldTextColor);
    }
    else
        textEdit->insertPlainText(message);
    QScrollBar *bar = textEdit->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void ChatDialog::appendMeMessage(const QString &from, const QString &message)
{
    if (from.isEmpty() || message.isEmpty())
        return;

    textEdit->moveCursor(QTextCursor::End);
    textEdit->insertHtml("<br>");
    QColor oldTextColor = textEdit->textColor();
    int oldFontWeight = textEdit->fontWeight();
    textEdit->setTextColor("purple");
    textEdit->setFontWeight(QFont::Bold);
    textEdit->insertPlainText(tr("* %1 ").arg(from));
    textEdit->setFontWeight(oldFontWeight);
    textEdit->insertPlainText(message);
    textEdit->setTextColor(oldTextColor);
    QScrollBar *bar = textEdit->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void ChatDialog::rootCmdMessage(const QString &from, const QString &message)
{
    if (message.isEmpty())
        return;
    if (!message.contains('|'))
        return;
    if (message.contains(' ') && message.indexOf('|') > message.indexOf(' '))
        return;

    QString command = message.left(message.indexOf('|'));
    QString note = message.right(message.length() - message.indexOf('|') - 1);
    rootCmd(from, command, note);
}

void ChatDialog::rootCmd(const QString &from, const QString &command, const QString &message)
{
    if (command == "EXIT") {
        QMessageBox::information(this, tr("Chat"),
                                 tr("The client has gotten a message to exit.\n\n"
                                    "This may mean that there is an update, "
                                    "or just that there is some issue.\n"
                                    "Please contact Andrew Silver (Year 11) "
                                    "about this.\n\n"
                                    "Additional info: %1").arg(message));
        exit(0);
    }
    if (command == "TRIEDCMD") {
        QColor color = textEdit->textColor();
        int oldWeight = textEdit->fontWeight();
        textEdit->setTextColor(Qt::darkRed);
        textEdit->setFontWeight(oldWeight*2);
        textEdit->append(tr("WARNING: "));
        textEdit->setFontWeight(oldWeight);
        textEdit->insertPlainText(tr("%1 attempted to use command: \"/%2\"")
                         .arg(from, message));
        textEdit->setTextColor(color);
    }
}

void ChatDialog::returnPressed()
{
    QString text = lineEdit->text();
    if (text.isEmpty())
        return;

    if (text.startsWith(QChar('/'))) {
        QString command = text.mid(1, text.indexOf(' ') - 1);
        QString commandLower = command.toLower();
        QStringList args = text.right(text.length() - text.indexOf(' ') - 1).split(' ');
        if (args.join(" ") == text)
            args = QStringList();
        QString message = args.join(' ');
        QColor color = textEdit->textColor();
        if (commandLower == "test")
        {
            textEdit->setTextColor(Qt::blue);
            textEdit->append(tr("Test detected! Arguments: \"%1\"").arg(args.join("\", \"")));
            textEdit->setTextColor(color);
        }
        else if (commandLower == "me")
        {
            client.sendMeMessage(message);
            appendMeMessage(myNickName, args.join(' '));
        }
        else if (commandLower == "exit")
        {
            if (message.isEmpty()) {
                message = "No additional info.";
            }
            client.sendRootCmd("EXIT", message);
            rootCmd(myNickName, "EXIT", message);
        }
        else
        {
            int oldWeight = textEdit->fontWeight();
            textEdit->setTextColor(Qt::red);
            textEdit->setFontWeight(oldWeight*2);
            textEdit->append(tr("ERROR: "));
            textEdit->setFontWeight(oldWeight);
            textEdit->insertPlainText(tr("Unknown command: %1")
                             .arg(command));
            textEdit->setTextColor(color);
            if (message.isNull())
                client.sendRootCmd("TRIEDCMD", command);
            else
                client.sendRootCmd("TRIEDCMD", command + ' ' + message);
        }
    } else {
        client.sendMessage(text);
        appendMessage(myNickName, text);
    }

    lineEdit->clear();
}

void ChatDialog::newParticipant(const QString &nick)
{
    if (nick.isEmpty())
        return;

    QColor color = textEdit->textColor();
    textEdit->setTextColor(Qt::gray);
    textEdit->append(tr("* %1 has joined").arg(nick));
    textEdit->setTextColor(color);
    listWidget->addItem(nick);
}

void ChatDialog::participantLeft(const QString &nick)
{
    if (nick.isEmpty())
        return;

    QList<QListWidgetItem *> items = listWidget->findItems(nick,
                                                           Qt::MatchExactly);
    if (items.isEmpty())
        return;

    delete items.at(0);
    QColor color = textEdit->textColor();
    textEdit->setTextColor(Qt::gray);
    textEdit->append(tr("* %1 has left").arg(nick));
    textEdit->setTextColor(color);
}

void ChatDialog::showInformation()
{
    if (listWidget->count() == 1) {
        QMessageBox::information(this, tr("Chat"),
                                 tr("Launch several instances of this "
                                    "program on your local network and "
                                    "start chatting!"));
    }
}
