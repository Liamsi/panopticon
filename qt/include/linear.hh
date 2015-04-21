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

#pragma once

typedef std::pair<po::region_wloc,boost::variant<po::bound,po::bblock_loc,po::struct_loc>> row_t;

class LinearModel : public QAbstractListModel
{
	Q_OBJECT

public:
	LinearModel(po::dbase_loc db, QObject* p = nullptr);

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex& parent = QModelIndex(), int role = Qt::DisplayRole) const;

	void postComment(int row, QString c);

	Q_INVOKABLE int rowForProcedure(QObject*) const;

protected:
	struct data_visitor : public boost::static_visitor<std::tuple<QString,po::bound,std::list<po::bound>>>
	{
		data_visitor(int r, boost::icl::interval<int>::type iv, po::region_loc re);

		std::tuple<QString,po::bound,std::list<po::bound>> operator()(po::bound b) const;
		std::tuple<QString,po::bound,std::list<po::bound>> operator()(po::bblock_loc bb) const;
		std::tuple<QString,po::bound,std::list<po::bound>> operator()(po::struct_loc s) const;

	private:
		int row;
		boost::icl::interval<int>::type ival;
		po::region_loc reg;
	};

	int findTrack(po::bound b, bool d);
	std::list<std::tuple<po::bound,po::region_wloc,bool>> filterUndefined(const std::list<std::pair<po::bound,po::region_wloc>>& l) const;

	po::dbase_loc _dbase;
	std::list<std::tuple<po::bound,po::region_wloc,bool>> _projection;
	boost::icl::split_interval_map<int,row_t> _rows;
	std::list<boost::icl::split_interval_map<po::offset,int>> _tracks;
	std::unordered_map<po::proc_wloc,int> _procedures;
};
