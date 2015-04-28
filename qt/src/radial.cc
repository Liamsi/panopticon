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

			if(p1 != p2)
			{
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
		const qreal len = 360 / procs.size();
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
			QLineF a = QLineF::fromPolar(bb1.width() / 2,i * len).translated(bb1.center());
			QLineF b = QLineF::fromPolar(bb2.width() / 2,i * len + len - 2).translated(bb1.center());

			QLineF m = QLineF::fromPolar(bb1.width() / 2 + 5,i * len + (len - 2) / 2).translated(bb1.center());
			QStaticText label = std::find_if(labels.begin(),labels.end(),[&](QPair<QVariant,QStaticText> const& qq) { return qq.first == v; })->second;

			pp.arcMoveTo(bb1,i * len);
			pp.arcTo(bb1,i * len,len - 2);
			pp.lineTo(b.p2());
			pp.arcTo(bb2,i * len + len - 2, -len + 2);
			pp.lineTo(a.p2());
			p->fillPath(pp,QBrush(QColor("red")));
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

		i = 0;

		// draw calls
		QListIterator<QPair<QVariant,QVariant>> kter(calls);
		while(kter.hasNext())
		{
			QPair<QVariant,QVariant> q = kter.next();
			auto ix = std::find(procs.begin(),procs.end(),q.first);
			auto iy = std::find(procs.begin(),procs.end(),q.second);
			qreal x_slots = 0, y_slots = 0;
			boost::optional<std::pair<qreal,qreal>> pos = boost::none;

			for(auto r: calls)
			{
				x_slots += ((r.first == q.first) ^ (r.second == q.first));
				y_slots += ((r.first == q.second) ^ (r.second == q.second));

				if(r == q)
					pos = std::make_pair(x_slots,y_slots);
			}

			ensure(pos);
			ensure(ix != procs.end());
			ensure(iy != procs.end());
			ensure(x_slots && y_slots);

			qreal x_angle_start = std::distance(procs.begin(),ix) * len + ((pos->first - 1) / x_slots * (len - 2));
			qreal y_angle_end = std::distance(procs.begin(),iy) * len + ((pos->second - 1) / y_slots * (len - 2));
			qreal x_angle_end = x_angle_start + (len - 2) / x_slots;
			qreal y_angle_start = y_angle_end + (len - 2) / y_slots;

			QLineF a = QLineF::fromPolar(bb2.width() / 2, x_angle_start).translated(bb1.center());
			QLineF ac = QLineF::fromPolar(bb2.width() / 2 - 100, x_angle_start).translated(bb1.center());

			QLineF b = QLineF::fromPolar(bb2.width() / 2, y_angle_start).translated(bb1.center());
			QLineF bc = QLineF::fromPolar(bb2.width() / 2 - 100, y_angle_start).translated(bb1.center());

			QLineF cc = QLineF::fromPolar(bb2.width() / 2 - 100, y_angle_end).translated(bb1.center());

			QLineF d = QLineF::fromPolar(bb2.width() / 2, x_angle_end).translated(bb1.center());
			QLineF dc = QLineF::fromPolar(bb2.width() / 2 - 100, x_angle_end).translated(bb1.center());

			QPainterPath pp;
			QColor fill = QColor::fromHsv((i * 54) % 256,150,100,100);

			pp.moveTo(a.p2());
			pp.cubicTo(ac.p2(),bc.p2(),b.p2());
			pp.arcTo(bb2,y_angle_start,y_angle_end - y_angle_start);
			pp.cubicTo(cc.p2(),dc.p2(),d.p2());
			pp.arcTo(bb2,x_angle_end,-(x_angle_end - x_angle_start));

			p->fillPath(pp,QBrush(fill));
			p->drawPath(pp);

			++i;
		}
	}

	p->restore();
}
