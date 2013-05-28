CGI Server Camera GUI

File list:
jquery*                     - jquery library
jqm-datebox*                - date box library
images/*                    - images used vlc plugin
stretchlogosmall.jpg        - Stretch Logo
index.html                  - the entry point html file.
video.html                  - html page for the vlc plugin
styles.css                  - misc styles
camera_control.css          - GUI specific css
control_script.js           - top level javascript code
camera_control_obj.js       - base class for constructing UI elements and excute set and get commands.
detailed_control_objs.js    - class definitions for each control page.

++++++++++++++++++++++++++++ 
index.html

- List of links:
Section under the heading "Camera Configurator", line 38, is for the list of links to each subpage.  The tag "linkReference", line 44, is cloned for each link to different pages.  The element "linkReference" itself is hidden.

- Configuration pages:
Section under the section "detailedControlReference" is the template for each configuration page.  This section is cloned for each configuration page, and modified to reflect the detailed configuration of that configuration page.  Each UI element, such as input box, is clone from its template.  The orginal UI element templates are hidden.

- back and logout button:
Line 50, "adta-add-back-btn", added the back button.  Line 53, "headerButtonLogout" adds logout button.


++++++++++++++++++++++++++++ 

control_script.js

"control_script.js" is the entry point of the javascript application.  It setup an array of "configItems", with each element representing a configuration page.  It create a list of link references to each of the configuration page, and register a click event callback that will redirect the view to the specific configuration page.

The script also test if the user is logged in by requesting the date from the server and parse the result.


++++++++++++++++++++++++++++ 

camera_control_obj.js

This is the parent class of each configuration page.

Member function "this.add*" adds a particular UI element.  New member function must be added if new UI element is introduced.

Member function "this.setupControls" parse the "controlArray" data structure and populate all the UI elements of this particular control page.

Member function "this.processGetCommand" is triggered by click events and will send the get command to the CGI server.  The optional "postProcessFunc" parameter allow user to overwrite the default UI status fill operation.

Member function "this.processSetCommand" is triggered by click events and will send the set command to the CGI server.

The "logout" event callback is register at the end of this function as well.

++++++++++++++++++++++++++++ 

detailed_control_objs.js

Sub class definition of each configuration page.  Each subclass have a "this.controlArray" memory data structure.  This array defines the content and order of all the UI elements on that page.  More entries can be added but must make sure that there is no naming conflict in the "id" field.

"*.process" member method is excuted once the first time user enter a particular configuration page.  This member function register all clicking event callbacks such as "set" and "get".  It typically call "get" once in the beginning to fill the UI elements with current status.

"CameraCtrlLogin" generate the login page.  It will redirect user to the main page if login is successful.

"CameraCtrlImage" generate the image control page.  It use the standard set and get methods.

"CameraCtrlDate" generate the date control page.  It use a custom post processing method for get, because the date element require a different format.  User control is grouped in this page as well.

"CameraCtrlStatus" generate the status page.  It doesn't use standard get method because the data need to be represented in a different format.

"CameraCtrlDeviceInfo" generate the device info page.  It use the standard get and set method, but reset and reboot are treated differently.

"CameraCtrlStream" generate the stream config page. The "this.showArray" and "this.hideArray" selectively show and hide element that is applicable.

"CameraCtrlSnapshot" generate the snapshot control page.  It use date as a special signiture for each snapshot, so the browser will refresh the image with each new snapshot.

"CameraCtrlFirmware" gernate the firmware update page.  It is currently disabled.
