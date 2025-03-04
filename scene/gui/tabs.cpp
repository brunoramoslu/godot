/*************************************************************************/
/*  tabs.cpp                                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2015 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#include "tabs.h"

#include "message_queue.h"

Size2 Tabs::get_minimum_size() const {


	Ref<StyleBox> tab_bg = get_stylebox("tab_bg");
	Ref<StyleBox> tab_fg = get_stylebox("tab_fg");
	Ref<Font> font = get_font("font");

	Size2 ms(0, MAX( tab_bg->get_minimum_size().height,tab_fg->get_minimum_size().height)+font->get_height() );

//	h+=MIN( get_constant("label_valign_fg"), get_constant("label_valign_bg") );

	for(int i=0;i<tabs.size();i++) {

		Ref<Texture> tex = tabs[i].icon;
		if (tex.is_valid()) {
			ms.height = MAX( ms.height, tex->get_size().height );
			if (tabs[i].text!="")
				ms.width+=get_constant("hseparation");

		}
		ms.width+=font->get_string_size(tabs[i].text).width;
		if (current==i)
			ms.width+=tab_fg->get_minimum_size().width;
		else
			ms.width+=tab_bg->get_minimum_size().width;


	}

	return ms;
}



void Tabs::_input_event(const InputEvent& p_event) {

	if (p_event.type==InputEvent::MOUSE_BUTTON &&
	    p_event.mouse_button.pressed &&
	    p_event.mouse_button.button_index==BUTTON_LEFT) {

		// clicks
		Point2 pos( p_event.mouse_button.x, p_event.mouse_button.y );

		int found=-1;
		for(int i=0;i<tabs.size();i++) {

			int ofs=tabs[i].ofs_cache;
			int size = tabs[i].ofs_cache;
			if (pos.x >=tabs[i].ofs_cache && pos.x<tabs[i].ofs_cache+tabs[i].size_cache) {

				found=i;
				break;
			}
		}


		if (found!=-1) {

			set_current_tab(found);
			emit_signal("tab_changed",found);
		}
	}

}

void Tabs::_notification(int p_what) {


	switch(p_what) {


		case NOTIFICATION_DRAW: {

			RID ci = get_canvas_item();

			Ref<StyleBox> tab_bg = get_stylebox("tab_bg");
			Ref<StyleBox> tab_fg = get_stylebox("tab_fg");
			Ref<Font> font = get_font("font");
			Color color_fg = get_color("font_color_fg");
			Color color_bg = get_color("font_color_bg");

			int h = get_size().height;

			int label_valign_fg = get_constant("label_valign_fg");
			int label_valign_bg = get_constant("label_valign_bg");

			int w=0;

			int mw = get_minimum_size().width;

			if (tab_align==ALIGN_CENTER) {
				w=(get_size().width-mw)/2;
			} else if (tab_align==ALIGN_RIGHT) {
				w=get_size().width-mw;

			}

			if (w<0) {
				w=0;
			}

			for(int i=0;i<tabs.size();i++) {

				tabs[i].ofs_cache=w;

				String s = tabs[i].text;
				int lsize=0;
				int slen=font->get_string_size(s).width;;
				lsize+=slen;

				Ref<Texture> icon;
				if (tabs[i].icon.is_valid()) {
					icon = tabs[i].icon;
					if (icon.is_valid()) {
						lsize+=icon->get_width();
						if (s!="")
							lsize+=get_constant("hseparation");

					}
				}


				Ref<StyleBox> sb;
				int va;
				Color col;

				if (i==current) {

					sb=tab_fg;
					va=label_valign_fg;
					col=color_fg;
				} else {
					sb=tab_bg;
					va=label_valign_bg;
					col=color_bg;
				}


				Size2i sb_ms = sb->get_minimum_size();
				Rect2 sb_rect = Rect2( w, 0, lsize+sb_ms.width, h);
				sb->draw(ci, sb_rect );

				w+=sb->get_margin(MARGIN_LEFT);

				if (icon.is_valid()) {

					icon->draw(ci, Point2i( w, sb->get_margin(MARGIN_TOP)+((sb_rect.size.y-sb_ms.y)-icon->get_height())/2 ) );
					if (s!="")
						w+=icon->get_width()+get_constant("hseparation");

				}

				font->draw(ci, Point2i( w, sb->get_margin(MARGIN_TOP)+((sb_rect.size.y-sb_ms.y)-font->get_height())/2+font->get_ascent() ), s, col );

				w+=slen+sb->get_margin(MARGIN_RIGHT);

				tabs[i].size_cache=w-tabs[i].ofs_cache;

			}


		} break;
	}
}

int Tabs::get_tab_count() const {


	return tabs.size();
}


void Tabs::set_current_tab(int p_current) {

	ERR_FAIL_INDEX( p_current, get_tab_count() );

    //printf("DEBUG %p: set_current_tab to %i\n", this, p_current);
	current=p_current;	

	_change_notify("current_tab");
	//emit_signal("tab_changed",current);
	update();
}

int Tabs::get_current_tab() const {

	return current;
}


void Tabs::set_tab_title(int p_tab,const String& p_title) {

	ERR_FAIL_INDEX(p_tab,tabs.size());
	tabs[p_tab].text=p_title;
	update();
	minimum_size_changed();

}

String Tabs::get_tab_title(int p_tab) const{

	ERR_FAIL_INDEX_V(p_tab,tabs.size(),"");
	return tabs[p_tab].text;


}

void Tabs::set_tab_icon(int p_tab,const Ref<Texture>& p_icon){

	ERR_FAIL_INDEX(p_tab,tabs.size());
	tabs[p_tab].icon=p_icon;
	update();
	minimum_size_changed();

}
Ref<Texture> Tabs::get_tab_icon(int p_tab) const{

	ERR_FAIL_INDEX_V(p_tab,tabs.size(),Ref<Texture>());
	return tabs[p_tab].icon;

}

void Tabs::add_tab(const String& p_str,const Ref<Texture>& p_icon) {

	Tab t;
	t.text=p_str;
	t.icon=p_icon;
	tabs.push_back(t);

	update();
	minimum_size_changed();

}

void Tabs::clear_tabs() {
	tabs.clear();
	current=0;
	update();
}

void Tabs::remove_tab(int p_idx) {

	ERR_FAIL_INDEX(p_idx,tabs.size());
	tabs.remove(p_idx);
	if (current>=p_idx)
		current--;
	update();
	minimum_size_changed();

	if (current<0)
		current=0;
	if (current>=tabs.size())
		current=tabs.size()-1;

	//emit_signal("tab_changed",current);

}

void Tabs::set_tab_align(TabAlign p_align) {

	tab_align=p_align;
	update();
}

Tabs::TabAlign Tabs::get_tab_align() const {

	return tab_align;
}


void Tabs::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("_input_event"),&Tabs::_input_event);
	ObjectTypeDB::bind_method(_MD("get_tab_count"),&Tabs::get_tab_count);
	ObjectTypeDB::bind_method(_MD("set_current_tab","tab_idx"),&Tabs::set_current_tab);
	ObjectTypeDB::bind_method(_MD("get_current_tab"),&Tabs::get_current_tab);
	ObjectTypeDB::bind_method(_MD("set_tab_title","tab_idx","title"),&Tabs::set_tab_title);
	ObjectTypeDB::bind_method(_MD("get_tab_title","tab_idx"),&Tabs::get_tab_title);
	ObjectTypeDB::bind_method(_MD("set_tab_icon","tab_idx","icon:Texture"),&Tabs::set_tab_icon);
	ObjectTypeDB::bind_method(_MD("get_tab_icon:Texture","tab_idx"),&Tabs::get_tab_icon);
	ObjectTypeDB::bind_method(_MD("remove_tab","tab_idx"),&Tabs::remove_tab);
	ObjectTypeDB::bind_method(_MD("add_tab","title","icon:Texture"),&Tabs::add_tab);
	ObjectTypeDB::bind_method(_MD("set_tab_align","align"),&Tabs::set_tab_align);
	ObjectTypeDB::bind_method(_MD("get_tab_align"),&Tabs::get_tab_align);

	ADD_SIGNAL(MethodInfo("tab_changed",PropertyInfo(Variant::INT,"tab")));

	ADD_PROPERTY( PropertyInfo(Variant::INT, "current_tab", PROPERTY_HINT_RANGE,"-1,4096,1",PROPERTY_USAGE_EDITOR), _SCS("set_current_tab"), _SCS("get_current_tab") );

	BIND_CONSTANT( ALIGN_LEFT );
	BIND_CONSTANT( ALIGN_CENTER );
	BIND_CONSTANT( ALIGN_RIGHT );
}

Tabs::Tabs() {

	current=0;
	tab_align=ALIGN_CENTER;

}
