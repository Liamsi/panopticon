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

#include <list>

#include <panopticon/digraph.hh>

#pragma once

template<typename N,typename E>
std::list<typename po::digraph<N,E>::vertex_descriptor> minimizeCrossing(po::digraph<N,E> const& g)
{
	auto initial = greedyOrdering(g);

	for(auto vx: iters(vertices(g)))
	{
		optimizeOrder(vx,initial,g);
	}

	return initial;
}

template<typename N,typename E>
std::list<typename po::digraph<N,E>::vertex_descriptor> greedyOrdering(po::digraph<N,E> const& g)
{
	std::list<typename po::digraph<N,E>::vertex_descriptor> ret;

	for(auto vx: iters(vertices(g)))
	{
		ret.push_back(vx);
	}

	return ret;
}

template<typename N,typename E>
void optimizeOrder(typename po::digraph<N,E>::vertex_descriptor, std::list<typename po::digraph<N,E>::vertex_descriptor>& order, po::digraph<N,E> const& g)
{
	return;
}

/*
 * Adapted from Baur & Brandes: "Crossing Reduction in Circular Layouts"
 *
QList<QPair<QVariant,QVariant>> Radial::minimizeCrossing(QList<QPair<QVariant,QVariant>> unsorted)
{
	using edge_t = QPair<QVariant,QVariant>;
	using node_t = QVariant;
	QList<node_t> nodes, todo, order;

	if(unsorted.empty())
		return unsorted;

	for(edge_t const& e: unsorted)
	{
		if(!nodes.contains(e.first))
			nodes.append(e.first);
		if(!nodes.contains(e.second))
			nodes.append(e.second);
	}

	todo = nodes;
	order.append(todo.takeFirst());
	std::function<unsigned int(node_t const&)> unplaced_neight = [&](node_t const& n)
	{
		return std::count_if(unsorted.begin(),unsorted.end(),[&](edge_t const& e)
		{
			return ((e.first == n && !order.contains(e.second)) ||
							(e.second == n && !order.contains(e.first))) && e.first != e.second;
		});
	};
	std::function<unsigned int(edge_t const&,QList<node_t> const&)> crossings = [&](edge_t const& e, QList<node_t> const& order)
	{
		return std::count_if(unsorted.begin(),unsorted.end(),[&](edge_t const& f)
		{
			return order.contains(f.first) && order.contains(f.second) && (
						 (order.indexOf(e.first) < order.indexOf(f.first) && order.indexOf(f.first) < order.indexOf(e.second)) ||
						 (order.indexOf(f.first) < order.indexOf(e.first) && order.indexOf(e.first) < order.indexOf(f.second)) ||
						 (order.indexOf(f.second) < order.indexOf(e.second) && order.indexOf(e.second) < order.indexOf(f.first)) ||
						 (order.indexOf(e.second) < order.indexOf(f.second) && order.indexOf(f.second) < order.indexOf(e.first)));
		});
	};

	while(!todo.empty())
	{
		std::sort(todo.begin(),todo.end(),[&](node_t const& a, node_t const& b)
			{ return unplaced_neight(a) < unplaced_neight(b); });

		auto tmp_front = order, tmp_back = order;
		auto node = todo.takeFirst();

		tmp_back.append(node);
		tmp_front.prepend(node);

		unsigned int back_cross = 0, front_cross = 0;

		for(edge_t const& e: unsorted)
		{
			if(e.first == node || e.second == node)
			{
				back_cross += crossings(e,tmp_back);
				front_cross += crossings(e,tmp_front);
			}
		}

		if(front_cross < back_cross)
			order = tmp_front;
		else
			order = tmp_back;
	}

	std::sort(unsorted.begin(),unsorted.end(),[&](edge_t const& b, edge_t const& a)
	{
		boost::optional<std::tuple<node_t,node_t,node_t>> t;

		if(a.first == b.first)
			t = boost::make_optional(std::make_tuple(a.first,a.second,b.second));
		else if(a.first == b.second)
			t = boost::make_optional(std::make_tuple(a.first,a.second,b.first));
		else if(a.second == b.first)
			t = boost::make_optional(std::make_tuple(b.first,a.first,b.second));
		else if(a.second == b.second)
			t = boost::make_optional(std::make_tuple(a.second,a.first,b.first));

		if(t)
		{
			return order.indexOf(std::get<1>(*t)) < order.indexOf(std::get<2>(*t));
		}
		else
		{
			return (order.indexOf(a.first) < order.indexOf(b.first) &&
						 order.indexOf(a.first) < order.indexOf(b.second)) ||
						 (order.indexOf(a.second) < order.indexOf(b.first) &&
						 order.indexOf(a.second) < order.indexOf(b.second));
		}
	});

	return unsorted;
}*/
