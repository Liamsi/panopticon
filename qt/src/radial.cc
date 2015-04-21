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

#include <QSet>
#include <QSetIterator>
#include <QList>
#include <QListIterator>
#include <QMap>
#include <QMapIterator>

#include "radial.hh"
#include "session.hh"

#include <panopticon/ensure.hh>

Radial::Radial(QQuickItem* p) : QQuickPaintedItem(p) {}

Radial::~Radial(void) {}

void Radial::paint(QPainter* p)
{
	QSet<Procedure*> procs;
	QMap<Procedure*,Procedure*> calls;
	QListIterator<QVariant> iter(_calls);

	while(iter.hasNext())
	{
		QVariantList vl = iter.next().value<QVariantList>();

		if(vl.size() < 2)
		{
			qWarning() << "invalid call:" << iter.peekPrevious();
		}
		else
		{
			Procedure* p1 = qobject_cast<Procedure*>(vl.takeFirst().value<QObject*>());
			Procedure* p2 = qobject_cast<Procedure*>(vl.takeFirst().value<QObject*>());

			if(!p1 || !p2)
			{
				qWarning() << "invalid call:" << iter.peekPrevious();
			}
			else
			{
				procs.insert(p1);
				procs.insert(p2);
				calls.insertMulti(p1,p2);
			}
		}
	}
	iter.toFront();

	ensure(p);
	p->save();
	p->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing,true);

	QPen pen(QBrush(QColor("blue")),3);
	pen.setCosmetic(true);
	p->setPen(pen);

	size_t i = 0;
	const size_t len = 360 / procs.size();
	QSetIterator<Procedure*> jter(procs);
	QRectF bb1 = boundingRect().adjusted(0,0,-2 * pen.width(),-2 * pen.width());

	bb1.setWidth(std::min(bb1.height(),bb1.width()));
	bb1.setHeight(std::min(bb1.height(),bb1.width()));
	bb1.moveCenter(boundingRect().center());

	QRectF bb2 = bb1.adjusted(0,0,-100,-100);
	bb2.moveCenter(boundingRect().center());

	// draw sectors
	while(jter.hasNext())
	{
		QPainterPath pp;
		QLineF a = QLineF::fromPolar(bb2.width() / 2,i * len + len - 2).translated(bb1.center());
		QLineF b = QLineF::fromPolar(bb1.width() / 2,i * len).translated(bb1.center());

		pp.arcMoveTo(bb1,i * len);
		pp.arcTo(bb1,i * len,len - 2);
		pp.lineTo(a.p2());
		pp.arcMoveTo(bb2,i * len);
		pp.arcTo(bb2,i * len,len - 2);
		pp.arcMoveTo(bb2,i * len);
		pp.lineTo(b.p2());
		p->drawPath(pp);

		jter.next();
		++i;
	}

	// draw calls
	QMapIterator<Procedure*,Procedure*> kter(calls);
	while(kter.hasNext())
	{
		kter.next();
		auto ix = std::find(procs.begin(),procs.end(),kter.key());
		auto iy = std::find(procs.begin(),procs.end(),kter.value());

		ensure(ix != procs.end());
		ensure(iy != procs.end());

		size_t x = std::distance(procs.begin(),ix);
		size_t y = std::distance(procs.begin(),iy);

		QLineF a = QLineF::fromPolar(bb2.width() / 2 ,x * len + (len - 2) / 2).translated(bb1.center());
		QLineF b = QLineF::fromPolar(bb2.width() / 2 ,y * len + (len - 2) / 2).translated(bb1.center());

		p->drawLine(a.p2(),b.p2());

		++i;
	}

	p->restore();
}
