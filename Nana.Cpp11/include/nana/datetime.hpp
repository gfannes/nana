/*
 *	A Date Time Implementation
 *	Copyright(C) 2003-2013 Jinhao(cnjinhao@hotmail.com)
 *
 *	Distributed under the Boost Software License, Version 1.0.
 *	(See accompanying file LICENSE_1_0.txt or copy at
 *	http://www.boost.org/LICENSE_1_0.txt)
 *
 *	@file: nana/datetime.hpp
 */

#ifndef NANA_DATETIME_HPP
#define NANA_DATETIME_HPP
#include <ctime>

namespace nana
{
	class date
	{
	public:
		struct value
		{
			unsigned year;	//1601 - 30827
			unsigned month;	//1-12
			unsigned day;	//1-31
		};

		date();
		date(const std::tm&);
		date(int year, int month, int day);

		date operator - (int off) const;
		date operator + (int off) const;
		bool operator==(const date&) const;
		bool operator!=(const date&) const;
		bool operator<(const date&) const;
		bool operator>(const date&) const;
		bool operator<=(const date&) const;
		bool operator>=(const date&) const;

		int day_of_week() const;
		const value & read() const;

		static int day_of_week(int year, int month, int day);
		static unsigned year_days(unsigned year);
		static unsigned month_days(unsigned year, unsigned month);
		static unsigned day_in_year(unsigned y, unsigned m, unsigned d);
	private:
		date _m_add(unsigned x) const;
		date _m_sub(unsigned x) const;
	private:
		value	value_;
	}; //end class date

	class time
	{
	public:
		struct value
		{
			unsigned hour;		//[0-23]
			unsigned minute;	//[0-59]
			unsigned second;	//[0-61], the range of [60, 61] is used for leap seconds
		};

		time();
		time(const std::tm&);
		time(unsigned hour, unsigned minute, unsigned second);
		const value& read() const;
	private:
		value	value_;
	};//end class time
}//end namespace nana

#endif
