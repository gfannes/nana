/*
 *	Platform Specification Implementation
 *	Copyright(C) 2003-2012 Jinhao(cnjinhao@hotmail.com)
 *
 *	Distributed under the Nana Software License, Version 1.0. 
 *	(See accompanying file LICENSE_1_0.txt or copy at 
 *	http://stdex.sourceforge.net/LICENSE_1_0.txt)
 *
 *	@file: nana/detail/win32/platform_spec.hpp
 *
 *	This file provides basis class and data structrue that required by nana
 *	This file should not be included by any header files.
 */

#ifndef NANA_DETAIL_PLATFORM_SPEC_HPP
#define NANA_DETAIL_PLATFORM_SPEC_HPP

#include <nana/deploy.hpp>
#include <nana/gui/basis.hpp>
#include <nana/paint/image.hpp>
#include <nana/refer.hpp>
#include <windows.h>
#include <map>

namespace nana
{

namespace detail
{
	//struct messages
	//@brief:	This defines some messages that are used for remote thread invocation.
	//			Some Windows APIs are window-thread-dependent, the operation in other thread
	//			must be posted to its own thread.
	struct messages
	{
		struct caret
		{
			int x;
			int y;
			unsigned width;
			unsigned height;
			bool visible;
		};

		struct move_window
		{
			enum { Pos = 1, Size = 2};
			int x;
			int y;
			unsigned width;
			unsigned height;
			unsigned ignore; //determinate that pos or size would be ignored.
		};

		enum
		{
			tray = 0x501,
			async_active_owner,
			async_set_focus,
			map_thread_root_buffer,
			remote_thread_destroy_window,
			remote_thread_move_window,
			operate_caret,	//wParam: 1=Destroy, 2=SetPos
			remote_thread_set_window_pos,
			remote_thread_set_window_text,
			user,
		};
	};

	struct font_tag
	{
		nana::string name;
		unsigned height;
		unsigned weight;
		bool italic;
		bool underline;
		bool strikeout;
		HFONT handle;

		struct deleter
		{
			void operator()(const font_tag*) const;
		};
	};

	struct drawable_impl_type
	{
		typedef nana::refer<font_tag*, font_tag::deleter> font_refer_t;

		HDC		context;
		HBITMAP	pixmap;
		font_refer_t	font;

		struct pen_spec
		{
			HPEN	handle;
			unsigned color;
			int style;
			int width;

			void set(HDC context, int style, int width, nana::color_t color);
		}pen;

		struct brush_spec
		{
			enum t{Solid, HatchBDiagonal};

			HBRUSH handle;
			t style;
			nana::color_t color;

			void set(HDC context, t style, nana::color_t color);
			void remove();
		}brush;

		struct round_region_spec
		{
			HRGN handle;
			nana::rectangle r;
			unsigned radius_x;
			unsigned radius_y;

			void set(const nana::rectangle& r, unsigned radius_x, unsigned radius_y);
		}round_region;

		struct string_spec
		{
			unsigned tab_length;
			unsigned tab_pixels;
			unsigned whitespace_pixels;
		}string;

		drawable_impl_type();
		~drawable_impl_type();

		void fgcolor(nana::color_t);
	private:
		nana::color_t fgcolor_;
	};

	class platform_spec
	{
	public:
		typedef drawable_impl_type::font_refer_t font_refer_t;

		platform_spec();

		const font_refer_t& default_native_font() const;
		unsigned font_size_to_height(unsigned) const;
		unsigned font_height_to_size(unsigned) const;
		font_refer_t make_native_font(const nana::char_t* name, unsigned height, unsigned weight, bool italic, bool underline, bool strike_out);

		void event_register_filter(nana::gui::native_window_type, unsigned eventid);
		static platform_spec& instance();

		void keep_window_icon(nana::gui::native_window_type, const nana::paint::image&);
		void release_window_icon(nana::gui::native_window_type);
	private:
		font_refer_t def_font_ref_;
		std::map<nana::gui::native_window_type, nana::paint::image> iconbase_;
	};

}//end namespace detail
}//end namespace nana
#endif
