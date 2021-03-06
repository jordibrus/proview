/* 
 * Proview   Open Source Process Control.
 * Copyright (C) 2005-2013 SSAB EMEA AB.
 *
 * This file is part of Proview.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with Proview. If not, see <http://www.gnu.org/licenses/>
 *
 * Linking Proview statically or dynamically with other modules is
 * making a combined work based on Proview. Thus, the terms and 
 * conditions of the GNU General Public License cover the whole 
 * combination.
 *
 * In addition, as a special exception, the copyright holders of
 * Proview give you permission to, from the build function in the
 * Proview Configurator, combine Proview with modules generated by the
 * Proview PLC Editor to a PLC program, regardless of the license
 * terms of these modules. You may copy and distribute the resulting
 * combined work under the terms of your choice, provided that every 
 * copy of the combined work is accompanied by a complete copy of 
 * the source code of Proview (the version used to produce the 
 * combined work), being distributed under the terms of the GNU 
 * General Public License plus this exception.
 */

package jpwr.jopg;
import jpwr.rt.*;
import java.io.*;
import java.util.*;

public class GrowAxis extends GrowRect {

    int text_size;
    int text_drawtype;
    int text_color_drawtype;
    double max_value;
    double min_value;
    int lines;
    int linelength;
    int longquotient;
    int valuequotient;
    double increment;
    String format;    
    Object userdata;
    GlowCFormat	cFormat;
    StringBuffer sb = new StringBuffer();

    public GrowAxis(GrowCmn cmn) {
	super(cmn);
	configure();
    }

    public int type() {
	return Glow.eObjectType_GrowAxis;
    }

    void configure() {
	if ( lines <= 1)
	    lines = 2;
	if ( longquotient <= 0)
	    longquotient = 1;
	if ( valuequotient <= 0)
	    valuequotient = 1;
	increment = (max_value - min_value) / (lines - 1);
    }

    public void open(BufferedReader reader) {
	String line;
	StringTokenizer token;
	boolean end_found = false;

	try {
	    while( (line = reader.readLine()) != null) {
		token = new StringTokenizer(line);
		int key = Integer.valueOf(token.nextToken());
		if ( cmn.debug) System.out.println( "GrowAxis : " + line);

		switch ( key) {

		case Glow.eSave_GrowAxis: 
		    break;
		case Glow.eSave_GrowAxis_text_size: 
		    text_size = Integer.valueOf(token.nextToken()); 
		    break;
		case Glow.eSave_GrowAxis_text_drawtype: 
		    text_drawtype = Integer.valueOf(token.nextToken()); 
		    break;
		case Glow.eSave_GrowAxis_text_color_drawtype: 
		    text_color_drawtype = Integer.valueOf(token.nextToken()); 
		    break;
		case Glow.eSave_GrowAxis_max_value: 
		    max_value = new Double(token.nextToken()).doubleValue(); 
		    break;
		case Glow.eSave_GrowAxis_min_value: 
		    min_value = new Double(token.nextToken()).doubleValue(); 
		    break;
		case Glow.eSave_GrowAxis_lines: 
		    lines = Integer.valueOf(token.nextToken()); 
		    break;
		case Glow.eSave_GrowAxis_longquotient: 
		    longquotient = Integer.valueOf(token.nextToken()); 
		    break;
		case Glow.eSave_GrowAxis_valuequotient: 
		    valuequotient = Integer.valueOf(token.nextToken()); 
		    break;
		case Glow.eSave_GrowAxis_format:
		    if ( token.hasMoreTokens())
			format = token.nextToken();			 
		    break;
		case Glow.eSave_GrowAxis_rect_part: 
		    super.open( reader);
		    break;
		case Glow.eSave_GrowAxis_userdata_cb:
		    if ( cmn.appl != null)
			userdata = cmn.appl.growUserdataOpen( reader, this, Glow.eUserdataCbType_Node);
		    break;
		case Glow.eSave_End:
		    end_found = true;
		    break;
		default:
		    System.out.println( "Syntax error in GrowAxis");
		    break;
		}
		if ( end_found)
		    break;
	    }

	    configure();
	    if ( format != null)
		cFormat = new GlowCFormat(format);
		
	} catch ( Exception e) {
	    System.out.println( "IOException GrowAxis");
	}
    }    

    public void draw(GlowTransform t, int highlight, int hot, Object node, Object colornode) {
	if ( cmn.nodraw != 0)
	    return;
	int i;
	boolean draw_text = (Math.abs(increment) > Double.MIN_VALUE);
	int idx;
	int x, y;
	String text;
	int line_length;
	int x_text, y_text;
	int z_height = 0, z_width, z_descent = 0;
	int max_z_width = 0;
	double rotation;
	int drawtype;
	int text_idx = (int)( cmn.mw.zoom_factor_y / cmn.mw.base_zoom_factor * (text_size +4) - 4);
	double tsize = cmn.mw.zoom_factor_y / cmn.mw.base_zoom_factor * (8+2*text_size);
	text_idx = Math.min( text_idx, Glow.DRAW_TYPE_SIZE-1);

	if ( node != null && ((GrowNode)node).line_width != 0)
	    idx = (int)( cmn.mw.zoom_factor_y / cmn.mw.base_zoom_factor * 
			 ((GrowNode)node).line_width - 1);
	else
	    idx = (int)( cmn.mw.zoom_factor_y / cmn.mw.base_zoom_factor * line_width - 1);
	idx += hot;
	
	idx = Math.max( 0, idx);
	idx = Math.min( idx, Glow.DRAW_TYPE_SIZE-1);
	int x1, y1, x2, y2, ll_x, ll_y, ur_x, ur_y;

	if (t == null) {
	    x1 = (int)( trf.x( ll.x, ll.y) * cmn.mw.zoom_factor_x) - cmn.mw.offset_x;
	    y1 = (int)( trf.y( ll.x, ll.y) * cmn.mw.zoom_factor_y) - cmn.mw.offset_y;
	    x2 = (int)( trf.x( ur.x, ur.y) * cmn.mw.zoom_factor_x) - cmn.mw.offset_x;
	    y2 = (int)( trf.y( ur.x, ur.y) * cmn.mw.zoom_factor_y) - cmn.mw.offset_y;
	    rotation = (trf.rot() / 360 - Math.floor( trf.rot() / 360)) * 360;
	}
	else {
	    x1 = (int)( trf.x( t, ll.x, ll.y) * cmn.mw.zoom_factor_x) - cmn.mw.offset_x;
	    y1 = (int)( trf.y( t, ll.x, ll.y) * cmn.mw.zoom_factor_y) - cmn.mw.offset_y;
	    x2 = (int)( trf.x( t, ur.x, ur.y) * cmn.mw.zoom_factor_x) - cmn.mw.offset_x;
	    y2 = (int)( trf.y( t, ur.x, ur.y) * cmn.mw.zoom_factor_y) - cmn.mw.offset_y;
	    rotation = (trf.rot( t) / 360 - Math.floor( trf.rot( t) / 360)) * 360;
	}

	ll_x = Math.min( x1, x2);
	ur_x = Math.max( x1, x2);
	ll_y = Math.min( y1, y2);
	ur_y = Math.max( y1, y2);
	drawtype = GlowColor.get_drawtype( draw_type, Glow.eDrawType_LineHighlight,
					   highlight, (GrowNode)colornode, 0, 0);

	if ( 45 >= rotation || rotation > 315) {
	    // Vertical line to the right and values to the left

	    cmn.gdraw.line( ur_x, ll_y, ur_x, ur_y, drawtype, idx, 0);

	    // Calculate max value text width
	    if ( draw_text) {
		for ( i = 0; i < lines; i++) {
		    if ( i % valuequotient == 0) {
			text = format_text( format, max_value - i * increment);
			GlowDimension d = cmn.gdraw.getTextExtent( text,
						   Math.max( 0, text_idx), Glow.eFont_Helvetica,
						   text_drawtype);
			z_width = d.width;
			z_height = d.height;
			z_descent = z_height/4;
			if ( max_z_width < z_width)
			    max_z_width = z_width;
		    }
		}
		x_text = ll_x + max_z_width;
		line_length = ur_x - ll_x - max_z_width;
		if ( line_length < 3)
		    line_length = 3;
	    }
	    else {
		x_text = ll_x;
		line_length = ur_x - ll_x;
	    }

	    for ( i = 0; i < lines; i++) {
		y = (int)( ll_y + (double)(ur_y - ll_y) / (lines - 1) * i);
		if ( i % longquotient == 0)
		    cmn.gdraw.line( ur_x - line_length, y, 
				      ur_x, y, drawtype, idx, 0);
		else
		    cmn.gdraw.line( ur_x -  (int)( 2.0 / 3 * line_length), y, 
				      ur_x, y, drawtype, idx, 0);
		if ( draw_text) {
		    text = format_text( format, max_value - i * increment);

		    if ( text_idx >= 0 && max_z_width < ur_x - ll_x &&
			 i % valuequotient == 0) {
			if ( i == lines - 1)
			    y_text = y;
			else if ( i == 0)
			    y_text = y + z_height - z_descent - 3;
			else
			    y_text = y + (z_height-z_descent)/2;
			cmn.gdraw.text( ll_x, y_text,
					  text, text_drawtype, text_color_drawtype, 
					  text_idx, highlight, 0, Glow.eFont_Helvetica, tsize, 0);
		    }
		}
	    }
	}
	else if ( 45 < rotation && rotation <= 135)   {
	    // Horizontal line at bottom and values to the top

	    cmn.gdraw.line( ll_x, ur_y, ur_x, ur_y, drawtype, idx, 0);

	    // Calculate max value text height
	    if ( draw_text) {
		GlowDimension d = cmn.gdraw.getTextExtent( "0",
				     Math.max( 0, text_idx), Glow.eFont_Helvetica, text_drawtype);

		z_width = d.width;
		z_height = d.height;
		z_descent = z_height/4;
		line_length = ur_y - ll_y - z_height;
		if ( line_length < 3)
		    line_length = 3;
	    }
	    else {
		line_length = ur_y - ll_y;
	    }

	    for ( i = 0; i < lines; i++) {
		x = (int)( ll_x + (double)(ur_x - ll_x) / (lines - 1) * (lines - 1- i));
		if ( i % longquotient == 0)
		    cmn.gdraw.line( x, ur_y - line_length, x, 
				      ur_y, drawtype, idx, 0);
		else
		    cmn.gdraw.line( x, ur_y -  (int)( 2.0 / 3 * line_length), x, 
				      ur_y, drawtype, idx, 0);

		if ( draw_text && i % valuequotient == 0) {
		    text = format_text( format, max_value - i * increment);
		    GlowDimension d = cmn.gdraw.getTextExtent( text, 
			       Math.max( 0, text_idx), Glow.eFont_Helvetica, text_drawtype);
		    z_width = d.width;
		    z_height = d.height;
		    z_descent = z_height/4;

		    if ( text_idx >= 0 && z_height < ur_y - ll_y ) {
			if ( i == lines - 1)
			    x_text = x;
			else if ( i == 0)
			    x_text = x - z_width;
			else
			    x_text = x - (z_width)/2;
			cmn.gdraw.text( x_text, ll_y + z_height - z_descent,
					  text, text_drawtype, text_color_drawtype, 
					  text_idx, highlight, 0, Glow.eFont_Helvetica, tsize, 0);
		    }
		}
	    }
	}
	else if ( 135 < rotation && rotation <= 225) {
	    // Vertical line to the left and values to the right

	    cmn.gdraw.line( ll_x, ll_y, ll_x, ur_y, drawtype, idx, 0);

	    // Calculate max value text width
	    if ( draw_text) {
		for ( i = 0; i < lines; i++) {
		    if ( i % valuequotient == 0) {
			text = format_text( format, max_value - i * increment);
			GlowDimension d = cmn.gdraw.getTextExtent( text, 
								   Math.max( 0, text_idx), Glow.eFont_Helvetica, text_drawtype);
			z_width = d.width;
			z_height = d.height;
			z_descent = z_height/4;
			if ( max_z_width < z_width)
			    max_z_width = z_width;
		    }
		}
		x_text = ur_x - max_z_width;
		line_length = ur_x - ll_x - max_z_width;
		if ( line_length < 3)
		    line_length = 3;
	    }
	    else {
		x_text = ur_x;
		line_length = ur_x - ll_x;
	    }

	    for ( i = 0; i < lines; i++) {
		y = (int)( ll_y + (double)(ur_y - ll_y) / (lines - 1) * ( lines - 1 - i));
		if ( i % longquotient == 0)
		    cmn.gdraw.line( ll_x, y, 
				      ll_x + line_length, y, drawtype, idx, 0);
		else
		    cmn.gdraw.line( ll_x, y, 
				      ll_x + (int)( 2.0 / 3 * line_length), y, drawtype, idx, 0);
		text = format_text( format, max_value - i * increment);

		if ( draw_text && 
		     text_idx >= 0 && max_z_width < ur_x - ll_x &&
		     i % valuequotient == 0) {
		    if ( i == lines - 1)
			y_text = y + z_height - z_descent - 3;
		    else if ( i == 0)
			y_text = y;
		    else
			y_text = y + (z_height-z_descent)/2;
		    cmn.gdraw.text( x_text, y_text,
				      text, text_drawtype, text_color_drawtype, 
				      text_idx, highlight, 0, Glow.eFont_Helvetica, tsize, 0);
		}
	    }
	}
	else { // if ( 225 < rotation && rotation <= 315)
	    // Horizontal line at top and values at the bottom

	    cmn.gdraw.line( ll_x, ll_y, ur_x, ll_y, drawtype, idx, 0);

	    // Calculate max value text height
	    if ( draw_text) {
		GlowDimension d = cmn.gdraw.getTextExtent( "0", 
							   Math.max( 0, text_idx), Glow.eFont_Helvetica,
							   text_drawtype);

		z_width = d.width;
		z_height = d.height;
		z_descent = z_height/4;
		line_length = ur_y - ll_y - (z_height - z_descent);
		if ( line_length < 3)
		    line_length = 3;
	    }
	    else {
		line_length = ur_y - ll_y;
	    }

	    for ( i = 0; i < lines; i++) {
		x = (int)( ll_x + (double)(ur_x - ll_x) / (lines - 1) * i);
		if ( i % longquotient == 0)
		    cmn.gdraw.line( x, ll_y, x, 
				      ll_y + line_length, drawtype, idx, 0);
		else
		    cmn.gdraw.line( x, ll_y, x, 
				      ll_y  +  (int)( 2.0 / 3 * line_length), drawtype, idx, 0);
		if ( draw_text && i % valuequotient == 0) {
		    text = format_text( format, max_value - i * increment);
		    GlowDimension d = cmn.gdraw.getTextExtent( text, 
							       Math.max( 0, text_idx), Glow.eFont_Helvetica, text_drawtype);
		    z_width = d.width;
		    z_height = d.height;
		    z_descent = z_height/4;

		    if ( text_idx >= 0 && z_height - z_descent < ur_y - ll_y) {
			if ( i == lines - 1)
			    x_text = x - z_width;
			else if ( i == 0)
			    x_text = x;
			else
			    x_text = x - (z_width)/2;
			cmn.gdraw.text( x_text, ur_y,
					  text, text_drawtype, text_color_drawtype, 
					  text_idx, highlight, 0, Glow.eFont_Helvetica, tsize, 0);
		    }
		}
	    }
	}

    }

    String format_text( String fmt, double value) {
	/*
	if ( fmt.equals( "%1t")) {
	    // Hours, minutes and seconds, value in seconds
	    int val = (int) Math.nearbyint(value);
	    int hours = val / 3600;
	    int minutes = (val - hours * 3600) / 60; 
	    int seconds = (val - hours * 3600 - minutes * 60); 
	    sprintf( text, "%d:%02d:%02d", hours, minutes, seconds);
	}
	else if ( fmt.equals( "%2t")) {
	    // Hours and minutes, value in seconds
	    int val = (int) nearbyint(value);
	    int hours = val / 3600;
	    int minutes = (val - hours * 3600) / 60; 
	    sprintf( text, "%d:%02d", hours, minutes);
	}
	else if ( fmt.equals( "%3t")) {
	    // Days, hours and minues, value in seconds
	    int val = (int) nearbyint(value);
	    int days = val / (24 * 3600);
	    int hours = (val - days * 24 * 3600) / 3600; 
	    int minutes = (val - days * 24 * 3600 - hours * 3600) / 60; 
	    sprintf( text, "%d %02d:%02d", days, hours, minutes);
	}
	else if ( fmt.equals( "%10t")) {
	    // Date
	    char timstr[40];
	    pwr_tTime t;
	    t.tv_sec = (int) nearbyint(value);
	    t.tv_nsec = 0;
    
	    time_AtoAscii( &t, time_eFormat_NumDateAndTime, timstr, sizeof(timstr));
	    timstr[19] = 0;
	    strcpy( text, timstr);
	}
	else if ( fmt.equals( "%11t")) {
	    // Date, no seconds
	    char timstr[40];
	    pwr_tTime t;
	    t.tv_sec = (int) nearbyint(value);
	    t.tv_nsec = 0;
    
	    time_AtoAscii( &t, time_eFormat_NumDateAndTime, timstr, sizeof(timstr));
	    timstr[16] = 0;
	    strcpy( text, timstr);
	}
	else {
	*/
	    if ( Math.abs(value) < Double.MIN_VALUE)
		value = 0;
	    sb = cFormat.format( (float)value, sb);
	    return new String(sb);
	/*	    
	}
	*/
	    
    }

    public Object getUserData() {
	return userdata;
    }

}
