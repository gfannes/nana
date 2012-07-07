/*
 *	Pixel Buffer Implementation
 *	Copyright(C) 2003-2012 Jinhao(cnjinhao@hotmail.com)
 *
 *	Distributed under the Nana Software License, Version 1.0.
 *	(See accompanying file LICENSE_1_0.txt or copy at
 *	http://stdex.sourceforge.net/LICENSE_1_0.txt)
 *
 *	@file: nana/paint/pixel_buffer.cpp
 */
#include <nana/paint/pixel_buffer.hpp>
#include <nana/detail/platform_spec.hpp>
#include <nana/gui/layout_utility.hpp>
#include <nana/paint/detail/native_paint_interface.hpp>
#include <nana/paint/detail/image_process_provider.hpp>

namespace nana{	namespace paint
{
	struct pixel_buffer::pixel_buffer_storage
		: private nana::noncopyable
	{
	public:
		pixel_rgb_t * const raw_pixel_buffer;
		const nana::size pixel_size;

		struct image_processor_tag
		{
			paint::image_process::stretch_interface * stretch;
			paint::image_process::blend_interface * blend;
			paint::image_process::line_interface * line;

			image_processor_tag()
				: stretch(nullptr), blend(nullptr), line(nullptr)
			{}
		}img_pro;

		pixel_buffer_storage(std::size_t width, std::size_t height)
			:	raw_pixel_buffer(new pixel_rgb_t[width * height]),
				pixel_size(static_cast<unsigned>(width), static_cast<unsigned>(height))
		{}

		~pixel_buffer_storage()
		{
			delete [] raw_pixel_buffer;
		}
	};

	pixel_buffer::pixel_buffer()
	{}

	pixel_buffer::pixel_buffer(drawable_type drawable, const nana::rectangle& want_rectangle)
	{
		open(drawable, want_rectangle);
	}

	pixel_buffer::pixel_buffer(drawable_type drawable, std::size_t top, std::size_t lines)
	{
		open(drawable, nana::rectangle(0, static_cast<int>(top), 0, static_cast<unsigned>(lines)));
	}

	pixel_buffer::pixel_buffer(std::size_t width, std::size_t height)
	{
		open(width, height);
	}

	pixel_buffer::~pixel_buffer()
	{
		close();
	}

	bool pixel_buffer::open(drawable_type drawable, const nana::rectangle & want_rectangle)
	{
		nana::size sz = nana::paint::detail::drawable_size(drawable);
		if(want_rectangle.x >= static_cast<int>(sz.width) || want_rectangle.y >= static_cast<int>(sz.height))
			return false;

		nana::rectangle want_r = want_rectangle;
		if(want_r.width == 0) want_r.width = sz.width - want_r.x;
		if(want_r.height == 0) want_r.height = sz.height - want_r.y;

		nana::rectangle r;
		if(false == nana::gui::overlap(nana::rectangle(0, 0, sz.width, sz.height), want_r, r))
			return false;
#if defined(NANA_WINDOWS)
		BITMAPINFO bmpinfo;
		bmpinfo.bmiHeader.biSize = sizeof(bmpinfo.bmiHeader);
		bmpinfo.bmiHeader.biWidth = want_r.width;
		bmpinfo.bmiHeader.biHeight = -static_cast<int>(want_r.height);
		bmpinfo.bmiHeader.biPlanes = 1;
		bmpinfo.bmiHeader.biBitCount = 32;
		bmpinfo.bmiHeader.biCompression = BI_RGB;
		bmpinfo.bmiHeader.biSizeImage = static_cast<DWORD>(want_r.width * want_r.height * sizeof(pixel_rgb_t));
		bmpinfo.bmiHeader.biClrUsed = 0;
		bmpinfo.bmiHeader.biClrImportant = 0;

		bool need_dup = (r.width != sz.width || r.height != sz.height);

		HDC context = drawable->context;
		HBITMAP pixmap = drawable->pixmap;
		HBITMAP orig_bmp;
		if(need_dup)
		{
			context = ::CreateCompatibleDC(drawable->context);
			pixmap = ::CreateCompatibleBitmap(drawable->context, static_cast<int>(want_r.width), static_cast<int>(want_r.height));
			orig_bmp = reinterpret_cast<HBITMAP>(::SelectObject(context, pixmap));
			::BitBlt(context, r.x - want_r.x, r.y - want_r.y, r.width, r.height, drawable->context, r.x, r.y, SRCCOPY);
		}

		storage_ref_ = new pixel_buffer_storage(want_r.width, want_r.height);
		std::size_t read_lines = ::GetDIBits(context, pixmap, 0, static_cast<UINT>(want_r.height), storage_ref_.handle()->raw_pixel_buffer, &bmpinfo, DIB_RGB_COLORS);

		if(need_dup)
		{
			::SelectObject(context, orig_bmp);
			::DeleteObject(pixmap);
			::DeleteDC(context);
		}

		return (want_r.height == read_lines);
#elif defined(NANA_X11)
		nana::detail::platform_spec & spec = nana::detail::platform_spec::instance();
		Window root;
		int x, y;
		unsigned width, height;
		unsigned border, depth;
		nana::detail::platform_scope_guard psg;
		::XGetGeometry(spec.open_display(), drawable->pixmap, &root, &x, &y, &width, &height, &border, &depth);

		XImage * image = ::XGetImage(spec.open_display(), drawable->pixmap, r.x, r.y, r.width, r.height, AllPlanes, ZPixmap);
		storage_ref_ = new pixel_buffer_storage(want_r.width, want_r.height);
		pixel_rgb_t * pixbuf = storage_ref_.handle()->raw_pixel_buffer;
		unsigned pixbuf_width = storage_ref_.handle()->pixel_size.width;
		if(image->depth == 32 || (image->depth == 24 && image->bitmap_pad == 32))
		{
			if(want_r.width != image->width || want_r.height != image->height)
			{
				pixbuf += (r.x - want_r.x);
				pixbuf += (r.y - want_r.y) * want_r.width;
				const char* img_data = image->data;
				for(int i = 0; i < image->height; ++i)
				{
					memcpy(pixbuf, img_data, image->bytes_per_line);
					img_data += image->bytes_per_line;
					pixbuf += want_r.width;
				}
			}
			else
				memcpy(pixbuf, image->data, image->bytes_per_line * image->height);
		}
		else
		{
			XDestroyImage(image);
			return false;
		}

		XDestroyImage(image);
#endif
        return true;
	}

	bool pixel_buffer::open(std::size_t width, std::size_t height)
	{
		if(width && height)
		{
			storage_ref_ = new pixel_buffer_storage(width, height);
			return true;
		}
		return false;
	}

	void pixel_buffer::close()
	{
		storage_ref_.release();
	}

	bool pixel_buffer::empty() const
	{
		return storage_ref_.empty();
	}

	pixel_buffer::operator const void*() const
	{
		return (storage_ref_.empty() ? 0 : this);
	}

	std::size_t pixel_buffer::bytes() const
	{
		if(storage_ref_.empty() == false)
		{
			nana::size sz = storage_ref_.handle()->pixel_size;
			return sizeof(pixel_rgb_t) * (static_cast<size_t>(sz.width) * static_cast<size_t>(sz.height));
		}
		return 0;
	}

	nana::size pixel_buffer::size() const
	{
		return (storage_ref_.empty() ? nana::size(0, 0) : storage_ref_.handle()->pixel_size);
	}

	pixel_rgb_t * pixel_buffer::raw_ptr() const
	{
		return (storage_ref_.empty() ? 0 : storage_ref_.handle()->raw_pixel_buffer);
	}

	pixel_rgb_t * pixel_buffer::raw_ptr(std::size_t row) const
	{
		if(!storage_ref_.empty())
		{
			pixel_buffer_storage * sp = storage_ref_.handle();
			if(row < sp->pixel_size.height)
				return sp->raw_pixel_buffer + sp->pixel_size.width * row;
		}
		return 0;
	}

	void pixel_buffer::put(const unsigned char* rawbits, std::size_t width, std::size_t height, std::size_t bits_per_pixel, std::size_t bytes_per_line, bool is_negative)
	{
		pixel_rgb_t* rawptr = (storage_ref_.empty() ? 0 : storage_ref_.handle()->raw_pixel_buffer);
		if(rawptr)
		{
			pixel_buffer_storage * sp = storage_ref_.handle();

			if(32 == bits_per_pixel)
			{
				if((sp->pixel_size.width == width) && (sp->pixel_size.height == height) && is_negative)
				{
					memcpy(rawptr, rawbits, (sp->pixel_size.width * sp->pixel_size.height) * 4);
				}
				else
				{
					std::size_t line_bytes = (sp->pixel_size.width < width ? sp->pixel_size.width : width) * sizeof(pixel_rgb_t);

					if(sp->pixel_size.height < height)
						height = sp->pixel_size.height;

					pixel_rgb_t * d = rawptr;
					if(is_negative)
					{
						const unsigned char* s = rawbits;
						for(std::size_t i = 0; i < height; ++i)
						{
							memcpy(d, s, line_bytes);
							d += sp->pixel_size.width;
							s += bytes_per_line;
						}
					}
					else
					{
						const unsigned char* s = rawbits + bytes_per_line * (height - 1);
						for(std::size_t i = 0; i < height; ++i)
						{
							memcpy(d, s, line_bytes);
							d += sp->pixel_size.width;
							s -= bytes_per_line;
						}
					}
				}
			}
			else if(24 == bits_per_pixel)
			{
				if(sp->pixel_size.width < width)
					width = sp->pixel_size.width;

				if(sp->pixel_size.height < height)
					height = sp->pixel_size.height;

				pixel_rgb_t * d = rawptr;
				if(is_negative)
				{
					const unsigned char* s = rawbits;
					for(std::size_t i = 0; i < height; ++i)
					{
						pixel_rgb_t * p = d;
						pixel_rgb_t * end = p + width;
						std::size_t s_index = 0;
						for(; p < end; ++p)
						{
							const unsigned char * s_p = s + s_index;
							p->u.element.blue = s_p[0];
							p->u.element.green = s_p[1];
							p->u.element.red = s_p[2];
							s_index += 3;
						}
						d += sp->pixel_size.width;
						s += bytes_per_line;
					}
				}
				else
				{
					const unsigned char* s = rawbits + bytes_per_line * (height - 1);
					for(std::size_t i = 0; i < height; ++i)
					{
						pixel_rgb_t * p = d;
						pixel_rgb_t * end = p + width;
						const unsigned char* s_p = s;
						for(; p < end; ++p)
						{
							p->u.element.blue = s_p[0];
							p->u.element.green = s_p[1];
							p->u.element.red = s_p[2];
							s_p += 3;
						}
						d += sp->pixel_size.width;
						s -= bytes_per_line;
					}
				}
			}
			else if(16 == bits_per_pixel)
			{
				if(sp->pixel_size.width < width)
					width = sp->pixel_size.width;

				if(sp->pixel_size.height < height)
					height = sp->pixel_size.height;

				pixel_rgb_t * d = rawptr;

				unsigned char * rgb_table = new unsigned char[32];
				unsigned char * table_block = rgb_table;
				for(std::size_t i = 0; i < 8; i += 4)
				{
					table_block[0] = static_cast<unsigned char>(i << 3);
					table_block[1] = static_cast<unsigned char>((i + 1) << 3);
					table_block[2] = static_cast<unsigned char>((i + 2) << 3);
					table_block[3] = static_cast<unsigned char>((i + 3) << 3);
					table_block += 4;
				}

				if(is_negative)
				{
					const unsigned char* s = rawbits;
					for(std::size_t i = 0; i < height; ++i)
					{
						pixel_rgb_t * p = d;
						const pixel_rgb_t * const end = p + width;
						const unsigned char* s_p = s;
						for(; p < end; ++p)
						{
							p->u.element.blue = rgb_table[(*s_p && 0xF800) >> 11];
							p->u.element.green = rgb_table[(*s_p && 0x7C0) >> 6];
							p->u.element.red = rgb_table[*s_p && 0x1F];
							++s_p;
						}
						d += sp->pixel_size.width;
						s += bytes_per_line;
					}
				}
				else
				{
					const unsigned char* s = rawbits + bytes_per_line * (height - 1);
					for(std::size_t i = 0; i < height; ++i)
					{
						pixel_rgb_t * p = d;
						const pixel_rgb_t * const end = p + width;
						const unsigned char* s_p = s;
						for(; p < end; ++p)
						{
							p->u.element.blue = rgb_table[(*s_p && 0xF800) >> 11];
							p->u.element.green = rgb_table[(*s_p && 0x7C0) >> 6];
							p->u.element.red = rgb_table[*s_p && 0x1F];
							++s_p;
						}
						d += sp->pixel_size.width;
						s -= bytes_per_line;
					}
				}
				delete [] rgb_table;
			}
		}
	}

	pixel_rgb_t pixel_buffer::pixel(int x, int y) const
	{
		pixel_buffer_storage * sp = storage_ref_.handle();
		if(sp && 0 <= x && x < static_cast<int>(sp->pixel_size.width) && 0 <= y && y < static_cast<int>(sp->pixel_size.height))
			return *(sp->raw_pixel_buffer + y * sp->pixel_size.width + x);
		return pixel_rgb_t();
	}

	void pixel_buffer::pixel(int x, int y, pixel_rgb_t px)
	{
		pixel_buffer_storage * sp = storage_ref_.handle();
		if(sp && 0 <= x && x < static_cast<int>(sp->pixel_size.width) && 0 <= y && y < static_cast<int>(sp->pixel_size.height))
		{
			*(sp->raw_pixel_buffer + y * sp->pixel_size.width + x) = px;
		}
	}

	void pixel_buffer::paste(drawable_type drawable, int x, int y) const
	{
		if(!storage_ref_.empty())
		{
			nana::size sz = storage_ref_.handle()->pixel_size;
			this->paste(nana::rectangle(0, 0, sz.width, sz.height), drawable, x, y);
		}
	}

	void pixel_buffer::paste(const nana::rectangle& src_r, drawable_type drawable, int x, int y) const
	{
		if(drawable && !storage_ref_.empty())
		{
			pixel_buffer_storage * sp = storage_ref_.handle();
#if defined(NANA_WINDOWS)
			BITMAPINFO bi;
			bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
			bi.bmiHeader.biWidth = sp->pixel_size.width;
			bi.bmiHeader.biHeight = -static_cast<int>(sp->pixel_size.height);
			bi.bmiHeader.biPlanes = 1;
			bi.bmiHeader.biBitCount = sizeof(pixel_rgb_t) * 8;
			bi.bmiHeader.biCompression = BI_RGB;
			bi.bmiHeader.biSizeImage = static_cast<DWORD>(sizeof(pixel_rgb_t) * sp->pixel_size.width * sp->pixel_size.height);
			bi.bmiHeader.biClrUsed = 0;
			bi.bmiHeader.biClrImportant = 0;

			::SetDIBitsToDevice(drawable->context,
				x, y, src_r.width, src_r.height,
				src_r.x, static_cast<int>(sp->pixel_size.height) - src_r.y - src_r.height, 0, sp->pixel_size.height,
				sp->raw_pixel_buffer, &bi, DIB_RGB_COLORS);
#elif defined(NANA_X11)
			nana::detail::platform_spec & spec = nana::detail::platform_spec::instance();
			XImage * img = ::XCreateImage(spec.open_display(), spec.screen_visual(), spec.screen_depth(), ZPixmap, 0, 0, sp->pixel_size.width, sp->pixel_size.height, 32, 0);
			img->data = reinterpret_cast<char*>(sp->raw_pixel_buffer);
			::XPutImage(spec.open_display(), drawable->pixmap, drawable->context, img, src_r.x, src_r.y, x, y, src_r.width, src_r.height);

			img->data = 0;  //Set null pointer to avoid XDestroyImage destroyes the buffer.
			XDestroyImage(img);
#endif
		}
	}

	void pixel_buffer::paste(nana::gui::native_window_type wd, int x, int y) const
	{
		pixel_buffer_storage * sp = storage_ref_.handle();
		if(wd && (0 == sp))	return;
#if defined(NANA_WINDOWS)
		HDC	handle = ::GetDC(reinterpret_cast<HWND>(wd));
		if(handle)
		{
			BITMAPINFO bi;
			bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
			bi.bmiHeader.biWidth = sp->pixel_size.width;
			bi.bmiHeader.biHeight = -static_cast<int>(sp->pixel_size.height);
			bi.bmiHeader.biPlanes = 1;
			bi.bmiHeader.biBitCount = sizeof(pixel_rgb_t) * 8;
			bi.bmiHeader.biCompression = BI_RGB;
			bi.bmiHeader.biSizeImage = static_cast<DWORD>(sizeof(pixel_rgb_t) * sp->pixel_size.width * sp->pixel_size.height);
			bi.bmiHeader.biClrUsed = 0;
			bi.bmiHeader.biClrImportant = 0;

			::SetDIBitsToDevice(handle,
				x, y, sp->pixel_size.width, sp->pixel_size.height,
				0, 0, 0, sp->pixel_size.height,
				sp->raw_pixel_buffer, &bi, DIB_RGB_COLORS);

			::ReleaseDC(reinterpret_cast<HWND>(wd), handle);
		}
#elif defined(NANA_X11)
			nana::detail::platform_spec & spec = nana::detail::platform_spec::instance();
			Display * disp = spec.open_display();
			XImage * img = ::XCreateImage(disp, spec.screen_visual(), spec.screen_depth(), ZPixmap, 0, 0, sp->pixel_size.width, sp->pixel_size.height, 32, 0);
			img->data = reinterpret_cast<char*>(sp->raw_pixel_buffer);
			::XPutImage(disp, reinterpret_cast<Window>(wd), XDefaultGC(disp, XDefaultScreen(disp)), img, 0, 0, x, y, sp->pixel_size.width, sp->pixel_size.height);

			img->data = 0;  //Set null pointer to avoid XDestroyImage destroyes the buffer.
			XDestroyImage(img);
#endif

	}

	void pixel_buffer::line(const std::string& name)
	{
		pixel_buffer_storage * sp = storage_ref_.handle();
		if(sp)
			sp->img_pro.line = (name.size() ? detail::image_process_provider::instance().ref_line(name) : 0);
	}

	void pixel_buffer::line(const nana::point &pos_beg, const nana::point &pos_end, nana::color_t color, double fade_rate)
	{
		pixel_buffer_storage * sp = storage_ref_.handle();
		if(0 == sp)	return;

		//Test if the line intersects the rectangle, and retrive the two points that
		//are always in the area of rectangle, good_pos_beg is left point, good_pos_end is right.
		nana::point good_pos_beg, good_pos_end;
		if(false == nana::gui::intersection(nana::rectangle(sp->pixel_size), pos_beg, pos_end, good_pos_beg, good_pos_end))
			return;

		(sp->img_pro.line ? sp->img_pro.line : detail::image_process_provider::instance().line())
			->process(*this, good_pos_beg, good_pos_end, color, fade_rate);
	}

	void pixel_buffer::rectangle(const nana::rectangle &r, nana::color_t col, double fade_rate, bool solid)
	{
		pixel_buffer_storage * sp = storage_ref_.handle();
		if((0 == sp) || (fade_rate == 1.0)) return;

		bool fade = (fade_rate != 0.0);
		unsigned char * fade_table = 0;
		nana::pixel_rgb_t rgb_imd;
		if(fade)
		{
			fade_table = detail::fade_table(1 - fade_rate);
			rgb_imd.u.color = col;
			rgb_imd = detail::fade_color_intermedia(rgb_imd, fade_table);
		}

		int xbeg = (0 <= r.x ? r.x : 0);
		int xend = static_cast<int>(r.x + r.width < sp->pixel_size.width ? r.x + r.width : sp->pixel_size.width);
		int ybeg = (0 <= r.y ? r.y : 0);
		int yend = static_cast<int>(r.y + r.height < sp->pixel_size.height ? r.y + r.height : sp->pixel_size.height);

		if((ybeg == r.y) && (r.y + static_cast<int>(r.height) == yend))
		{
			nana::pixel_rgb_t * p_rgb = sp->raw_pixel_buffer + ybeg * sp->pixel_size.width;
			nana::pixel_rgb_t * i = p_rgb + xbeg;
			nana::pixel_rgb_t * end = p_rgb + xend;
			nana::pixel_rgb_t * i_other = sp->raw_pixel_buffer + (yend - 1) * sp->pixel_size.width + xbeg;
			if(fade)
			{
				for(;i != end; ++i, ++i_other)
				{
					*i = detail::fade_color_by_intermedia(*i, rgb_imd, fade_table);
					*i_other = detail::fade_color_by_intermedia(*i_other, rgb_imd, fade_table);
				}
			}
			else
			{
				for(;i != end; ++i, ++i_other)
				{
					i->u.color = col;
					i_other->u.color = col;
				}
			}
		}
		else
		{
			if(ybeg == r.y)
			{
				nana::pixel_rgb_t * p_rgb = sp->raw_pixel_buffer + ybeg * sp->pixel_size.width;
				nana::pixel_rgb_t * i = p_rgb;
				nana::pixel_rgb_t * end = p_rgb + xend;
				if(fade)
				{
					for(; i != end; ++i)
						*i = detail::fade_color_by_intermedia(*i, rgb_imd, fade_table);
				}
				else
				{
					for(;i != end; ++i)
						i->u.color = col;
				}
			}

			if(r.y + static_cast<int>(r.height) == yend)
			{
				nana::pixel_rgb_t * p_rgb = sp->raw_pixel_buffer + (yend - 1) * sp->pixel_size.width;
				nana::pixel_rgb_t * i = p_rgb;
				nana::pixel_rgb_t * end = p_rgb + xend;

				if(fade)
				{
					for(; i != end; ++i)
						*i = detail::fade_color_by_intermedia(*i, rgb_imd, fade_table);
				}
				else
				{
					for(;i != end; ++i)
						i->u.color = col;
				}
			}
		}

		if((xbeg == r.x) && (r.x + static_cast<int>(r.width) == xend))
		{
			nana::pixel_rgb_t * p_rgb = sp->raw_pixel_buffer + ybeg * sp->pixel_size.width;
			nana::pixel_rgb_t * i = p_rgb + xbeg;
			nana::pixel_rgb_t * end = sp->raw_pixel_buffer + (yend - 1) * sp->pixel_size.width + xbeg;

			nana::pixel_rgb_t * i_other = p_rgb + (xend - 1);

			if(fade)
			{
				while(true)
				{
					*i = detail::fade_color_by_intermedia(*i, rgb_imd, fade_table);
					*i_other = detail::fade_color_by_intermedia(*i_other, rgb_imd, fade_table);
					if(i == end)
						break;

					i += sp->pixel_size.width;
					i_other += sp->pixel_size.width;
				}
			}
			else
			{
				while(true)
				{
					i->u.color = col;
					i_other->u.color = col;
					if(i == end)
						break;

					i += sp->pixel_size.width;
					i_other += sp->pixel_size.width;
				}
			}
		}
		else
		{
			if(xbeg == r.x)
			{
				nana::pixel_rgb_t * p_rgb = sp->raw_pixel_buffer + ybeg * sp->pixel_size.width;
				nana::pixel_rgb_t * i = p_rgb + xbeg;
				nana::pixel_rgb_t * end = sp->raw_pixel_buffer + (yend - 1) * sp->pixel_size.width + xbeg;
				if(fade)
				{
					while(true)
					{
						*i = detail::fade_color_by_intermedia(*i, rgb_imd, fade_table);
						if(i == end)	break;

						i += sp->pixel_size.width;
					}
				}
				else
				{
					while(true)
					{
						i->u.color = col;
						if(i == end)	break;

						i += sp->pixel_size.width;
					}
				}
			}

			if(r.x + static_cast<int>(r.width) == xend)
			{
				nana::pixel_rgb_t * p_rgb = sp->raw_pixel_buffer + ybeg * sp->pixel_size.width;
				nana::pixel_rgb_t * i = p_rgb + (xend - 1);
				nana::pixel_rgb_t * end = sp->raw_pixel_buffer + (yend - 1) * sp->pixel_size.width + (xend - 1);
				if(fade)
				{
					while(true)
					{
						*i = detail::fade_color_by_intermedia(*i, rgb_imd, fade_table);
						if(i == end)	break;
						i += sp->pixel_size.width;
					}
				}
				else
				{
					while(true)
					{
						i->u.color = col;
						if(i == end)	break;
						i += sp->pixel_size.width;
					}
				}
			}
		}

		detail::free_fade_table(fade_table);
	}

	//stretch
	void pixel_buffer::stretch(const std::string& name)
	{
		pixel_buffer_storage * sp = storage_ref_.handle();
		if(sp)
			sp->img_pro.stretch = (name.size() ? detail::image_process_provider::instance().ref_stretch(name) : 0);
	}

	void pixel_buffer::stretch(const nana::rectangle& src_r, drawable_type drawable, const nana::rectangle& r) const
	{
		pixel_buffer_storage * sp = storage_ref_.handle();
		if(sp == 0) return;

		nana::rectangle good_src_r, good_dst_r;
		if(nana::gui::overlap(src_r, sp->pixel_size, r, paint::detail::drawable_size(drawable), good_src_r, good_dst_r) == false)
			return;

		(sp->img_pro.stretch ? sp->img_pro.stretch : detail::image_process_provider::instance().stretch())
			->process(*this, good_src_r, drawable, good_dst_r);
	}

	//blend
	void pixel_buffer::blend(const std::string& name)
	{
		pixel_buffer_storage * sp = storage_ref_.handle();
		if(sp)
			sp->img_pro.blend = (name.size() ? detail::image_process_provider::instance().ref_blend(name) : 0);
	}

	void pixel_buffer::blend(const nana::point& s_pos, drawable_type dw_dst, const nana::rectangle& r_dst, double fade_rate) const
	{
		pixel_buffer_storage * sp = storage_ref_.handle();
		if(sp == 0) return;

		nana::rectangle s_r(s_pos.x, s_pos.y, r_dst.width, r_dst.height);

		nana::rectangle s_good_r, d_good_r;
		if(nana::gui::overlap(s_r, sp->pixel_size, r_dst, paint::detail::drawable_size(dw_dst), s_good_r, d_good_r) == false)
			return;

		(sp->img_pro.blend ? sp->img_pro.blend : detail::image_process_provider::instance().blend())
			->process(dw_dst, d_good_r, *this, nana::point(s_good_r.x, s_good_r.y), fade_rate);
	}
}//end namespace paint
}//end namespace nana
