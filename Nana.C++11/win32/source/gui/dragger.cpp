
#include <nana/gui/dragger.hpp>

namespace nana{ namespace gui{


	class dragger::dragger_impl_t
	{
		struct drag_target_t
		{
			nana::gui::window wd;
			nana::point origin;
		};

		struct trigger_t
		{
			nana::gui::window wd;
			nana::gui::event_handle press;
			nana::gui::event_handle over;
			nana::gui::event_handle release;
			event_handle destroy;
		};
	public:
		dragger_impl_t()
			: dragging_(false)
		{}

		~dragger_impl_t()
		{
			_m_clear_triggers();
		}

		void drag_target(nana::gui::window wd)
		{
			drag_target_t dt;
			dt.wd = wd;
			targets_.push_back(dt);
		}

		void trigger(nana::gui::window wd)
		{
			trigger_t tg;
			tg.wd = wd;
			auto f = std::bind(&dragger_impl_t::_m_trace, this, std::placeholders::_1);
			tg.press = API::make_event<events::mouse_down>(wd, f);
			tg.over = API::make_event<events::mouse_move>(wd, f);
			tg.release = API::make_event<events::mouse_up>(wd, f);
			tg.destroy = API::make_event<events::destroy>(wd, f);

			triggers_.push_back(tg);
		}
	private:
		void _m_clear_triggers()
		{
			for(std::vector<trigger_t>::iterator i = triggers_.begin(); i != triggers_.end(); ++i)
			{
				API::umake_event(i->press);
				API::umake_event(i->over);
				API::umake_event(i->release);
				API::umake_event(i->destroy);
			}
			triggers_.clear();
		}

		void _m_destroy(const eventinfo& ei)
		{
			for(std::vector<trigger_t>::iterator i = triggers_.begin(); i != triggers_.end(); ++i)
			{
				if(i->wd == ei.window)
				{
					triggers_.erase(i);
					return;
				}
			}
		}

		void _m_trace(const eventinfo& ei)
		{
			switch(ei.identifier)
			{
			case events::mouse_down::identifier:
				dragging_ = true;
				API::capture_window(ei.window, true);
				origin_ = API::cursor_position();
				for(std::vector<drag_target_t>::iterator i = targets_.begin(); i != targets_.end(); ++i)
				{
					i->origin = API::window_position(i->wd);
					nana::gui::window owner = API::get_owner_window(i->wd);
					if(owner)
						API::calc_screen_point(owner, i->origin);
				}
				break;
			case events::mouse_move::identifier:
				if(dragging_ && ei.mouse.left_button)
				{
					nana::point pos = API::cursor_position();
					pos.x -= origin_.x;
					pos.y -= origin_.y;
					for(std::vector<drag_target_t>::iterator i = targets_.begin(); i != targets_.end(); ++i)
					{
						if(API::is_window_zoomed(i->wd, true) == false)
						{
							nana::gui::window owner = API::get_owner_window(i->wd);
							if(owner)
							{
								nana::point t = i->origin;
								API::calc_window_point(owner, t);
								t.x += pos.x;
								t.y += pos.y;
								API::move_window(i->wd, t.x, t.y);
							}
							else
								API::move_window(i->wd, i->origin.x + pos.x, i->origin.y + pos.y);
						}
					}
				}
				break;
			case events::mouse_up::identifier:
				API::capture_window(ei.window, false);
				dragging_ = false;
				break;
			}
		}
		
	private:
		bool dragging_;
		nana::point origin_;
		std::vector<drag_target_t> targets_;
		std::vector<trigger_t> triggers_;
	};

	//class dragger
		dragger::dragger()
			: impl_(new dragger_impl_t)
		{
		}

		dragger::~dragger()
		{
			delete impl_;
		}

		void dragger::drag_target(nana::gui::window wd)
		{
			impl_->drag_target(wd);
		}

		void dragger::trigger(nana::gui::window tg)
		{
			impl_->trigger(tg);
		}
	//end class dragger

}//end namespace gui
}//end namespace nana
