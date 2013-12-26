/*
 * PdEnhanced - Pure Data Enhanced 
 *
 * An add-on for Pure Data
 *
 * Copyright (C) 2013 Pierre Guillot, CICM - Université Paris 8
 * All rights reserved.
 *
 * Website  : http://www.mshparisnord.fr/HoaLibrary/
 * Contacts : cicm.mshparisnord@gmail.com
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; either version 2 of the License.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "ebox.h"

static t_pt mouse_global_pos;
static t_pt mouse_patcher_pos;

static char *my_cursorlist[] = {
#ifdef _WINDOWS
	"right_ptr",
	"left_ptr",
#else
	"left_ptr",
    "center_ptr",
#endif
    "sb_v_double_arrow",
    "plus",
    "hand2",
    "circle",
    "X_cursor",
    "bottom_side",
    "bottom_right_corner",
    "right_side",
    "double_arrow",
    "exchange",
    "xterm"
};

//! The global mouse position method called by tcl/tk (PRIVATE)
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @param x_p       The abscissa of the mouse position
 \ @param y_p       The ordinate of the mouse position
 \ @return          Nothing
*/
void ebox_set_mouse_global_position(t_ebox* x, float x_p, float y_p)
{
    mouse_global_pos.x = x_p;
    mouse_global_pos.y = y_p;
}

//! The canvas mouse position method called by tcl/tk (PRIVATE)
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @param x_p       The abscissa of the mouse position
 \ @param y_p       The ordinate of the mouse position
 \ @return          Nothing
*/
void ebox_set_mouse_canvas_position(t_ebox* x, float x_p, float y_p)
{
    mouse_patcher_pos.x = x_p;
    mouse_patcher_pos.y = y_p;
}

//! Retrieve the global mouse position
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @return          This function return a point that contains the global abscissa and ordiante of the mouse
*/
t_pt ebox_get_mouse_global_position(t_ebox* x)
{
	t_eclass *c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;
	sys_vgui("ebox_%s_global_mousepos %s\n",c->c_class.c_name->s_name, x->b_object_id->s_name);
    //sys_vgui("global_mousepos %s\n", x->e_object_id->s_name);
    return mouse_global_pos;
}

//! Retrieve the canvas mouse position
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @return          This function return a point that contains abscissa and ordinate of the mouse in the ebox's canvas
*/
t_pt ebox_get_mouse_canvas_position(t_ebox* x)
{
    t_pt point;
	t_eclass *c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;
    //ebox_get_mouse_global_position(x);
    sys_vgui("ebox_%s_patcher_mousepos %s %s\n", c->c_class.c_name->s_name, x->b_object_id->s_name, x->b_canvas_id->s_name);
    //sys_vgui("patcher_mousepos %s %s\n", x->e_object_id->s_name, x->b_canvas_id->s_name);
    point.x = mouse_global_pos.x - mouse_patcher_pos.x;
    point.y = mouse_global_pos.y - mouse_patcher_pos.y;
    return point;
}

//! Change the cursor
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @param mode      The index of the cursor list
 \ @return          Nothing
*/
void ebox_set_cursor(t_ebox* x, int mode)
{
    mode = pd_clip_minmax(mode, 0, 12);
    sys_vgui("%s configure -cursor %s\n", x->b_drawing_id->s_name, my_cursorlist[mode]);
}

//! The mouse enter method called by tcl/tk (PRIVATE)
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @return          Nothing
*/
void ebox_mouse_enter(t_ebox* x)
{
    t_eclass *c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;
    
    if(!x->b_obj.o_canvas->gl_edit && !x->b_mouse_down)
    {
        sys_vgui("focus %s\n", x->b_drawing_id->s_name);
        if(c->c_widget.w_mouseenter)
            c->c_widget.w_mouseenter(x);
    }
}

//! The mouse leave method called by tcl/tk (PRIVATE)
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @return          Nothing
*/
void ebox_mouse_leave(t_ebox* x)
{
    t_eclass *c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;
    
    if(!x->b_obj.o_canvas->gl_edit && !x->b_mouse_down)
    {
        sys_vgui("focus %s\n", x->b_canvas_id->s_name);
        if(c->c_widget.w_mouseleave)
            c->c_widget.w_mouseleave(x);

        ebox_set_cursor(x, 0);
    }
    else if(x->b_obj.o_canvas->gl_edit && !x->b_mouse_down)
    {
        ebox_set_cursor(x, 4);
    }
}

//! The mouse move method called by tcl/tk (PRIVATE)
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @param s         The message selector
 \ @param argc      The size of the array of atoms
 \ @param argv      The array of atoms
 \ @return          Nothing
*/
void ebox_mouse_move(t_ebox* x, t_symbol* s, long argc, t_atom* argv)
{
    t_atom av[2];
    t_eclass *c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;
    ebox_get_mouse_global_position(x);
    
    x->b_modifiers = (long)atom_getfloat(argv+2);
#ifdef _WINDOWS
	
	if(x->b_modifiers >= 131080)
	{
		x->b_modifiers -= 131080;
		x->b_modifiers += EMOD_ALT;
	}
	else
		x->b_modifiers -= 8;
#endif

    if(!x->b_obj.o_canvas->gl_edit)
    {
        if(!x->b_mouse_down)
        {
            if(x->b_flags & EBOX_IGNORELOCKCLICK)
                ebox_set_cursor(x, 0);
            else
                ebox_set_cursor(x, 1);
            
            x->b_mouse.x = atom_getfloat(argv);
            x->b_mouse.y = atom_getfloat(argv+1);
            
            if(c->c_widget.w_mousemove)
                c->c_widget.w_mousemove(x, x->b_obj.o_canvas, x->b_mouse, x->b_modifiers);
        }
        else
        {
            ebox_mouse_drag(x, s, argc, argv);
        }
    }
    else
    {
        if(!x->b_mouse_down)
        {
            ebox_mouse_move_editmode(x, atom_getfloat(argv), atom_getfloat(argv+1), x->b_modifiers);
        }
        else
        {
            x->b_mouse.x = atom_getfloat(argv);
            x->b_mouse.y = atom_getfloat(argv+1);
            if(x->b_selected_item == EITEM_NONE)
            {
                x->b_move_box = ebox_get_mouse_canvas_position(x);
                sys_vgui("pdtk_canvas_motion %s %i %i 0\n", x->b_canvas_id->s_name, (int)x->b_move_box.x, (int)x->b_move_box.y);
            }
            else
            {
                if(x->b_flags & EBOX_GROWNO)
                    return;
                else if(x->b_flags & EBOX_GROWLINK)
                {
                    if(x->b_selected_item == EITEM_BOTTOM)
                    {
                        atom_setfloat(av, x->b_rect_last.width + (x->b_mouse.y - x->b_rect_last.height));
                        atom_setfloat(av+1, x->b_mouse.y);
                    }
                    else if(x->b_selected_item == EITEM_RIGHT)
                    {
                        atom_setfloat(av, x->b_mouse.x);
                        atom_setfloat(av+1, x->b_rect_last.height + (x->b_mouse.x - x->b_rect_last.width));
                    }
                    else if(x->b_selected_item == EITEM_CORNER)
                    {
                        if(x->b_mouse.y > x->b_mouse.x)
                        {
                            atom_setfloat(av, x->b_mouse.y);
                            atom_setfloat(av+1, x->b_mouse.y);
                        }
                        else
                        {
                            atom_setfloat(av, x->b_mouse.x);
                            atom_setfloat(av+1, x->b_mouse.x);
                        }
                    }
                }
                else if (x->b_flags & EBOX_GROWINDI)
                {
                    if(x->b_selected_item == EITEM_BOTTOM)
                    {
                        atom_setfloat(av, x->b_rect_last.width);
                        atom_setfloat(av+1, x->b_mouse.y);
                    }
                    else if(x->b_selected_item == EITEM_RIGHT)
                    {
                        atom_setfloat(av, x->b_mouse.x);
                        atom_setfloat(av+1, x->b_rect_last.height);
                    }
                    else if(x->b_selected_item == EITEM_CORNER)
                    {
                        atom_setfloat(av, x->b_mouse.x);
                        atom_setfloat(av+1, x->b_mouse.y);
                    }
                }
                mess3((t_pd *)x, gensym("size"),  gensym("size"), 2, av);
            }
        }
    }
}

//! The mouse drag method called via mouse move method when mouse down (PRIVATE)
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @param s         The message selector
 \ @param argc      The size of the array of atoms
 \ @param argv      The array of atoms
 \ @return          Nothing
*/
void ebox_mouse_drag(t_ebox* x, t_symbol* s, long argc, t_atom* argv)
{
    t_eclass *c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;
    
    x->b_modifiers -= 256;

    if(!x->b_obj.o_canvas->gl_edit)
    {
        x->b_mouse.x = atom_getfloat(argv);
        x->b_mouse.y = atom_getfloat(argv+1);
        if(c->c_widget.w_mousedrag && x->b_mouse_down)
            c->c_widget.w_mousedrag(x, x->b_obj.o_canvas, x->b_mouse, x->b_modifiers);
    }
}

//! The mouse down method called by tcl/tk (PRIVATE)
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @param s         The message selector
 \ @param argc      The size of the array of atoms
 \ @param argv      The array of atoms
 \ @return          Nothing
*/
void ebox_mouse_down(t_ebox* x, t_symbol* s, long argc, t_atom* argv)
{
    t_eclass *c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;
	
    x->b_modifiers = (long)atom_getfloat(argv+2);
#ifdef _WINDOWS
	
	if(x->b_modifiers >= 131080)
	{
		x->b_modifiers -= 131080;
		x->b_modifiers += EMOD_ALT;
	}
	else
		x->b_modifiers -= 8;
#endif

    if(!x->b_obj.o_canvas->gl_edit)
    {
        x->b_mouse.x = atom_getfloat(argv);
        x->b_mouse.y = atom_getfloat(argv+1);
        if(c->c_widget.w_mousedown)
            c->c_widget.w_mousedown(x, x->b_obj.o_canvas, x->b_mouse, x->b_modifiers);
    }
    else
    {
        x->b_mouse.x = atom_getfloat(argv);
        x->b_mouse.y = atom_getfloat(argv+1);
        if(x->b_selected_item == EITEM_NONE)
        {
            x->b_move_box = ebox_get_mouse_canvas_position(x);
            if(x->b_modifiers == EMOD_CMD)
                sys_vgui("pdtk_canvas_rightclick %s %i %i 0\n", x->b_canvas_id->s_name, (int)x->b_move_box.x, (int)x->b_move_box.y);
            else
                sys_vgui("pdtk_canvas_mouse %s %i %i 0 0\n", x->b_canvas_id->s_name, (int)(x->b_move_box.x), (int)(x->b_move_box.y));
        }
        else
        {
            x->b_rect_last = x->b_rect;
        }
    }
    x->b_mouse_down = 1;
}

//! The mouse up method called by tcl/tk (PRIVATE)
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @param s         The message selector
 \ @param argc      The size of the array of atoms
 \ @param argv      The array of atoms
 \ @return          Nothing
*/
void ebox_mouse_up(t_ebox* x, t_symbol* s, long argc, t_atom* argv)
{
    t_eclass *c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;
   
    x->b_modifiers = (long)atom_getfloat(argv+2);
#ifdef _WINDOWS
	
	if(x->b_modifiers >= 131080)
	{
		x->b_modifiers -= 131080;
		x->b_modifiers += EMOD_ALT;
	}
	else
		x->b_modifiers -= 8;
#endif    
    if(!x->b_obj.o_canvas->gl_edit)
    {
        x->b_mouse.x = atom_getfloat(argv);
        x->b_mouse.y = atom_getfloat(argv+1);
        if(c->c_widget.w_mouseup)
            c->c_widget.w_mouseup(x, x->b_obj.o_canvas, x->b_mouse, x->b_modifiers);
    }
    else
    {
        x->b_move_box = ebox_get_mouse_canvas_position(x);
        sys_vgui("pdtk_canvas_mouseup %s %i %i 0\n", x->b_canvas_id->s_name, (int)x->b_move_box.x, (int)x->b_move_box.y);
        if(x->b_selected_inlet != -1 || x->b_selected_outlet != -1)
        {
            x->b_selected_inlet = -1;
            x->b_selected_outlet = -1;
            ebox_redraw(x);
        }
    }
    
    x->b_mouse_down = 0;
}

//! The mouse double click method called by tcl/tk (PRIVATE)
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @param s         The message selector
 \ @param argc      The size of the array of atoms
 \ @param argv      The array of atoms
 \ @return          Nothing
*/
void ebox_mouse_dblclick(t_ebox* x, t_symbol* s, long argc, t_atom* argv)
{
    t_eclass *c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;
    
    x->b_modifiers = (long)atom_getfloat(argv+2);
#ifdef _WINDOWS
	
	if(x->b_modifiers >= 131080)
	{
		x->b_modifiers -= 131080;
		x->b_modifiers += EMOD_ALT;
	}
	else
		x->b_modifiers -= 8;
#endif    
    if(!x->b_obj.o_canvas->gl_edit)
    {
        x->b_mouse.x = atom_getfloat(argv);
        x->b_mouse.y = atom_getfloat(argv+1);
        if(c->c_widget.w_dblclick)
            c->c_widget.w_dblclick(x, x->b_obj.o_canvas, x->b_mouse);
    }
}

//! The mouse wheel method called by tcl/tk (PRIVATE)
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @param s         The message selector
 \ @param argc      The size of the array of atoms
 \ @param argv      The array of atoms
 \ @return          Nothing
*/
void ebox_mouse_wheel(t_ebox* x, t_symbol* s, long argc, t_atom* argv)
{
    float delta;
    t_eclass* c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;
    
    x->b_modifiers = (long)atom_getfloat(argv+3);
    delta = atom_getfloat(argv+2);

#ifdef _WINDOWS
	
	if(x->b_modifiers >= 131080)
	{
		x->b_modifiers -= 131080;
		x->b_modifiers += EMOD_ALT;
	}
	else
		x->b_modifiers -= 8;

	delta /= 120.;
#endif
    if(!x->b_obj.o_canvas->gl_edit)
    {
        x->b_mouse.x = atom_getfloat(argv);
        x->b_mouse.y = atom_getfloat(argv+1);
        if(c->c_widget.w_mousewheel)
            c->c_widget.w_mousewheel(x, x->b_obj.o_canvas, x->b_mouse, (long)x->b_modifiers, delta, delta);
    }
}

//! The mouse move editmode method called via mouse move method when mouse down (PRIVATE)
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @param s         The message selector
 \ @param argc      The size of the array of atoms
 \ @param argv      The array of atoms
 \ @return          Nothing
*/
void ebox_mouse_move_editmode(t_ebox* x, float x_p, float y_p, float key)
{
    int i;
    int right, bottom;
    
    x->b_selected_outlet    = -1;
    x->b_selected_inlet     = -1;
    x->b_selected_item      = EITEM_NONE;
    x->b_move_box = ebox_get_mouse_canvas_position(x);
    sys_vgui("pdtk_canvas_motion %s %i %i 0\n", x->b_canvas_id->s_name, (int)x->b_move_box.x, (int)x->b_move_box.y);
    
    right   = x->b_rect.width + x->b_boxparameters.d_borderthickness * 2.;
    bottom  = x->b_rect.height + x->b_boxparameters.d_borderthickness * 2.;
    
    // TOP //
    if(y_p >= 0 && y_p < 3)
    {
        for(i = 0; i < obj_noutlets((t_object *)x); i++)
        {
            int pos_x_inlet = 0;
            if(obj_ninlets((t_object *)x) != 1)
                pos_x_inlet = (int)(i / (float)(obj_ninlets((t_object *)x) - 1) * (x->b_rect.width - 8));
            
            if(x_p >= pos_x_inlet && x_p <= pos_x_inlet +7)
            {
                x->b_selected_inlet = i;
                ebox_set_cursor(x, 4);
                break;
            }
        }
        ebox_invalidate_layer(x, gensym("eboxio"));
        ebox_redraw(x);
        return;
    }
    // BOTTOM & RIGHT //
    else if(y_p > bottom - 3 && y_p <= bottom && x_p > right - 3 && x_p <= right)
    {
        x->b_selected_item = EITEM_CORNER;
        ebox_set_cursor(x, 8);
        return;
    }
    // BOTTOM //
    else if(y_p > bottom - 3 && y_p < bottom)
    {
        for(i = 0; i < obj_noutlets((t_object *)x); i++)
        {
            int pos_x_outlet = 0;
            if(obj_noutlets((t_object *)x) != 1)
                pos_x_outlet = (int)(i / (float)(obj_noutlets((t_object *)x) - 1) * (x->b_rect.width - 8));
            
            if(x_p >= pos_x_outlet && x_p <= pos_x_outlet +7)
            {
                x->b_selected_outlet = i;
                ebox_set_cursor(x, 5);
                break;
            }
        }
        if(x->b_selected_outlet == -1)
        {
            x->b_selected_item = EITEM_BOTTOM;
            ebox_set_cursor(x, 7);
        }
        ebox_invalidate_layer(x, gensym("eboxio"));
        ebox_redraw(x);
        return;
    }
    // RIGHT //
    else if(x_p > right - 3 && x_p <= right)
    {
        x->b_selected_item = EITEM_RIGHT;
        ebox_set_cursor(x, 9);
        return;
    }
    
    // BOX //
    ebox_set_cursor(x, 4);

    ebox_invalidate_layer(x, gensym("eboxio"));
    ebox_redraw(x);
}

//! The key down method called by tcl/tk (PRIVATE AND NOT READY)
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @param s         The message selector
 \ @param argc      The size of the array of atoms
 \ @param argv      The array of atoms
 \ @return          Nothing
 */
void ebox_key(t_ebox* x, t_symbol* s, long argc, t_atom* argv)
{
    t_eclass* c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;
   
    if(argc >= 2 && argv && atom_gettype(argv+1) == A_FLOAT)
    {
        if(!x->b_obj.o_canvas->gl_edit)
        {
            if(atom_getfloat(argv+1) == 65288)
            {
                if(c->c_widget.w_keyfilter)
                {
                    c->c_widget.w_keyfilter(x, NULL, EKEY_DEL, 0);
                }
                else if(c->c_widget.w_key)
                {
                    c->c_widget.w_key(x, NULL, EKEY_DEL, 0);
                }
            }
            else if(atom_getfloat(argv+1) == 65289)
            {
                if(c->c_widget.w_keyfilter)
                {
                    c->c_widget.w_keyfilter(x, NULL, EKEY_TAB, 0);
                }
                else if(c->c_widget.w_key)
                {
                    c->c_widget.w_key(x, NULL, EKEY_TAB, 0);
                }
            }
            else if(atom_getfloat(argv+1) == 65293)
            {
                if(c->c_widget.w_keyfilter)
                {
                    c->c_widget.w_keyfilter(x, NULL, EKEY_ENTER, 0);
                }
                else if(c->c_widget.w_key)
                {
                    c->c_widget.w_key(x, NULL, EKEY_ENTER, 0);
                }
            }
            else if(atom_getfloat(argv+1) == 65307)
            {
                if(c->c_widget.w_keyfilter)
                {
                    c->c_widget.w_keyfilter(x, NULL, EKEY_ESC, 0);
                }
                else if(c->c_widget.w_key)
                {
                    c->c_widget.w_key(x, NULL, EKEY_ESC, 0);
                }
            }
            else
            {
                if(c->c_widget.w_key)
                {
                    c->c_widget.w_key(x, NULL, (char)atom_getfloat(argv+1), 0);
                }
            }
        }
        else
        {
            ;
        }
    }
}

//! The key up method called by tcl/tk (PRIVATE AND NOT READY)
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @param s         The message selector
 \ @param argc      The size of the array of atoms
 \ @param argv      The array of atoms
 \ @return          Nothing
*/
void ebox_deserted(t_ebox *x)
{
    t_eclass* c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;

    if(c->c_widget.w_deserted && x->b_ready_to_draw)
        c->c_widget.w_deserted(x);
}

//! The popup method called by tcl/tk (PRIVATE)
/*
 \ @memberof        ebox
 \ @param x         The ebox pointer
 \ @param s         The message selector
 \ @param itemid    the id of the selected item
 \ @return          Nothing
*/
void ebox_popup(t_ebox* x, t_symbol* s, float itemid)
{
    t_eclass* c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;

    if(c->c_widget.w_popup)
        c->c_widget.w_popup(x, s, (long)itemid);
}

//! The default save method for UI ebox (PRIVATE)
/*
 \ @memberof        ebox
 \ @param z         The gobj
 \ @param b         The binbuf
 \ @return          Nothing
*/
void ebox_dosave_box(t_gobj* z, t_binbuf *b)
{
    t_ebox*   x = (t_ebox *)z;
    t_eclass* c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;
    
    binbuf_addv(b, "ssii", gensym("#X"), gensym("obj"), (t_int)x->b_obj.o_obj.te_xpix, (t_int)x->b_obj.o_obj.te_ypix);
    binbuf_add(b, binbuf_getnatom(x->b_obj.o_obj.te_binbuf), binbuf_getvec(x->b_obj.o_obj.te_binbuf));
    
    if(c->c_widget.w_save != NULL)
        c->c_widget.w_save(x, b);
    
    binbuf_addv(b, ";");
}

//! The default save method for none UI ebox (PRIVATE)
/*
 \ @memberof        ebox
 \ @param z         The gobj
 \ @param b         The binbuf
 \ @return          Nothing
*/
void ebox_dosave_nobox(t_gobj* z, t_binbuf *b)
{
    int         i;
    char        attr_name[MAXPDSTRING];
    long        argc    = 0;
    t_atom*     argv    = NULL;
    t_ebox*   x = (t_ebox *)z;
    t_eclass* c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;
    
    binbuf_addv(b, "ssiis", gensym("#X"), gensym("obj"), (int)x->b_obj.o_obj.te_xpix, (int)x->b_obj.o_obj.te_ypix, gensym(class_getname(x->b_obj.o_obj.te_g.g_pd)));
    
    for(i = 0; i < c->c_nattr; i++)
    {
        if(c->c_attr[i].save)
        {
            sprintf(attr_name, "@%s", c->c_attr[i].name->s_name);
            object_attr_getvalueof((t_object *)x, c->c_attr[i].name, &argc, &argv);
            if(argc && argv)
            {
                binbuf_append_attribute(b, gensym(attr_name), argc, argv);
                argc = 0;
                free(argv);
                argv = NULL;
            }
        }
    }

    if(c->c_widget.w_save != NULL)
        c->c_widget.w_save(x, b);
    
    binbuf_addv(b, ";");
}

//! The default save method for all ebox called by PD (PRIVATE)
/*
 \ @memberof        ebox
 \ @param z         The gobj
 \ @param b         The binbuf
 \ @return          Nothing
*/
void ebox_save(t_gobj* z, t_binbuf *b)
{
    t_ebox*   x = (t_ebox *)z;
    t_eclass* c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;
    if(c->c_box)
    {
        ebox_dosave_box(z, b);
    }
    else if(x->b_ready_to_draw)
    {
       ebox_dosave_nobox(z, b);
    }
}

//! The default user id method for all ebox called by PD (PRIVATE)
/*
 \ @memberof        ebox
 \ @param x         The gobj
 \ @param attr      Nothing (for Max 6 compatibility)
 \ @param argc      The size of the array of atoms
 \ @param argv      The array of atoms
 \ @return          Always 0 (for the moment)
*/
t_pd_err ebox_set_id(t_ebox *x, t_object *attr, long argc, t_atom *argv)
{
    if(argc && argv && atom_gettype(argv) == A_SYM)
    {
		if(x->b_objuser_id != gensym("(null)"))
			pd_unbind(&x->b_obj.o_obj.ob_pd, x->b_objuser_id);
		if(atom_getsym(argv) != gensym("(null)"))
		{
			x->b_objuser_id = atom_getsym(argv);
			pd_bind(&x->b_obj.o_obj.ob_pd, x->b_objuser_id);
		}
    }
    return 0;
}

void ebox_write(t_ebox* x, t_symbol* s, long argc, t_atom* argv)
{
    char buf[MAXPDSTRING];
    char* pch;
    t_atom av[1];
    t_eclass* c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;
    
    if(argc && argv && atom_gettype(argv) == A_SYM)
    {
        pch = strpbrk(atom_getsym(argv)->s_name, "/\"");
        if(pch != NULL)
        {
            atom_setsym(av, atom_getsym(argv));
            if(c->c_widget.w_write)
                c->c_widget.w_write(x, s, 1, av);
            return;
        }
        else
        {
            sprintf(buf, "%s/%s", canvas_getdir(x->b_obj.o_canvas)->s_name, atom_getsym(argv)->s_name);
            atom_setsym(av, gensym(buf));
            if(c->c_widget.w_write)
                c->c_widget.w_write(x, s, 1, av);
            return;
        }
    }
    sys_vgui("ebox%s_saveas %s nothing nothing\n", c->c_class.c_name->s_name, x->b_object_id->s_name);
}

void ebox_read(t_ebox* x, t_symbol* s, long argc, t_atom* argv)
{
    char buf[MAXPDSTRING];
    char* pch;
    t_atom av[1];
    t_eclass* c = (t_eclass *)x->b_obj.o_obj.te_g.g_pd;
    
    if(argc && argv && atom_gettype(argv) == A_SYM)
    {
        pch = strpbrk(atom_getsym(argv)->s_name, "/\"");
        if(pch != NULL)
        {
            atom_setsym(av, atom_getsym(argv));
            if(c->c_widget.w_read)
                c->c_widget.w_read(x, s, 1, av);
            return;
        }
        else
        {
            sprintf(buf, "%s/%s", canvas_getdir(x->b_obj.o_canvas)->s_name, atom_getsym(argv)->s_name);
            atom_setsym(av, gensym(buf));
            if(c->c_widget.w_read)
                c->c_widget.w_read(x, s, 1, av);
            return;
        }
    }
    sys_vgui("ebox%s_openfrom %s\n", c->c_class.c_name->s_name, x->b_object_id->s_name);
}









