//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2019 by the CryoFLARE Authors
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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QSettings>

const QString  CRYOFLARE_DIRECTORY(".cryoflare");
const QString  CRYOFLARE_GRIDSQUARES_DIRECTORY("gridsquares");
const QString  CRYOFLARE_FOILHOLES_DIRECTORY("foilholes");
const QString  CRYOFLARE_INI(".cryoflare/cryoflare.ini");
//fw decl
class SettingsGroup;

class Settings: public QObject
{
    Q_OBJECT
public:
    Settings(QObject *parent = NULL);
    bool loadFromFile(const QString& path, const QStringList &excludes=QStringList(), const QStringList &includes=QStringList());
    void saveToFile(const QString& path, const QStringList &excludes=QStringList(), const QStringList &includes=QStringList()) const;
    void loadFromQSettings(const QStringList &excludes=QStringList(), const QStringList &includes=QStringList());
    void saveToQSettings(const QStringList &excludes=QStringList(), const QStringList &includes=QStringList()) const;
    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    bool contains(const QString &key);
    void beginGroup(const QString &prefix);
    void endGroup();
    QStringList childGroups() const;
    QStringList allKeys() const;
    QStringList childKeys() const;
    void remove(const QString & key);
    void clear();
protected:
    SettingsGroup* current_;
};
#endif // SETTINGS_H
