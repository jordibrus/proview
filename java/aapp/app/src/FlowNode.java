/* 
 * Proview   Open Source Process Control.
 * Copyright (C) 2005-2012 SSAB EMEA AB.
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



package jpwr.app;
import jpwr.rt.*;
import java.io.*;
import java.util.*;


public class FlowNode implements FlowArrayElem {
  static final int OFFSET = 2;
  double x_right;
  double x_left;
  double y_high;
  double y_low;
  FlowCmn cmn;
  FlowNodeClass nc;
  FlowPoint pos;
  String n_name;
  String annotv[] = new String[10];
  int annotsize[] = new int[10];
  String trace_object;
  String trace_attribute;
  int trace_attr_type;
  boolean highlight;
  boolean select;

  public FlowNode( FlowCmn cmn) {
    this.cmn = cmn;
    pos = new FlowPoint( cmn);
  }

  @Override
  public int type() {
	 return Flow.eObjectType_Node;
  }

  public boolean getSelect() {
    return select;
  }
  public void setSelect( boolean select) {
    boolean redraw = (this.select != select);
    this.select = select;
    if ( redraw)
      draw();
  }

  public String getName() {
    return n_name;
  }
  public FlowCmn getCmn() {
	  return cmn;
  }
  public String getTraceObject() {
    return trace_object;
  }
  public FlowDimension measureNode() {
	  return new FlowDimension(x_left, y_low, x_right, y_high);  
  }
  public void setHighlight( boolean highlight) {
    boolean redraw = (this.highlight != highlight);
    this.highlight = highlight;
    if ( redraw)
      draw();
  }

  public void draw() {
	  // TODO
  }
  public void open( BufferedReader reader) {
    String line;
    StringTokenizer token;
    boolean end = false;
    boolean found = false;
    int i;

    try {
      while( (line = reader.readLine()) != null) {
	token = new StringTokenizer(line);
	int key = new Integer(token.nextToken()).intValue();
	if ( cmn.debug) System.out.println( "line : " + line);

	switch ( key) {
	case Flow.eSave_Node_nc:
	  String nc_name = token.nextToken();
	  found = false;
          for ( i = 0; i < cmn.a_nc.size(); i++) {
	    if ( ((FlowNodeClass)cmn.a_nc.get(i)).nc_name.equals( nc_name)) {
	      nc = (FlowNodeClass) cmn.a_nc.get(i);
	      found = true;
	      break;
	    }
	  }
	  if ( !found)
	    System.out.println( "FlowNode: NodeClass not found: " + nc_name);
	  break;
	case Flow.eSave_Node_n_name:
	  if ( token.hasMoreTokens())
	    n_name = token.nextToken();
          else
	    n_name = new String();
	  break;
	case Flow.eSave_Node_refcon_cnt:
	  for ( i = 0; i < 32; i++)
	    reader.readLine();
	  break;
	case Flow.eSave_Node_x_right:
	  x_right = new Double( token.nextToken()).doubleValue();
	  break;
	case Flow.eSave_Node_x_left:
	  x_left = new Double( token.nextToken()).doubleValue();
	  break;
	case Flow.eSave_Node_y_high:
	  y_high = new Double( token.nextToken()).doubleValue();
	  break;
	case Flow.eSave_Node_y_low:
	  y_low = new Double( token.nextToken()).doubleValue();
	  break;
	case Flow.eSave_Node_annotsize:
	  for ( i = 0; i < 10; i++) {
	    line = reader.readLine();
	    token = new StringTokenizer(line);
	    annotsize[i] = new Integer( token.nextToken()).intValue();
	  }
	  break;
	case Flow.eSave_Node_annotv:
	  // Annotation are surrouded by quotes. A quote inside a
	  // annotation is preceded by a backslash. The size is calculated
	  // without backslashes
	  for ( i = 0; i < 10; i++) {
	    if ( annotsize[i] > 0) {
	      StringBuffer buf = new StringBuffer();
	      char c_old = 0;
	      char c;
	      reader.read();
	      for ( int j = 0; j < annotsize[i]; j++) {
	    	  c = (char) reader.read();
	    	  if ( c == '"') {
	    		  if ( c_old == '\\') {
	    			  buf.setLength( buf.length() - 1);
	    			  j--;
	    		  }
	    		  else
	    			  break;
	    	  }
	    	  buf.append(c); // TODO convert to UTF-8
	    	  c_old = c;
	      }
	      annotv[i] = new String( buf);
	      reader.readLine();  // Read linefeed
	    }
	  }
	  break;
	case Flow.eSave_Node_pos:
	  pos.open( reader);
	  break;
	case Flow.eSave_Node_trace_object:
	  if ( token.hasMoreTokens())
	    trace_object = token.nextToken();
	  break;
	case Flow.eSave_Node_trace_attribute:
	  if ( token.hasMoreTokens())
	    trace_attribute = token.nextToken();
	  break;
	case Flow.eSave_Node_trace_attr_type:
	  trace_attr_type = new Integer( token.nextToken()).intValue();
	  break;
	case Flow.eSave_Node_obst_x_right:
	case Flow.eSave_Node_obst_x_left:
	case Flow.eSave_Node_obst_y_high:
	case Flow.eSave_Node_obst_y_low:
	case Flow.eSave_Node_trace_inverted:
	  break;
	case Flow.eSave_End:
	  end = true;
	  break;
	default:
	  System.out.println( "Syntax error in FlowNode");
	  break;
	}
	if ( end)
	  break;
      }
    } catch ( Exception e) {
      System.out.println( "IOException FlowNode");
    }
/*
    if ( nc.group == Flow.eNodeGroup_Common) {
      this.addMouseListener(new MouseAdapter() {
	    public void mouseReleased(MouseEvent e) {
	      if ( e.isPopupTrigger()) {
		new JopMethodsMenu( cmn.session, trace_object, JopUtility.TRACE, 
				    component, e.getX(), e.getY());
		return;
	      }
	    }
	    public void mousePressed(MouseEvent e) {
		System.out.println( "Mouse event" + n_name);
	      if ( e.isPopupTrigger()) {
		new JopMethodsMenu( cmn.session, trace_object, JopUtility.TRACE, 
				    component, e.getX(), e.getY());
		return;
	      }
	      if ( select) {
		setSelect( false);
	      }
	      else {
		cmn.unselect();
		setSelect( true);
	      }
	    }
	    public void mouseClicked(MouseEvent e) {
	      // Detect double click
	      if ( e.getClickCount() == 2) {
                if ( trace_object == null || trace_object.equals(""))
		  return;
                cmn.session.openCrrFrame( trace_object);
	      }
	    }
        });
    }
*/
  }

  public void draw( FlowPoint p0, String[] annotv0, boolean hl) {
	  if ( select) {
		  // Draw blue background
		  cmn.gdraw.rect(false, Plow.COLOR_LIGHTBLUE, (float)(x_left * cmn.zoom_factor - cmn.offset_x), 
				  (float)(y_low * cmn.zoom_factor - cmn.offset_y), 
				  (float)(x_right * cmn.zoom_factor - cmn.offset_x), 
				  (float)(y_high * cmn.zoom_factor - cmn.offset_y));
	  }

	  nc.draw( pos, annotv, highlight);
	  
  }

  public boolean eventHandler(PlowEvent e) {
	  System.out.println( "x(" + x_left + "," + x_right + ") (" + y_low + "," + y_high + ")");
	  switch ( e.type) {
	  case PlowEvent.TYPE_CLICK:
		  if (nc.group == Flow.eNodeGroup_Document)
			  return false;
		  if ( x_left <= e.x && e.x <= x_right &&
	  		 y_low <= e.y && e.y <= y_high) {
	  		cmn.currentNode = this;
	  		e.object = this;
	  		System.out.println("Hit !!");
	  		return true;
	  	}	
	  	break;
	  }
	  return false;
  }

  boolean attrFound;
  PwrtRefId subid;
  int refid;  
  boolean oldValue; 
  boolean firstScan;

  public Object dynamicGetRoot() {
    return null;
  }
  public void dynamicOpen() {
    if ( trace_object == null || trace_attribute == null ||
	   trace_object.equals(""))
      return;

    if ( trace_attr_type != Flow.eTraceType_Boolean)
      return;

    String attrName = trace_object + "." + trace_attribute;

    GdhrRefObjectInfo ret = cmn.gdh.refObjectInfo( attrName);
    if ( ret.evenSts())
      System.out.println( "ObjectInfoError " + attrName);
    else {
      attrFound = true;
      refid = ret.id;
      subid = ret.refid;

      if (trace_object.equals("H1-Dv1"))
          System.out.println("FlowNode sts: " + ret.sts + " refid:" + refid + " " + trace_object + "." + trace_attribute);
    }
  }

  public void dynamicClose() {
    if ( attrFound)
      cmn.gdh.unrefObjectInfo( subid);
  }

  public void dynamicUpdate( boolean animationOnly) {
    if ( attrFound) {
      boolean value = cmn.gdh.getObjectRefInfoBoolean( refid);

      if (trace_object.equals("H1-Dv1"))
          System.out.println("FlowNode value: " + value + " refid:" + refid + " " + trace_object + "." + trace_attribute);

      if ( value != oldValue || firstScan) {
    	  highlight = value;
    	  oldValue = value;

      }  
    }
  }

}






