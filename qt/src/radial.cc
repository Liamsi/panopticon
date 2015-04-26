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

Radial::Radial(QQuickItem* p) : QQuickPaintedItem(p)
{
	connect(this,SIGNAL(callsChanged()),this,SLOT(update()));
}

Radial::~Radial(void) {}

void Radial::paint(QPainter* p)
{
	QList<QVariant> procs;
	QList<QPair<QVariant,QVariant>> calls;
	QListIterator<QVariant> iter(_calls);
	QList<QPair<QVariant,QStaticText>> labels;
	qreal max_label = 0;

	while(iter.hasNext())
	{
		QVariantList vl = iter.next().value<QVariantList>();

		if(vl.size() < 2)
		{
			qWarning() << "invalid call:" << iter.peekPrevious();
		}
		else
		{
			QVariant p1 = vl.takeFirst();
			QVariant p2 = vl.takeFirst();

			if(!procs.contains(p1))
			{
				Procedure* proc = qobject_cast<Procedure*>(p1.value<QObject*>());
				QStaticText label(proc && proc->procedure() ? QString::fromStdString((*proc->procedure())->name) : p1.toString());

				labels.append(QPair<QVariant,QStaticText>(p1,label));
				procs.append(p1);
				max_label = std::max(max_label,label.size().width());
			}

			if(!procs.contains(p2))
			{
				Procedure* proc = qobject_cast<Procedure*>(p2.value<QObject*>());
				QStaticText label(proc && proc->procedure() ? QString::fromStdString((*proc->procedure())->name) : p2.toString());

				labels.append(QPair<QVariant,QStaticText>(p2,label));
				procs.append(p2);
				max_label = std::max(max_label,label.size().width());
			}

			calls.append(QPair<QVariant,QVariant>(p1,p2));
		}
	}
	iter.toFront();

	ensure(p);
	p->save();
	p->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing,true);

	QPen pen(QBrush(QColor("blue")),1);
	pen.setCosmetic(true);
	p->setPen(pen);

	if(procs.size() > 0)
	{
		size_t i = 0;
		const size_t len = 360 / procs.size();
		QListIterator<QVariant> jter(procs);
		const qreal padding = 2 * pen.width() + 10 + 2 * max_label;
		QRectF bb1 = boundingRect();

		if(padding + 100 <= std::min(bb1.width(),bb1.height()))
			bb1.adjust(0,0,-padding,-padding);

		bb1.setWidth(std::min(bb1.height(),bb1.width()));
		bb1.setHeight(std::min(bb1.height(),bb1.width()));
		bb1.moveCenter(boundingRect().center());

		QRectF bb2 = bb1.adjusted(0,0,-100,-100);
		bb2.moveCenter(boundingRect().center());

		// draw sectors
		while(jter.hasNext())
		{
			QVariant v = jter.next();
			QPainterPath pp;
			QLineF a = QLineF::fromPolar(bb2.width() / 2,i * len + len - 2).translated(bb1.center());
			QLineF b = QLineF::fromPolar(bb1.width() / 2,i * len).translated(bb1.center());
			QLineF m = QLineF::fromPolar(bb1.width() / 2 + 5,i * len + (len - 2) / 2).translated(bb1.center());
			QStaticText label = std::find_if(labels.begin(),labels.end(),[&](QPair<QVariant,QStaticText> const& qq) { return qq.first == v; })->second;

			pp.arcMoveTo(bb1,i * len);
			pp.arcTo(bb1,i * len,len - 2);
			pp.lineTo(a.p2());
			pp.arcMoveTo(bb2,i * len);
			pp.arcTo(bb2,i * len,len - 2);
			pp.arcMoveTo(bb2,i * len);
			pp.lineTo(b.p2());
			p->drawPath(pp);

			p->save();
			QPen pen(QBrush(QColor("red")),2);
			pen.setCosmetic(true);
			p->setPen(pen);

			p->translate(m.p2());
			qreal r = (360 - i * len - ((len - 2) / 2));
			p->rotate(r);

			if(r > 90 && r < 270)
			{
				p->translate(QPointF(label.size().width(),label.size().height() / 2));
				p->rotate(180);
			}
			else
			{
				p->translate(QPointF(0,-label.size().height() / 2));
			}

			p->drawStaticText(QPointF(0,0),label);
			p->restore();

			++i;
		}

		// draw calls
		QListIterator<QPair<QVariant,QVariant>> kter(calls);
		while(kter.hasNext())
		{
			QPair<QVariant,QVariant> q = kter.next();
			auto ix = std::find(procs.begin(),procs.end(),q.first);
			auto iy = std::find(procs.begin(),procs.end(),q.second);

			ensure(ix != procs.end());
			ensure(iy != procs.end());

			size_t x = std::distance(procs.begin(),ix);
			size_t y = std::distance(procs.begin(),iy);

			QLineF a = QLineF::fromPolar(bb2.width() / 2 ,x * len + (len - 2) / 2).translated(bb1.center());
			QLineF b = QLineF::fromPolar(bb2.width() / 2 ,y * len + (len - 2) / 2).translated(bb1.center());

			p->drawLine(a.p2(),b.p2());

			++i;
		}
	}

	p->restore();
}
