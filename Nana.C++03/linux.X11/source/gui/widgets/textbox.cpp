/*
 *	A Textbox Implementation
 *	Copyright(C) 2003-2012 Jinhao(cnjinhao@hotmail.com)
 *
 *	Distributed under the Nana Software License, Version 1.0.
 *	(See accompanying file LICENSE_1_0.txt or copy at
 *	http://stdex.sourceforge.net/LICENSE_1_0.txt)
 *
 *	@file: nana/gui/widgets/textbox.hpp
 */

#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/skeletons/text_editor.hpp>
#include <stdexcept>

namespace nana{ namespace gui{ namespace drawerbase {
	namespace textbox
	{
	//class draweer
		drawer::drawer()
			: widget_(0), editor_(0)
		{
			status_.border = true;
			status_.has_focus = false;
		}

		bool drawer::border(bool has_border)
		{
			if(status_.border != has_border)
			{
				status_.border = has_border;
				return true;
			}
			return false;
		}

		drawer::text_editor* drawer::editor()
		{
			return editor_;
		}

		const drawer::text_editor* drawer::editor() const
		{
			return editor_;
		}
	//private:
		void drawer::bind_window(nana::gui::widget& widget)
		{
			widget_ = &widget;
		}

		void drawer::attached(nana::paint::graphics& graph)
		{
			window wd = widget_->handle();
			editor_ = new text_editor(wd, graph);
			editor_->border_renderer(nana::make_fun(*this, &drawer::_m_draw_border));

			_m_text_area(graph.width(), graph.height());

			using namespace API::dev;
			make_drawer_event<events::focus>(wd);
			make_drawer_event<events::key_char>(wd);
			make_drawer_event<events::key_down>(wd);
			make_drawer_event<events::mouse_down>(wd);
			make_drawer_event<events::mouse_up>(wd);
			make_drawer_event<events::mouse_move>(wd);
			make_drawer_event<events::mouse_wheel>(wd);
			make_drawer_event<events::mouse_enter>(wd);
			make_drawer_event<events::mouse_leave>(wd);

			API::tabstop(wd);
			API::eat_tabstop(wd, true);
			effects::edge_nimbus(wd, effects::edge_nimbus_active);
			effects::edge_nimbus(wd, effects::edge_nimbus_over);
		}

		void drawer::detached()
		{
			delete editor_;
			editor_ = 0;
			API::dev::umake_drawer_event(widget_->handle());
		}

		void drawer::refresh(nana::paint::graphics& graph)
		{
			editor_->redraw(status_.has_focus);
		}

		void drawer::focus(nana::paint::graphics& graph, const nana::gui::eventinfo& ei)
		{
			status_.has_focus = ei.focus.getting;
			refresh(graph);

			editor_->show_caret(status_.has_focus);
			editor_->reset_caret();
			API::lazy_refresh();
		}

		void drawer::mouse_down(graph_reference graph, const nana::gui::eventinfo& ei)
		{
			if(editor_->mouse_down(ei.mouse.left_button, ei.mouse.x, ei.mouse.y))
				API::lazy_refresh();
		}

		void drawer::mouse_move(graph_reference graph, const nana::gui::eventinfo& ei)
		{
			if(editor_->mouse_move(ei.mouse.left_button, ei.mouse.x, ei.mouse.y))
				API::lazy_refresh();
		}

		void drawer::mouse_up(graph_reference graph, const nana::gui::eventinfo& ei)
		{
			if(editor_->mouse_up(ei.mouse.left_button, ei.mouse.x, ei.mouse.y))
				API::lazy_refresh();
		}

		void drawer::mouse_wheel(graph_reference graph, const nana::gui::eventinfo& ei)
		{
			if(editor_->scroll(ei.wheel.upwards, true))
			{
				editor_->reset_caret();
				API::lazy_refresh();
			}
		}

		void drawer::mouse_enter(graph_reference graph, const nana::gui::eventinfo&)
		{
			if(editor_->mouse_enter(true))
				API::lazy_refresh();
		}

		void drawer::mouse_leave(graph_reference graph, const nana::gui::eventinfo&)
		{
			if(editor_->mouse_enter(false))
				API::lazy_refresh();
		}

		void drawer::key_down(graph_reference graph, const nana::gui::eventinfo& ei)
		{
			if(editor_->move(ei.keyboard.key))
			{
				editor_->reset_caret();
				API::lazy_refresh();
			}
		}

		void drawer::key_char(graph_reference graph, const nana::gui::eventinfo& ei)
		{
			using namespace nana::gui;

			if(editor_->editable())
			{
				switch(ei.keyboard.key)
				{
				case '\b':
					editor_->backspace();	break;
				case '\n': case '\r':
					editor_->enter();	break;
				case keyboard::cancel:
					editor_->copy();	break;
				case keyboard::sync:
					editor_->paste();	break;
				case keyboard::tab:
					editor_->put(keyboard::tab); break;
				default:
					if(ei.keyboard.key >= 0xFF || (32 <= ei.keyboard.key && ei.keyboard.key <= 126))
						editor_->put(ei.keyboard.key);
					else if(sizeof(nana::char_t) == sizeof(char))
					{	//Non-Unicode Version for Non-English characters
						if(ei.keyboard.key & (1<<(sizeof(nana::char_t)*8 - 1)))
							editor_->put(ei.keyboard.key);
					}
				}
				editor_->reset_caret();
				API::lazy_refresh();
			}
			else if(ei.keyboard.key == keyboard::cancel)
				editor_->copy();
		}

		void drawer::resize(graph_reference graph, const nana::gui::eventinfo& ei)
		{
			_m_text_area(ei.size.width, ei.size.height);
			refresh(graph);
			API::lazy_refresh();
		}

		void drawer::_m_text_area(unsigned width, unsigned height)
		{
			if(editor_)
			{
				nana::rectangle r(0, 0, width, height);
				if(status_.border)
				{
					r.x = r.y = 2;
					r.width = (width > 4 ? width - 4 : 0);
					r.height = (height > 4 ? height - 4 : 0);
				}
				editor_->text_area(r);
			}
		}

		void drawer::_m_draw_border(graph_reference graph)
		{
			if(status_.border)
			{
				nana::rectangle r(graph.size());
				graph.rectangle(r, (status_.has_focus ? 0x0595E2 : 0x999A9E), false);
				r.x = r.y = 1;
				r.width -= 2;
				r.height -= 2;
				graph.rectangle(r, 0xFFFFFF, false);
			}
		}
	//end class drawer
}//end namespace textbox
}//end namespace drawerbase

	//class textbox
		textbox::textbox()
		{
		}

		textbox::textbox(window wd)
		{
			create(wd, 0, 0, 0, 0);
		}

		textbox::textbox(window wd, const nana::rectangle& r)
		{
			create(wd, r.x, r.y, r.width, r.height);
		}

		textbox::textbox(window wd, int x, int y, unsigned width, unsigned height)
		{
			create(wd, x, y, width, height);
		}

		bool textbox::getline(size_t n, nana::string& text) const
		{
			const drawerbase::textbox::drawer::text_editor * editor = get_drawer_trigger().editor();
			return (editor ? editor->getline(n, text) : false);
		}

		textbox& textbox::append(const nana::string& text, bool at_caret)
		{
			drawerbase::textbox::drawer::text_editor * editor = get_drawer_trigger().editor();
			if(editor)
			{
				if(at_caret == false)
					editor->move_caret_end();

				editor->put(text);
				API::update_window(this->handle());
			}
			return *this;
		}

		textbox& textbox::border(bool has_border)
		{
			if(get_drawer_trigger().border(has_border))
			{
				drawerbase::textbox::drawer::text_editor * editor = get_drawer_trigger().editor();
				if(editor)
					API::refresh_window(this->handle());
			}
			return *this;
		}

		bool textbox::multi_lines() const
		{
			const drawerbase::textbox::drawer::text_editor * editor = get_drawer_trigger().editor();
			return (editor ? editor->multi_lines() : false);
		}

		textbox& textbox::multi_lines(bool ml)
		{
			drawerbase::textbox::drawer::text_editor * editor = get_drawer_trigger().editor();
			if(editor && editor->multi_lines(ml))
				API::update_window(handle());
			return *this;
		}

		bool textbox::editable() const
		{
			const drawerbase::textbox::drawer::text_editor * editor = get_drawer_trigger().editor();
			return (editor ? editor->editable() : false);
		}

		textbox& textbox::editable(bool able)
		{
			drawerbase::textbox::drawer::text_editor * editor = get_drawer_trigger().editor();
			if(editor)
				editor->editable(able);
			return *this;
		}

		textbox& textbox::tip_string(const nana::string& str)
		{
			internal_scope_guard isg;
			drawerbase::textbox::drawer::text_editor * editor = get_drawer_trigger().editor();
			if(editor && editor->tip_string(str))
				API::refresh_window(this->handle());
			return *this;
		}

		textbox& textbox::mask(nana::char_t ch)
		{
			drawerbase::textbox::drawer::text_editor * editor = get_drawer_trigger().editor();
			if(editor && editor->mask(ch))
				API::refresh_window(this->handle());
			return *this;
		}

		bool textbox::selected() const
		{
			internal_scope_guard isg;
			const drawerbase::textbox::drawer::text_editor * editor = get_drawer_trigger().editor();
			return (editor ? editor->selected() : false);
		}

		void textbox::select_all()
		{
			internal_scope_guard isg;
			widgets::skeletons::text_editor * editor = get_drawer_trigger().editor();
			if(editor)
			{
				editor->select_all();
				API::refresh_window(*this);
			}
		}

		void textbox::select_cancel()
		{
			internal_scope_guard isg;
			widgets::skeletons::text_editor * editor = get_drawer_trigger().editor();
			if(editor && editor->cancel_select())
				API::refresh_window(*this);
		}

		void textbox::copy() const
		{
			internal_scope_guard isg;
			const drawerbase::textbox::drawer::text_editor * editor = get_drawer_trigger().editor();
			if(editor)
				editor->copy();
		}

		void textbox::paste()
		{
			internal_scope_guard isg;
			drawerbase::textbox::drawer::text_editor * editor = get_drawer_trigger().editor();
			if(editor)
			{
				editor->paste();
				API::refresh_window(*this);
			}
		}

		void textbox::del()
		{
			internal_scope_guard isg;
			drawerbase::textbox::drawer::text_editor * editor = get_drawer_trigger().editor();
			if(editor)
			{
				editor->del();
				API::refresh_window(*this);
			}
		}

		//Override _m_caption for caption()
		nana::string textbox::_m_caption() const
		{
			internal_scope_guard isg;
			const drawerbase::textbox::drawer::text_editor * editor = get_drawer_trigger().editor();
			return (editor ? editor->text() : nana::string());
		}

		void textbox::_m_caption(const nana::string& str)
		{
			internal_scope_guard isg;
			drawerbase::textbox::drawer::text_editor * editor = get_drawer_trigger().editor();
			if(editor)
			{
				editor->text(str);
				API::update_window(this->handle());
			}
		}

		//Override _m_typeface for changing the caret
		void textbox::_m_typeface(const nana::paint::font& font)
		{
			widget::_m_typeface(font);
			drawerbase::textbox::drawer::text_editor * editor = get_drawer_trigger().editor();
			if(editor)
				editor->reset_caret_height();
		}
	//end class textbox
}//end namespace gui
}//end namespace nana

