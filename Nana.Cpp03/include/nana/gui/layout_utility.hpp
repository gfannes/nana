/*
 *	Utility Implementation
 *	Copyright(C) 2003-2012 Jinhao(cnjinhao@hotmail.com)
 *
 *	Distributed under the Nana Software License, Version 1.0. 
 *	(See accompanying file LICENSE_1_0.txt or copy at 
 *	http://stdex.sourceforge.net/LICENSE_1_0.txt)
 *
 *	@file: nana/gui/layout_utility.hpp
 *
 *	
 */

#ifndef NANA_GUI_LAYOUT_UTILITY_HPP
#define NANA_GUI_LAYOUT_UTILITY_HPP

#include "basis.hpp"

namespace nana
{
namespace gui
{
	//overlap test if overlaped between r1 and r2
	bool overlap(const rectangle& r1, const rectangle& r2);
	bool overlap(int x, int y, unsigned width, unsigned height,
				 int x2, int y2, unsigned width2, unsigned height2);

	// overlap, compute the overlap area between r1 and r2. the r is for root
	bool overlap(const rectangle& r1, const rectangle& r2, rectangle& r);

	bool overlap(int x, int y, unsigned width, unsigned height,
				 int x2, int y2, unsigned width2, unsigned height2,
				 rectangle& r);

	bool overlap(const rectangle& ir, const size& valid_input_area, const rectangle & dr, const size& valid_dst_area, rectangle& output_src_r, rectangle& output_dst_r);

	bool intersection(const rectangle & r, point pos_beg, point pos_end, point& good_pos_beg, point& good_pos_end);

	//zoom
	//@brief:	Calculate the scaled rectangle by refer dst rectangle, that scale factor is same as that between scaled and refer.
	void zoom(const rectangle& refer, const rectangle& scaled, const rectangle& refer_dst, rectangle& r);

	//covered
	//@brief:	Tests a rectangle whether it is wholly covered by another.
	bool covered(const rectangle& underlying, //Rectangle 1 is must under rectangle 2
						const rectangle& cover);

	int get_min(int x, int y);
	int get_max(int x, int y);

	bool is_hit_the_rectangle(const rectangle& r, int x, int y);
}//end namespace gui
}//end namespace nana
#endif