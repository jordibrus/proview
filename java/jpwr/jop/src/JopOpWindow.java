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

package jpwr.jop;
import jpwr.rt.*;
import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

public class JopOpWindow extends JPanel {

  JopSession session;
  JopEngine en;
  Object root;
  JLabel label = null;
  String user;
  OpWindButton langButton;
  OpWindButton loginButton;
  OpWindButton logoutButton;
  OpWindButton alarmButton;
  OpWindButton navigatorButton;
  OpWindButton eventLogButton;
  OpWindButton helpButton;

  public JopOpWindow( JopSession session, Object root) {
    // super( BoxLayout.Y_AXIS);
    // createGlue();
    this.session = session;
    this.root = root;
    en = session.getEngine();

    // Get WebHandler object
    CdhrObjid oretWebH = en.gdh.getClassList( Pwrb.cClass_WebHandler);
    if ( oretWebH.evenSts()) return;

    CdhrString sret = en.gdh.objidToName( oretWebH.objid, Cdh.mName_volumeStrict);            
    if ( sret.evenSts()) return;

    // Set language
    JopLang lng = new JopLang(session);

    String s = sret.str + ".Language";
    CdhrInt iret = en.gdh.getObjectInfoInt( s);
    if ( iret.oddSts())
      lng.set( iret.value);

    JopLang.setDefault(lng);

    if ( root instanceof JFrame)
      ((JFrame)root).setTitle( JopLang.transl("Operator Window"));

    s = sret.str + ".Title";
    CdhrString srettxt = en.gdh.getObjectInfoString( s);
    if ( srettxt.evenSts()) return;

    JLabel llabel = new JLabel( srettxt.str);
    Font f = new Font("Helvetica", Font.BOLD, 24);
    llabel.setFont( f);
    this.add( llabel);

    s = sret.str + ".Text";
    srettxt = en.gdh.getObjectInfoString( s);
    if ( srettxt.evenSts()) return;

    llabel = new JLabel( srettxt.str);
    f = new Font("Helvetica", Font.BOLD, 16);
    llabel.setFont( f);
    this.add( llabel);

    this.add( new JSeparator());

    label = new JLabel();
    label.setHorizontalAlignment( SwingConstants.CENTER);
    this.add( label);

    OpWindButton button;

    s = sret.str + ".EnableLanguage";
    iret = en.gdh.getObjectInfoInt( s);
    if ( iret.evenSts()) return;

    if ( iret.value != 0) {
      langButton = new OpWindButton( session, "", JopLang.transl("Language"),
				     OpWindButton.LANGUAGE);
      this.add( langButton);
    }

    s = sret.str + ".EnableLogin";
    iret = en.gdh.getObjectInfoInt( s);
    if ( iret.evenSts()) return;

    if ( iret.value != 0) {

      loginButton = new OpWindButton( session, "", JopLang.transl("Login"),
				 OpWindButton.LOGIN);
      this.add( loginButton);

      logoutButton = new OpWindButton( session, "", JopLang.transl("Logout"),
				 OpWindButton.LOGOUT);
      this.add( logoutButton);
    }
      
    s = sret.str + ".EnableAlarmList";
    iret = en.gdh.getObjectInfoInt( s);
    if ( iret.evenSts()) return;

    if ( iret.value != 0 && !(root instanceof JFrame)) {
      alarmButton = new OpWindButton( session, "", JopLang.transl("Alarm and Event List"),
				 OpWindButton.ALARMLIST);
      this.add( alarmButton);
    }
      
    s = sret.str + ".EnableEventLog";
    iret = en.gdh.getObjectInfoInt( s);
    if ( iret.evenSts()) return;

    if ( iret.value != 0 && !(root instanceof JFrame)) {
      eventLogButton = new OpWindButton( session, "", JopLang.transl("Event Log"),
				 OpWindButton.EVENTLOG);
      this.add( eventLogButton);
    }
      
    s = sret.str + ".EnableNavigator";
    iret = en.gdh.getObjectInfoInt( s);
    if ( iret.evenSts()) return;

    if ( iret.value != 0) {
      navigatorButton = new OpWindButton( session, "", JopLang.transl("Navigator"),
				 OpWindButton.NAVIGATOR);
      this.add( navigatorButton);
    }
      
    s = sret.str + ".DisableHelp";
    iret = en.gdh.getObjectInfoInt( s);
    if ( iret.evenSts()) return;

    if ( iret.value != 0) {
      helpButton = new OpWindButton( session, "", JopLang.transl("Help"),
			       OpWindButton.HELP);
      this.add( helpButton);
    }

    s = sret.str + ".DisableProview";
    iret = en.gdh.getObjectInfoInt( s);
    if ( iret.evenSts()) return;

    if ( iret.value != 0) {
      button = new OpWindButton( session, "", "Proview",
				 OpWindButton.PROVIEW);
      this.add( button);
    }
    this.add( new JSeparator());

    CdhrString sretName = null;
    CdhrString sretText = null;
    CdhrString sretURL = null;
    CdhrInt iretTarget = null;

    CdhrObjid oret = en.gdh.getChild( oretWebH.objid);
    while ( oret.oddSts()) {
      CdhrClassId retCid = en.gdh.getObjectClass( oret.objid);
      if ( retCid.evenSts()) return;

      switch( retCid.classId) {
        case Pwrb.cClass_WebGraph:
          sret = en.gdh.objidToName( oret.objid, Cdh.mName_volumeStrict);            
          if ( sret.evenSts()) return;

          s = sret.str + ".Name";
          sretName = en.gdh.getObjectInfoString( s);

          s = sret.str + ".Text";
          sretText = en.gdh.getObjectInfoString( s);

 
          button = new OpWindButton( session, sretName.str, sretText.str,
						  OpWindButton.WEBGRAPH);
          this.add( button);
          break;
      }
      oret = en.gdh.getNextSibling( oret.objid);
    }

    this.add( new JSeparator());

    oret = en.gdh.getChild( oretWebH.objid);
    while ( oret.oddSts()) {
      CdhrClassId retCid = en.gdh.getObjectClass( oret.objid);
      if ( retCid.evenSts()) return;

      switch( retCid.classId) {
        case Pwrb.cClass_WebLink:
          sret = en.gdh.objidToName( oret.objid, Cdh.mName_volumeStrict);
          if ( sret.evenSts()) return;

          s = sret.str + ".URL";
          sretURL = en.gdh.getObjectInfoString( s);

          s = sret.str + ".Text";
          sretText = en.gdh.getObjectInfoString( s);

          s = sret.str + ".WebTarget";
          iretTarget = en.gdh.getObjectInfoInt( s);

	  String cmd = "open url \"" + sretURL.str + "\"";
	  if ( iretTarget.value == Pwrb.eWebTargetEnum_RightFrame)
	    cmd = cmd + " /name=\"right\"";
	  else if ( iretTarget.value == Pwrb.eWebTargetEnum_ParentWindow)
	    cmd = cmd + " /name=\"_parent\"";
 
          button = new OpWindButton( session, cmd, sretText.str,
				     OpWindButton.WEBLINK);
          this.add( button);
          break;
      }
      oret = en.gdh.getNextSibling( oret.objid);
    }
  }

  class OpWindButton extends JButton {
    public static final int WEBGRAPH = 1;
    public static final int WEBLINK = 2;
    public static final int LOGIN = 3;
    public static final int LOGOUT = 4;
    public static final int NAVIGATOR = 5;
    public static final int ALARMLIST = 6;
    public static final int EVENTLOG = 7;
    public static final int HELP = 8;
    public static final int PROVIEW = 9;
    public static final int LANGUAGE = 10;
    JopSession session;
    String action;
    int type;
    boolean scrollbar;
    String instance;

    public Dimension getPreferredSize() {
      return new Dimension( 200, 25);
    }
    public Dimension getMininumSize() { return getPreferredSize();}
    public Dimension getMaximumSize() { return getPreferredSize();}

    public OpWindButton( JopSession bsession, String name, String text, int btype) {
      this.session = bsession;
      this.action = name;
      this.type = btype;
      setText( text);
      setHorizontalTextPosition( SwingConstants.LEFT);
      this.addMouseListener(new MouseAdapter() {
        public void mouseReleased(MouseEvent e) {
	  switch ( type) {
	    case NAVIGATOR:
	      if ( ! en.gdh.isAuthorized( Pwr.mAccess_AllPwr))
		break;
	      session.openNavigator( null);
	      break;
	    case LOGIN:
	      session.openLogin();
	      break;
	    case LOGOUT:
	      en.gdh.logout();
	      setLabelText( " ");
	      break;
	    case ALARMLIST:
	      if ( ! en.gdh.isAuthorized( Pwr.mAccess_AllPwr))
		break;
	      session.openAlarmList();
	      break;
	    case EVENTLOG:
	      if ( ! en.gdh.isAuthorized( Pwr.mAccess_AllPwr))
		break;
	      session.openEventLog();
	      break;
	    case HELP:
	      if ( ! en.gdh.isAuthorized( Pwr.mAccess_AllPwr))
		break;
	      session.executeCommand("help index");
	      break;
	    case PROVIEW:
	      if ( ! en.gdh.isAuthorized( Pwr.mAccess_AllPwr))
		break;
	      session.executeCommand("open url \"$pwr_doc/" + session.getLang() + "/index.html\"");
	      break;
	    case WEBGRAPH:
	      if ( ! en.gdh.isAuthorized( Pwr.mAccess_AllPwr))
		break;
	      session.openGraphFrame( action, instance, scrollbar, false);
	      break;
	    case WEBLINK:
	      if ( ! en.gdh.isAuthorized( Pwr.mAccess_AllPwr))
		break;
	      String cmd = "open url \"" + action + "\"";
	      session.executeCommand( action);
	      break;
	    case LANGUAGE:
	      if ( ! en.gdh.isAuthorized( Pwr.mAccess_AllPwr))
		break;
	      session.openLanguage();
	      break;
	  }
	  System.out.println( "Action: " + action);
	}
	  });
    }

    public void setScrollbar( boolean scrollbar) {
      this.scrollbar = scrollbar;
    }
    public void setInstance( String instance) {
      this.instance = instance;
    }
  }

  public void setLabelText( String text) {
    label.setText( text);
  }

  public void setLanguage( int language) {

    if ( root instanceof JFrame)
      ((JFrame)root).setTitle( JopLang.transl("Operator Window"));

    if ( langButton != null)
      langButton.setText( JopLang.transl("Language"));
    if ( loginButton != null)
      loginButton.setText( JopLang.transl("Login"));
    if ( logoutButton != null)
      logoutButton.setText( JopLang.transl("Logout"));
    if ( alarmButton != null)
      alarmButton.setText( JopLang.transl("Alarm and Event List"));
    if ( eventLogButton != null)
      eventLogButton.setText( JopLang.transl("Event Log"));
    if ( navigatorButton != null) 
      navigatorButton.setText( JopLang.transl("Navigator"));
    if ( helpButton != null) 
      helpButton.setText( JopLang.transl("Help"));
  }
}













