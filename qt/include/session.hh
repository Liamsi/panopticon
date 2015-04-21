/*
 * This file is part of Panopticon (http://panopticon.re).
 * Copyright (C) 2014 Panopticon authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtQuick>
#include <QAbstractTableModel>
#include <panopticon/database.hh>
#include <panopticon/region.hh>
#include <panopticon/procedure.hh>

#include "linear.hh"

#pragma once

class Procedure : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString name READ name NOTIFY nameChanged)

public:
	Procedure(QObject* parent = 0);
	virtual ~Procedure(void);

	void setProcedure(po::proc_loc p) { _procedure = p; }
	boost::optional<po::proc_loc> procedure(void) const { return _procedure; }

	QString name(void) const { return _procedure ? QString::fromStdString((*_procedure)->name) : ""; }

signals:
	void nameChanged(void);

private:
	boost::optional<po::proc_loc> _procedure;
};

class Session : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString title READ title NOTIFY titleChanged)
	Q_PROPERTY(QObject* linear READ linear NOTIFY linearChanged)
	Q_PROPERTY(QObject* activeProcedure READ activeProcedure WRITE setActiveProcedure NOTIFY activeProceduresChanged)
	Q_PROPERTY(bool dirty READ isDirty NOTIFY dirtyFlagChanged)
	Q_PROPERTY(QString savePath READ savePath NOTIFY savePathChanged)
	Q_PROPERTY(QVariantList calls READ calls NOTIFY callsChanged)
	Q_PROPERTY(QVariantList procedures READ procedures NOTIFY proceduresChanged)

public:
	Session(po::session, QObject *parent = nullptr);
	virtual ~Session(void);

	static Session* open(QString);
	static Session* createRaw(QString);
	static Session* createAvr(QString);

	Q_INVOKABLE void postComment(int row, QString c);
	Q_INVOKABLE void disassemble(int row, int col);
	Q_INVOKABLE void save(QString);

	QString title(void) const { return QString::fromStdString(_session.dbase->title); }
	QObject* linear(void) const { return _linear; }
	QObject* activeProcedure(void) const { return _activeProcedure; }
	bool isDirty(void) const { return _dirty; }
	QString savePath(void) const { return _savePath; }
	QVariantList const& procedures(void) const { return _procedures; }
	QVariantList const& calls(void) const { return _calls; }

	void setActiveProcedure(QObject*);

signals:
	void titleChanged(void);
	void linearChanged(void);
	void proceduresChanged(void);
	void callsChanged(void);
	void activeProceduresChanged(void);
	void dirtyFlagChanged(void);
	void savePathChanged(void);

private:
	po::session _session;
	LinearModel* _linear;
	Procedure* _activeProcedure;
	bool _dirty;
	QString _savePath;

	void makeDirty(void) { if(!_dirty) { _dirty = true; emit dirtyFlagChanged(); } }
	QVariantList _procedures;
	QVariantList _calls;
};
