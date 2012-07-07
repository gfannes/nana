/*
 *	A Progress Indicator Implementation
 *	Copyright(C) 2003-2012 Jinhao(cnjinhao@hotmail.com)
 *
 *	Distributed under the Nana Software License, Version 1.0.
 *	(See accompanying file LICENSE_1_0.txt or copy at
 *	http://stdex.sourceforge.net/LICENSE_1_0.txt)
 *
 *	@file: nana/gui/widgets/progress.cpp
 */

#include <nana/gui/widgets/progress.hpp>

namespace nana
{
namespace gui
{
	namespace drawerbase
	{
		namespace progress
		{
			//class trigger
			trigger::trigger()
				:   graph_(nullptr), draw_width_(static_cast<unsigned>(-1)), has_value_(true),
                    style_(style::known), max_(100), value_(0)
			{}

			void trigger::bind_window(trigger::widget_reference wd)
			{
				widget_ = &wd;
			}

			void trigger::attached(trigger::graph_reference graph)
			{
				graph_ = &graph;
			}

			unsigned trigger::value() const
			{
				return value_;
			}

			unsigned trigger::value(unsigned v)
			{
				nana::gui::internal_scope_guard isg;
				if(style_ == style::known)
				{
					if(value_ != v)
						value_ = v > max_?max_:v;
				}
				else if(style_ == style::unknown)
					value_ += (v?10:v);

				if(_m_check_changing(value_))
				{
					_m_draw();
					nana::gui::API::update_window(widget_->handle());
				}
				return v;
			}

			unsigned trigger::inc()
			{
				nana::gui::internal_scope_guard isg;
				if(style_ == style::known)
				{
					if(value_ < max_)
						++value_;
				}
				else if(style_ == style::unknown)
					value_ += 10;

				if(_m_check_changing(value_))
					nana::gui::API::refresh_window(widget_->handle());
				return value_;
			}

			unsigned trigger::Max() const
			{
				return max_;
			}

			unsigned trigger::Max(unsigned value)
			{
				max_ = value;
				if(max_ == 0)	++max_;

				API::refresh_window(widget_->handle());
				return max_;
			}

			void trigger::Style(bool known)
			{
				if(false == known)
				{
					style_ = style::unknown;
					draw_width_ = static_cast<unsigned int>(-1);
				}
				else
					style_ = style::known;
			}

			void trigger::refresh(trigger::graph_reference)
			{
				_m_draw();
			}

			void trigger::_m_draw()
			{
				if(style_ == style::known)
					draw_width_ = static_cast<unsigned>((graph_->width() - border * 2) * (double(value_) / max_));

				_m_draw_box(*graph_);
				_m_draw_progress(*graph_);
			}

			void trigger::_m_draw_box(trigger::graph_reference graph)
			{
				unsigned width = graph.width();
				unsigned height = graph.height();
				graph.shadow_rectangle(0, 0, width, height, gui::color::button_face_shadow_end, gui::color::button_face_shadow_start, true);
				graph.line(0, height - 2, 0, 0, 0x808080);
				graph.line(0, 0, width - 2, 0, 0x808080);

				int right = width - 1;
				int bottom = height - 1;
				graph.line(0, bottom, right, bottom, 0xFFFFFF);
				graph.line(right, 0, right, bottom, 0xFFFFFF);
			}

			void trigger::_m_draw_progress(trigger::graph_reference graph)
			{
				unsigned width = graph.width() - border * 2;
				unsigned height = graph.height() - border * 2;

				if(style_ == style::known)
				{
					if(draw_width_)
						graph.shadow_rectangle(border, border, draw_width_, height, 0x6FFFA8, 0x107515, true);
				}
				else if(style_ == style::unknown)
				{
					unsigned block = width / 3;

					int left = (value_ < block ? 0 : value_ - block) + border;
					int right = (value_ >= width - 1 + border? width - 1 + border: value_);

					graph.shadow_rectangle(left, border, right - left + 1, height, 0x6FFFA8, 0x107515, true);

					if(value_ >= width + block)	value_ = 0;
				}
			}

			bool trigger::_m_check_changing(unsigned newvalue) const
			{
				if(graph_)
					return (((graph_->width() - border * 2) * newvalue / max_) != draw_width_);
				return false;
			}
			//end class drawer
		}//end namespace progress
	}//end namespace drawerbase

	//class progress
		progress::progress(){}

		progress::progress(window wd)
		{
			create(wd, 0, 0, 0, 0);
		}

		progress::progress(window wd, const nana::rectangle & r)
		{
			create(wd, r.x, r.y, r.width, r.height);
		}

		progress::progress(window wd, int x, int y, unsigned width, unsigned height)
		{
			create(wd, x, y, width, height);
		}

		unsigned progress::value() const
		{
			return get_drawer_trigger().value();
		}

		unsigned progress::value(unsigned val)
		{
			nana::gui::internal_scope_guard isg;
			if(API::empty_window(this->handle()) == false)
				return get_drawer_trigger().value(val);
			return 0;
		}

		unsigned progress::inc()
		{
			nana::gui::internal_scope_guard isg;
			return get_drawer_trigger().inc();
		}

		unsigned progress::amount() const
		{
			return get_drawer_trigger().Max();
		}

		unsigned progress::amount(unsigned value)
		{
			return get_drawer_trigger().Max(value);
		}

		void progress::style(bool known)
		{
			get_drawer_trigger().Style(known);
		}
	//end class progress
}//end namespace gui
}//end namespace nana
