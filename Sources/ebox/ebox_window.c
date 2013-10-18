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

static const char* modifiers_list[] = {
    "",
    "Mod1-",
    "Mod2-",
    "Control-",
    "Shift-",
    
    "Mod1-Mod2-",
    "Mod1-Control-",
    "Mod1-Shift-",
    "Mod2-Control-",
    "Mod2-Shift-"
    "Control-Shift-",
    
    "Mod1-Mod2-Control-",
    "Mod1-Mod2-Shift-",
    "Mod2-Control-Shift-",

    "Mod1-Mod2-Shift-Control-",
};

void ebox_tk_ids(t_ebox *x, t_canvas *canvas)
{
    char buffer[MAXPDSTRING];
    x->e_canvas = canvas;
    
    sprintf(buffer,".x%lx.c", (long unsigned int) canvas);
    x->e_canvas_id = gensym(buffer);
    
    sprintf(buffer,"%s.canvas%lx", x->e_canvas_id->s_name, (long unsigned int)x);
    x->e_drawing_id = gensym(buffer);
    
    sprintf(buffer,"%s.frame%lx", x->e_canvas_id->s_name, (long unsigned int)x);
    x->e_frame_id = gensym(buffer);
    
    sprintf(buffer,"%s.window%lx", x->e_canvas_id->s_name, (long unsigned int)x);
    x->e_window_id = gensym(buffer);
    
    sprintf(buffer,"%s.handle%lx", x->e_canvas_id->s_name, (long unsigned int)x);
    x->e_handle_id = gensym(buffer);
    
    sprintf(buffer,"all%lx", (long unsigned int)x);
    x->e_all_id = gensym(buffer);
}

void ebox_bind_events(t_ebox* x)
{
    int i;
    
    // Mouse Enter //
    sys_vgui("bind %s <Enter> {pdsend {%s mouseenter}}\n", x->e_drawing_id->s_name, x->e_name_rcv->s_name);
    // Mouse Leave //
    sys_vgui("bind %s <Leave> {pdsend {%s mouseleave}}\n", x->e_drawing_id->s_name, x->e_name_rcv->s_name);
    
    // Right click //
    sys_vgui("bind %s <Button-2> {pdsend {%s rightclick %%x %%y}}\n", x->e_drawing_id->s_name, x->e_name_rcv->s_name);
    for(i = 0; i < 14; i++)
    {
        // Mouse Down //
        sys_vgui("bind %s <%sButtonPress> {pdsend {%s mousedown %%x %%y %i}}\n", x->e_drawing_id->s_name, modifiers_list[i], x->e_name_rcv->s_name, i);
        
        // Mouse Up //
        sys_vgui("bind %s <%sButtonRelease> {pdsend {%s mouseup %%x %%y %i}}\n", x->e_drawing_id->s_name, modifiers_list[i], x->e_name_rcv->s_name, i);
        
        // Mouse Move //
        sys_vgui("bind %s <%sMotion> {pdsend {%s mousemove %%x %%y %i}}\n", x->e_drawing_id->s_name, modifiers_list[i], x->e_name_rcv->s_name, i);
        
        // Mouse Wheel //
        sys_vgui("bind %s <%sMouseWheel> {pdsend {%s mousewheel %%D %i}}\n", x->e_drawing_id->s_name, modifiers_list[i], x->e_name_rcv->s_name, i);
    }
    /*
         sys_vgui("bind %s <$::modifier-Button> {pdtk_canvas_rightclick %s \
         [expr %%X - [winfo rootx %s]] [expr %%Y - [winfo rooty %s]] %%b}\n",
         x->e_drawing_id->s_name, x->e_canvas_id->s_name, x->e_canvas_id->s_name, x->e_canvas_id->s_name);
        
         sys_vgui("bind %s <KeyRelease> {+pdsend {%s keyup %%N}} \n",
         x->e_drawing_id->s_name, x->e_name_rcv->s_name);
          */
}

void ebox_create_widget(t_ebox* x)
{
    sys_vgui("namespace eval ebox%lx {} \n", x);
    sys_vgui("destroy %s \n", x->e_drawing_id->s_name);
    
    sys_vgui("canvas %s -width %d -height %d -bd 0 -state normal -takefocus 0 -highlightthickness 0  -insertborderwidth 0 -insertwidth 0 -confine 0\n",
             x->e_drawing_id->s_name,
             (int)(x->e_rect.width + x->e_boxparameters.d_borderthickness),
             (int)(x->e_rect.height + x->e_boxparameters.d_borderthickness));
    
    ebox_bind_events(x);    
}

void ebox_create_window(t_ebox* x, t_glist* glist)
{
    ebox_tk_ids(x, glist);
    ebox_create_widget(x);
    
    sys_vgui("%s create window %d %d -anchor nw -window %s -tags %s -width %d -height %d\n",
             x->e_canvas_id->s_name,
             (int)(x->e_obj.te_xpix - x->e_boxparameters.d_borderthickness),
             (int)(x->e_obj.te_ypix - x->e_boxparameters.d_borderthickness),
             x->e_drawing_id->s_name,
             x->e_window_id->s_name,
             (int)(x->e_rect.width + x->e_boxparameters.d_borderthickness * 2.),
             (int)(x->e_rect.height + x->e_boxparameters.d_borderthickness * 2.));

    sys_vgui("focus -force %s\n", x->e_canvas_id->s_name);
}


