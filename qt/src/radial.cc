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

#include "radial.hh"
#include "session.hh"

#include <panopticon/ensure.hh>

Radial::Radial(QQuickItem* p) : QQuickPaintedItem(p) {}

Radial::~Radial(void) {}

void Radial::paint(QPainter* p)
{
	QSet<Procedure*> procs;
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
			}
		}
	}

	ensure(p);
	p->save();
	p->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing,true);

	QPen pen(QBrush(QColor("blue")),3);
	pen.setCosmetic(true);
	p->setPen(pen);

	size_t i = 0;
	const size_t len = 5760 / procs.size();
	QSetIterator<Procedure*> jter(procs);
	QRectF bb = boundingRect().adjusted(0,0,-pen.width(),-pen.width());

	bb.setWidth(std::min(bb.height(),bb.width()));
	bb.setHeight(std::min(bb.height(),bb.width()));
	bb.moveCenter(boundingRect().center());

	while(jter.hasNext())
	{
		jter.next();
		p->drawArc(bb,i * len,len - 50);
		++i;
	}

	p->restore();
}
