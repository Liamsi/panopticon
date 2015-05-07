/*
 * This file is part of Panopticon (http://panopticon.re).
 * Copyright (C) 2015 Panopticon authors
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

#pragma once

class Radial : public QQuickPaintedItem
{
	Q_OBJECT
	Q_PROPERTY(QVariantList calls READ calls WRITE setCalls NOTIFY callsChanged)

public:
	Radial(QQuickItem *parent = nullptr);
	virtual ~Radial(void);

	virtual void paint(QPainter*) override;

	QVariantList const& calls(void) const { return _calls; }
	void setCalls(QVariantList const& c) { _calls = c; emit callsChanged(); }

signals:
	void callsChanged(void);

private:
	QVariantList _calls;
};
