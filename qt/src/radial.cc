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
#include "circle.hh"

#include <panopticon/ensure.hh>

using namespace po;
using namespace boost;

Radial::Radial(QQuickItem* p) : QQuickPaintedItem(p)
{
	connect(this,SIGNAL(callsChanged()),this,SLOT(update()));
}

Radial::~Radial(void) {}

void Radial::paint(QPainter* p)
{
	using vertex_t = std::tuple<QVariant,QStaticText>;

	qreal max_label = 0;
	po::digraph<vertex_t,int> calls;

	auto insert_vx = [&](QVariant v)
	{
		Procedure* proc = qobject_cast<Procedure*>(v.value<QObject*>());
		QStaticText label(proc && proc->procedure() ? QString::fromStdString((*proc->procedure())->name) : v.toString());

		max_label = std::max(max_label,label.size().width());
		return insert_vertex(std::make_tuple(v,label),calls);
	};

	// convert to digraph
	// XXX: cache
	for(auto v: _calls)
	{
		QVariantList vl = v.value<QVariantList>();

		if(vl.size() < 2)
		{
			qWarning() << "invalid call:" << v;
		}
		else
		{
			QVariant p1 = vl.takeFirst();
			QVariant p2 = vl.takeFirst();

			if(p1 != p2)
			{
				boost::optional<po::digraph<vertex_t,int>::vertex_descriptor> from, to;

				for(auto vx: iters(vertices(calls)))
				{
					if(std::get<0>(get_vertex(vx,calls)) == p1)
						from = vx;
					if(std::get<0>(get_vertex(vx,calls)) == p2)
						to = vx;
				}

				if(!from)
					from = insert_vx(p1);
				if(!to)
				{
					if(to == from)
						to = from;
					else
						to = insert_vx(p2);
				}

				ensure(from && to);

				insert_edge(0,*from,*to,calls);
			}
		}
	}

	auto vert_lst = minimizeCrossing(calls);

	ensure(p);
	p->save();
	p->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing,true);

	QPen pen(QBrush(QColor("blue")),1);
	pen.setCosmetic(true);
	p->setPen(pen);

	if(!vert_lst.empty())
	{
		size_t i = 0;
		const qreal len = 360 / vert_lst.size();
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
		for(auto _vx: vert_lst)
		{
			QVariant v;
			QStaticText label;
			QPainterPath pp;
			QLineF a = QLineF::fromPolar(bb1.width() / 2,i * len).translated(bb1.center());
			QLineF b = QLineF::fromPolar(bb2.width() / 2,i * len + len - 2).translated(bb1.center());
			QLineF m = QLineF::fromPolar(bb1.width() / 2 + 5,i * len + (len - 2) / 2).translated(bb1.center());

			std::tie(v,label) = get_vertex(_vx,calls);

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
		for(auto _ed: iters(edges(calls)))
		{
			auto ix = source(_ed,calls);
			auto iy = target(_ed,calls);
			qreal x_slots = 0, y_slots = 0;
			boost::optional<std::pair<qreal,qreal>> pos = boost::none;

			for(auto r: iters(edges(calls)))
			{
				x_slots += ((source(r,calls) == ix) ^ (target(r,calls) == ix));
				y_slots += ((source(r,calls) == iy) ^ (target(r,calls) == iy));

				if(get_edge(r,calls) == get_edge(_ed,calls))
					pos = std::make_pair(x_slots,y_slots);
			}

			ensure(pos);
			ensure(x_slots && y_slots);

			qreal x_angle_start = std::distance(vert_lst.begin(),std::find(vert_lst.begin(),vert_lst.end(),ix)) * len + ((pos->first - 1) / x_slots * (len - 2));
			qreal y_angle_end = std::distance(vert_lst.begin(),std::find(vert_lst.begin(),vert_lst.end(),iy)) * len + ((pos->second - 1) / y_slots * (len - 2));
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
