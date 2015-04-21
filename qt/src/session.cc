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

#include "session.hh"

#include <QObject>
#include <QPair>

#include <boost/variant.hpp>

#include <panopticon/program.hh>
#include <panopticon/avr/avr.hh>
#include <panopticon/loc.hh>

using namespace po;

Session::Session(po::session sess, QObject *p)
: QObject(p),
	_session(sess), _linear(new LinearModel(sess.dbase,this)),
	_activeProcedure(nullptr), _procedures(), _calls()
{
	if(!_session.dbase->programs.empty())
	{
		bool set = false;
		prog_loc prog = *_session.dbase->programs.begin();

		for(auto proc: prog->procedures())
		{
			auto p = new Procedure(this);

			p->setProcedure(proc);
			_procedures.append(QVariant::fromValue<QObject*>(p));
			if(!set)
			{
				set = true;
				//_graph->setProcedure(proc);
			}
		}

		auto cg = prog->calls();
		for(auto ed: iters(edges(cg)))
		{
			auto vx = get_vertex(source(ed,cg),cg);
			auto wx = get_vertex(target(ed,cg),cg);

			auto vp = boost::get<po::proc_loc>(&vx);
			auto wp = boost::get<po::proc_loc>(&wx);

			if(vp && wp)
			{
				std::function<bool(QVariant,po::proc_loc)> cmp = [](QVariant var,po::proc_loc n)
				{
					Procedure* pp = qobject_cast<Procedure*>(var.value<QObject*>());
					return pp && pp->procedure() && *pp->procedure() == n;
				};

				auto p = std::find_if(_procedures.begin(),_procedures.end(),std::bind(cmp,std::placeholders::_1,*vp));
				auto q = std::find_if(_procedures.begin(),_procedures.end(),std::bind(cmp,std::placeholders::_1,*wp));

				ensure(p != _procedures.end());
				ensure(q != _procedures.end());

				_calls.append(QVariant::fromValue(QPair<QVariant,QVariant>(*p,*q)));
			}
		}
	}
}

Session::~Session(void)
{}

Session* Session::open(QString s)
{
	po::discard_changes();
	return new Session(po::open(s.toStdString()));
}

Session* Session::createRaw(QString s)
{
	po::discard_changes();
	return new Session(po::raw(s.toStdString()));
}

Session* Session::createAvr(QString s)
{
	po::discard_changes();
	return new Session(po::raw_avr(s.toStdString(),po::avr_state::mega88()));
}

void Session::postComment(int r, QString c)
{
	_linear->postComment(r,c);
	makeDirty();
}

void Session::disassemble(int r, int c)
{
	qDebug() << "start disassemble at" << r << "/" << c;
}

void Session::setActiveProcedure(QObject* s)
{
	Procedure *proc = qobject_cast<Procedure*>(s);
	if(proc && proc != _activeProcedure && proc->procedure())
	{
		_activeProcedure = proc;

		/*auto prog = *_session.dbase->programs.begin();
		auto p = std::find_if(prog->procedures().begin(),prog->procedures().end(),[&](proc_loc p) { return p->name == s.toStdString(); });

		ensure(p != prog->procedures().end());

		_graph->setProcedure(*p);*/
		//_linear->setProcedure(*proc->procedure());

		emit activeProceduresChanged();
	}
}

void Session::save(QString path)
{
	if(isDirty())
	{
		qDebug() << "Saving session at " << path;

		try
		{
			save_point(*_session.store);
			_session.store->snapshot(path.toStdString());

			bool x = _savePath == path;
			_savePath = path;
			_dirty = false;

			emit dirtyFlagChanged();
			if(!x)
				emit savePathChanged();

			qDebug() << "Done";
		}
		catch(std::runtime_error const& e)
		{
			qWarning() << QString(e.what());
		}
	}
	else
	{
		qDebug() << "No changes to save";
	}
}
