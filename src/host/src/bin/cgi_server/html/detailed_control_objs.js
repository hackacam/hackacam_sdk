
////////////////////////////////////////////////////////////////////////////
// Login/logout class
////////////////////////////////////////////////////////////////////////////

function CameraCtrlLogin(){
    this.controlArray = [{type:"label", value:"Username", id:"usernameLable"},
                         {type:"input", subtype:"text", defaultText:"root", id:"usernameInput"},
                         {type:"label", value:"Password", id:"passwordLable"},
                         {type:"input", subtype:"password", defaultText:"password", id:"passwordInput"},
                         {type:"button", label:"Login", id:"loginButton"},
                        ];    
    CameraCtrl.call(this, "Login", "loginLogout");
}

CameraCtrlLogin.prototype = Object.create(CameraCtrl.prototype);

CameraCtrlLogin.prototype.process = function(){
    var self = this;

    // process login
    $("#loginButton").click(function(){
        var url = self.serverPrefix + "login";
        var user = $("#usernameInput").val();   
        var password =$("#passwordInput").val();
        $.get(url,
              {user:user, password:password}, 
              function(data){
                  self.configSection.find("#commandStatus").html("Action Status: " + data);
                  if (data.indexOf("OK")==0){
                      // navigate back to main page
                      window.location.href="http://" + window.location.hostname + ":" + window.location.port;
                  }
              });       
    });
}

////////////////////////////////////////////////////////////////////////////
// Image control class
////////////////////////////////////////////////////////////////////////////

function CameraCtrlImage(){
    this.controlArray = [{type:"radioButton", name:"Flip", values:["none", "horizontal", "vertical", "both"], id:"flipRadioButton"},
                         {type:"label", value:"Brightness", id:"brightnessLable"},
                         {type:"slider", value:"50", min:"0", max:"100", id:"brightnessSlider"},
                         {type:"label", value:"Hue", id:"hueLable"},
                         {type:"slider", value:"50", min:"0", max:"100", id:"hueSlider"},
                         {type:"label", value:"Saturation", id:"saturationLable"},
                         {type:"slider", value:"50", min:"0", max:"100", id:"saturationSlider"},
                         {type:"label", value:"Contrast", id:"contrastLable"},
                         {type:"slider", value:"50", min:"0", max:"100", id:"contrastSlider"},
                         {type:"label", value:"Sharpness", id:"sharpnessLable"},
                         {type:"slider", value:"50", min:"0", max:"100", id:"sharpnessSlider"},
                         {type:"label", value:"Exposure", id:"exposureLable"},
                         {type:"slider", value:"50", min:"0", max:"100", id:"exposureSlider"},
                         {type:"label", value:"Motion rate", id:"motionRateLable"},
                         {type:"slider", value:"0", min:"0", max:"100", id:"motion_rateSlider"},
                         {type:"radioButton", name:"TDN", values:["auto", "hardware", "day", "night"], id:"tdnRadioButton"},
                         {type:"radioButton", name:"Sensor Rate", values:["25", "30"], id:"sensor_rateRadioButton"},
                         {type:"toggle", name:"OSD on/off", label1:"true", label2:"false", id:"image_osdToggle"},
                         {type:"toggle", name:"CVBS on/off", label1:"true", label2:"false", id:"cvbsToggle"},
                         {type:"dualButton", name:"imageAdjust", label1:"Save Settings", label2:"Get Settings", id:"imageSetDualButton"},
                        ];    
    CameraCtrl.call(this, "Image Configuration", "imageControl");
}
CameraCtrlImage.prototype = Object.create(CameraCtrl.prototype);
CameraCtrlImage.prototype.process = function(){
    var self = this;
    var url = self.serverPrefix + "image"

    // get current settting
    this.processGetCommand($("#imageSetDualButton").find("#buttonB"), self, url);

    // process image control
    this.processSetCommand($("#imageSetDualButton").find("#buttonA"), self, url);
    $("#imageSetDualButton").find("#buttonB").click();

//     // over write flip control
//     $("#flipRadioButton").click(function(){
//         var searchStr = "input[value=" + "none" + "]";
//         var radioCheckElement = $("#flipRadioButton").find(searchStr);
// //        $("#flipRadioButton").find(searchStr).attr("checked", true);
//         radioCheckElement.attr("checked", true);
//         $("#flipRadioButton").find("input").checkboxradio("refresh");
//         console.log("lkjaslkfj")
//     })
}

function CameraCtrlDate(){
    this.controlArray = [{type:"label", value:"New Password", id:"newPasswordLable"},
                         {type:"input", subtype:"password", defaultText:"password", id:"newPasswordInput"},
                         {type:"input", subtype:"password", defaultText:"password", id:"newPasswordInput2"},
                         {type:"button", label:"Change Password", id:"setUserButton"},
                         {type:"label", value:"Date and Time", id:"dateTimeSectionLable"},
                         {type:"calendar", name:"Date", id:"dateCalendar"},
                         {type:"labelInput", name:"Time", id:"timeLabelInput"},
                         {type:"labelInput", name:"Timezone", id:"timezoneLabelInput"},
                         {type:"toggle", name:"DST", label1:"true", label2:"false", id:"dstToggle"},
                         {type:"dualButton", name:"setGet", label1:"Get Date", label2:"Set Date", id:"setGetDualButton"},
                        ];
    CameraCtrl.call(this, "Admin", "date");   
}

CameraCtrlDate.prototype = Object.create(CameraCtrl.prototype);
CameraCtrlDate.prototype.getDate = function(data){
    if (data.indexOf("Error:")==-1){
        data = data.replace(/\n/g, " ").replace(/\s+/g, " ").replace(/\//g, " ");
        if (data.charAt(0)===" ") data = data.slice(1);
        if (data.charAt(data.length-1)===" "){
            data = data.slice(0,-1);
        }
        data = data.split(" ");
        // set the current date
        var dateString = data[2] + '-' + data[0] + '-' + data[1];
        $("#dateCalendar").find("#calendarCalendar").val(dateString);
        // set the current time string
        var timeString = data[3];
        $("#timeLabelInput").find("#labelInput").val(timeString);
        // set the time zone
        var timezoneString = data[4];
        $("#timezoneLabelInput").find("#labelInput").val(timezoneString);
        // set the DST
        if (typeof data[5] == 'undefined'){
            $("#dstToggle").find("#flip-1").val("false");
        }else{
            $("#dstToggle").find("#flip-1").val("true");        
        }
        $("#dstToggle").find("#flip-1").slider("refresh");
        $("#date").find("#commandStatus").html("Action Status: " + "OK");
    }else{
        $("#date").find("#commandStatus").html("Action Status: " + data);               
    }

}

CameraCtrlDate.prototype.process = function(){
    var self = this;
    var url = self.serverPrefix + "date";

    // get current settting
    this.processGetCommand($("#setGetDualButton").find("#buttonA"), this, url, true, self.getDate);
    
    // set current date
    $("#setGetDualButton").find("#buttonB").click(function(){
        var dateArray = $("#dateCalendar").find("#calendarCalendar").val().split("-");
        var timeArray = $("#timeLabelInput").find("#labelInput").val().split(":");
        var timezoneString = $("#timezoneLabelInput").find("#labelInput").val();
        var dstVal = $("#dstToggle").find("#flip-1").val();
        var urlStr = "?action=set" + "&year=" + dateArray[0] + "&month=" + dateArray[1] +
            "&day=" + dateArray[2] + "&hour=" + timeArray[0] + "&minute=" + timeArray[1] +
            "&second=" + timeArray[2] + "&timezone=" + timezoneString + "&dst=" + dstVal;
        $.get(url + urlStr,
              function(data){
                  if (data.split(" ")[0]==="Error:"){
                      self.configSection.find("#commandStatus").html("Action Status: " + data);
                  }else{
                      self.configSection.find("#commandStatus").html("Action Status: " + "OK");
                  }
              });

    });
    
    // run first get 
    $("#setGetDualButton").find("#buttonA").click();

    // process set user
    $("#setUserButton").click(function(){
        var url = self.serverPrefix + "user";
        var action = "set";
        var user = "root";
        var password = $("#newPasswordInput").val();
        var password2 = $("#newPasswordInput2").val();
        console.log(password);
        if (password==password2){
            $.get(url,
                  {action:action, user:user, password:password}, 
                  function(data){
                      self.configSection.find("#commandStatus").html("Action Status: " + data);
                  });       
        }else{
            self.configSection.find("#commandStatus").html("Action Status: " + "new password doesn't match.");
        }
    });                             
}

////////////////////////////////////////////////////////////////////////////
// status class
////////////////////////////////////////////////////////////////////////////

function CameraCtrlStatus(){
    this.controlArray = [{type:"button", label:"Get Camera Status", id:"getStatusButton"},
                        ];
    CameraCtrl.call(this, "Camera Status Log", "status");   
}

CameraCtrlStatus.prototype = Object.create(CameraCtrl.prototype);
CameraCtrlStatus.prototype.process = function(){
    var self = this;
    var url = self.serverPrefix;

    // get current settting
    $("#getStatusButton").click(function(){
        $.get(url + "status",
              function(data){
                  data = data.replace(/\n/g, "\n<br>\n");
                  data = data.replace(/,/g, ', ');
                  self.configSection.find("#commandStatus").html("Action Status: " + data);
              });
    });
}

////////////////////////////////////////////////////////////////////////////
// deviceInfo class
////////////////////////////////////////////////////////////////////////////

function CameraCtrlDeviceInfo(){
    this.controlArray = [{type:"label", value:"1234", id:"firmwareInfoLable"},
                         {type:"label", value:"Hostname", id:"hostnameLable"},
                         {type:"input", subtype:"text", defaultText:"ipcam-00:00:00:00:00:00:00", id:"hostnameInput"},
                         {type:"label", value:"IP Address", id:"ipAddressLable"},
                         {type:"input", subtype:"text", defaultText:"192.168.1.60", id:"ip_addressInput"},
                         // {type:"label", value:"DHCP", id:"dhcpLable"},
                         // {type:"input", subtype:"text", defaultText:"true", id:"dhcpInput"},
                         {type:"label", value:"Subnet", id:"subnetLable"},
                         {type:"input", subtype:"text", defaultText:"255.255.255.0", id:"subnetInput"},
                         {type:"label", value:"Gateway", id:"gatewayLable"},
                         {type:"input", subtype:"text", defaultText:"192.168.1.1", id:"gatewayInput"},
                         {type:"label", value:"Model", id:"modelLable"},
                         {type:"input", subtype:"text", defaultText:"ipcam", id:"modelInput"},
                         {type:"label", value:"Motion Destination", id:"motionDstLable"},
                         {type:"input", subtype:"text", defaultText:"192.168.1.1:9000", id:"motion_destInput"},
                         {type:"label", value:"Packet Gap", id:"packetGapLable"},
                         {type:"input", subtype:"text", defaultText:"0", id:"packet_gapInput"},
                         {type:"label", value:"User Data", id:"userDataLable"},
                         {type:"input", subtype:"text", defaultText:"", id:"user_dataInput"},
//                         {type:"label", value:"MAC Address", id:"macAddressLable"},
                         {type:"toggle", name:"DHCP on/off", label1:"true", label2:"false", id:"dhcpToggle"},
                         {type:"label", value:"00:00:00:00:00:00", id:"mac_addressInfoLable"},
//                         {type:"label", value:"Serial Number", id:"serialNumberLable"},
                         {type:"label", value:"1234", id:"serial_numberInfoLable"},
                         {type:"label", value:"1234", id:"uptimeInfoLable"},
                         {type:"button", label:"Get Settings", id:"getDeviceInfoButton"},
                         {type:"button", label:"Save Settings", id:"setDeviceInfoButton"},
                         {type:"button", label:"Reset Camera", id:"resetButton"},
                         {type:"button", label:"Reboot Camera", id:"rebootButton"},
                        ];
    CameraCtrl.call(this, "Device Info", "deviceInfo");   
}

CameraCtrlDeviceInfo.prototype = Object.create(CameraCtrl.prototype);
CameraCtrlDeviceInfo.prototype.process = function(){
    var self = this;
    var url = self.serverPrefix + "device_info";

    // get current settting
    this.processGetCommand($("#getDeviceInfoButton"), self, url);

    // set new settings
    this.processSetCommand($("#setDeviceInfoButton"), self, url, true);

    // reboot the camera
    $("#rebootButton").click(function(){
        var urlStr = url.replace("device_info", "reboot");
        $.get(urlStr,
              function(data){
                  self.configSection.find("#commandStatus").html("Action Status: " + data);
              });
    });
    // reset the camera
    $("#resetButton").click(function(){
        var urlStr = url.replace("device_info", "reset");
        $.get(urlStr,
              function(data){
                  self.configSection.find("#commandStatus").html("Action Status: " + data);
              });
    });    

    $("#getDeviceInfoButton").click();
}

////////////////////////////////////////////////////////////////////////////
// Stream class
////////////////////////////////////////////////////////////////////////////

function CameraCtrlStream(){
    this.controlArray = [{type:"radioButton", name:"Stream", values:["0", "1", "2", "3"], id:"idRadioButton"},                  
                         {type:"radioButton", name:"Encoder", values:["h264", "mjpeg", "mpeg4"], id:"encoderRadioButton"},
                         {type:"radioButton", name:"Profile", values:["base", "main", "high"], id:"profileRadioButton"},
                         {type:"radioButton", name:"Rate Control", values:["cbr", "vbr", "cq"], id:"rate_controlRadioButton"},
                         {type:"label", value:"FPS", id:"fpsLable"},
                         {type:"slider", value:"30", min:"1", max:"30", id:"fpsSlider"},
                         {type:"label", value:"Width", id:"widthLable"},
                         {type:"slider", value:"176", min:"176", max:"1920", step:"16", id:"widthSlider"},
                         {type:"label", value:"Height", id:"heightLable"},
                         {type:"slider", value:"128", min:"128", max:"1080", step:"16", id:"heightSlider"},
                         {type:"label", value:"Bitrate", id:"bitrateLable"},
                         {type:"slider", value:"5000", min:"100", max:"15000", id:"bitrateSlider"},
                         {type:"label", value:"Max Bitrate", id:"maxBitrateLable"},
                         {type:"slider", value:"5000", min:"100", max:"15000", id:"max_bitrateSlider"},
                         {type:"label", value:"Quality", id:"qualityLable"},
                         {type:"slider", value:"80", min:"10", max:"90", id:"qualitySlider"},
                         {type:"label", value:"GOP", id:"gopLable"},
                         {type:"slider", value:"15", min:"1", max:"30", id:"gopSlider"},

                         {type:"toggle", name:"Enable Stream", label1:"true", label2:"false", id:"enableToggle"},
                         {type:"toggle", name:"OSD", label1:"true", label2:"false", id:"stream_osdToggle"},
                         {type:"dualButton", name:"getSetStream", label1:"Save Settings", label2:"Get Settings", id:"streamSetGetDualButton"},
                         {type:"button", label:"Open Streams", id:"openStreamButton"},
                        ];

    this.showArray = [["#bitrateSlider", "#gopSlider", "#profileRadioButton", "#rate_controlRadioButton", "#max_bitrateSlider"],
                      ["#qualitySlider"],                          
                      ["#bitrateSlider", "#gopSlider", "#rate_controlRadioButton", "#max_bitrateSlider"],
                     ];
    this.hideArray = ["#bitrateSlider", "#gopSlider", "#profileRadioButton", "#qualitySlider", "#rate_controlRadioButton", "#max_bitrateSlider"];

    CameraCtrl.call(this, "Stream Configuration", "Stream");   
}

CameraCtrlStream.prototype = Object.create(CameraCtrl.prototype);
CameraCtrlStream.prototype.process = function(){
    var self = this;
    var url = self.serverPrefix + "stream";
    // get current settting
    $("#streamSetGetDualButton").find("#buttonB").click(function(){
        var chName = $("#idRadioButton").find("input:checked").val();
        var tempUrl = url + "?action=get&id=" + chName;
//        console.log(tempUrl);
        $.get(tempUrl,
             function(data){
                 // console.log(data);
                if (data.indexOf("Error:")!=-1){
                    self.configSection.find("#commandStatus").html("Action Status: " + data);                   
                }else{
                    self.configSection.find("#commandStatus").html("Action Status: " + "OK");
                    self.currentStatusFill(data);
                    $("#encoderRadioButton").change();
                }
             });
    });
    
    // set stream
    this.processSetCommand($("#streamSetGetDualButton").find("#buttonA"), self, url);

    // selectively hide options
    $("#encoderRadioButton").change(function(){
        var showIndex = 0;
        if ($("#encoderRadioButton").find("input:checked").val()=="h264"){
            showIndex = 0;
        }else if ($("#encoderRadioButton").find("input:checked").val()=="mjpeg"){
            showIndex = 1;
        }else{
            showIndex = 2;
        }

        // hide all buttons in the hide list
        for (var i in self.hideArray){
            try{$(self.hideArray[i]).slider("disable");}
            catch(err)
            {$(self.hideArray[i]).find("input").checkboxradio("disable");}
        }

        // show all buttons in the show list
        for (var j in self.showArray[showIndex]){
            try{$(self.showArray[showIndex][j]).slider("enable");}
            catch(err)
            {$(self.showArray[showIndex][j]).find("input").checkboxradio("enable");}
        }
        
        // for (var i in self.showHideArray){
        //     for (var j in self.showHideArray[i]){
        //         if (i==showIndex){
        //             try{$(self.showHideArray[i][j]).slider("enable");}
        //             catch(err)
        //             {$(self.showHideArray[i][j]).find("input").checkboxradio("enable");}
        //         }else{
        //             try{$(self.showHideArray[i][j]).slider("disable");}
        //             catch(err)
        //             {$(self.showHideArray[i][j]).find("input").checkboxradio("disable");}
        //         }
        //     }
        // }       
    });

    // trigger a get when channel change
    $("#idRadioButton").change(function(){
        $("#streamSetGetDualButton").find("#buttonB").click();
    });

    // select first channel to start
    $("#idRadioButton").find("input[value=0]").attr("checked", true);
    $("#idRadioButton").change();

    // open rtsp stream in a new tab
    $("#openStreamButton").click(function(){
        window.open('http://' + window.location.hostname + '/video.html');
    });
}


////////////////////////////////////////////////////////////////////////////
// snapshot class
////////////////////////////////////////////////////////////////////////////

function CameraCtrlSnapshot(){
    this.controlArray = [{type:"label", value:"Decimation", id:"decimationLable"},
                         {type:"input", subtype:"text", defaultText:"1", id:"decimationInput"},
                         {type:"button", label:"Save Settings", id:"setSnapshotButton"},
                         {type:"button", label:"Get Settings", id:"getSnapshotButton"},
                         {type:"button", label:"Get Snapshot", id:"sendSnapshotButton"},
                        ];
    CameraCtrl.call(this, "Snapshot Configuration", "snapshot");   
    this.last_snapshot_time = 0;
}

CameraCtrlSnapshot.prototype = Object.create(CameraCtrl.prototype);
CameraCtrlSnapshot.prototype.process = function(){
    var self = this;
    var url = self.serverPrefix + "snapshot";
    // get current settting
    this.processGetCommand($("#getSnapshotButton"), self, url);
    // set snapshot
    this.processSetCommand($("#setSnapshotButton"), self, url);
    // send snapshot
    $("#sendSnapshotButton").click(function(){
        var delayTimer = new Date();
        var currentTime = delayTimer.getTime();
        var currentDelta = currentTime - self.last_snapshot_time;
        if (currentDelta > 3000){
            $.get(url,
                  {action:"send"},
                  function(data){
                      var imageTag = "<img src=\"snapshot.jpg?" + currentTime + "\">";
                      self.configSection.find("#commandStatus").html(imageTag);
                      self.last_snapshot_time = currentTime
                  });
        }
    });

    $("#getSnapshotButton").click();
}


////////////////////////////////////////////////////////////////////////////
// firmware update class
////////////////////////////////////////////////////////////////////////////

function CameraCtrlFirmware(){
    this.controlArray = [{type:"label", value:"Updating u-boot", id:"uBootLable"},
                         {type:"fileUpload", fileName:"u-boot.bin", id:"uBootFileUpload"},
                         {type:"label", value:"Updating uImage", id:"uImageLable"},
                         {type:"fileUpload", fileName:"uImage", id:"uImageFileUpload"},
                         {type:"label", value:"Updating rootfs", id:"rootfsLable"},
                         {type:"fileUpload", fileName:"rootfs.img.gz", id:"rootfsFileUpload"},
                        ];
    CameraCtrl.call(this, "Firmware", "Firmware");
}

CameraCtrlFirmware.prototype = Object.create(CameraCtrl.prototype);
CameraCtrlFirmware.prototype.process = function(){
   var self = this;
   console.log($("#rootfsFileUpload"));

   $("#fileUploadHiddenIFrame").load(function(){
           self.configSection.find("#commandStatus").html($("#fileUploadHiddenIFrame").contents().find("pre").text());
       });
//    $("form").submit(function(){
//            self.configSection.find("#commandStatus").html("uploading");
//        });
}


////////////////////////////////////////////////////////////////////////////
// roi class
////////////////////////////////////////////////////////////////////////////

function CameraCtrlRoi(){
    this.controlArray = [{type:"button", label:"Refresh", id:"refreshSnapshotButton"},
                         {type:"dualButton", name:"addDelete", label1:"Add", label2:"Delete", id:"roiAddDelDualButton"},
                         {type:"button", label:"Delete All", id:"deleteAllButton"},
                         {type:"radioButton", name:"ID", values:["0", "1", "2", "3", "4"], id:"roiIdRadioButton"},
                        ];
    CameraCtrl.call(this, "ROI Configuration", "ROI");   
    this.coords = [0, 0, 0, 0];
    this.last_refresh_time = 0;
}

CameraCtrlRoi.prototype.showCoord = function(coords){
    console.log(coords.x);
    console.log(coords.y);
    console.log(coords.x2);
    console.log(coords.y2);
}
CameraCtrlRoi.prototype = Object.create(CameraCtrl.prototype);
CameraCtrlRoi.prototype.process = function(){
    var self = this;
    var snapshotUrl = self.serverPrefix + "snapshot";
    var url = self.serverPrefix + "roi";

    var coordsCallback = function(c){
        var width = $("#roiImage").width();
        var height = $("#roiImage").height();
        
        self.coords[0] = c.x/width;
        self.coords[1] = c.y/height;
        self.coords[2] = c.x2/width;
        self.coords[3] = c.y2/height;
    };

    $("#refreshSnapshotButton").click(function(){
        var delayTimer = new Date();
        var currentTime = delayTimer.getTime();
        var currentDelta = currentTime - self.last_refresh_time;
        if (currentDelta > 3000){
            $.get(snapshotUrl,
                  {action:"send"},
                  function(data){
                      var imageTag = "<img src=\"snapshot.jpg?" + currentTime + "\" id=\"roiImage\" />";
                      self.configSection.find("#roiReferenceImage").html(imageTag);
                      self.last_refresh_time = currentTime;
                      $("#roiImage").Jcrop({
                          onChange: coordsCallback,
                          onSelect: coordsCallback
                      });                     
                      self.coords[0]=-1;
                  });

        }
    });

    // initial refresh
    $("#refreshSnapshotButton").click();

    // set the default id selection
    var radioCheckElement = $("#roiIdRadioButton").find("input[value=0]");
    radioCheckElement.attr("checked", true);

    // add region
    $("#roiAddDelDualButton").find("#buttonA").click(function(){
        if (self.coords[0]!=-1){
            var coordStr = self.coords[0] + "," + self.coords[1] + "," + self.coords[2] + "," + self.coords[3];
            var layerId = $("#roiIdRadioButton").find("input:checked").val();
            var tmpUrl = url + "?action=add&id=" + layerId + "&rectangles="+coordStr;
            $.get(tmpUrl,
                  function(data){
                      self.configSection.find("#commandStatus").html("Action Status: " + data);
                      $("#refreshSnapshotButton").click();
                  });
        }
    });

    // delete region
    $("#roiAddDelDualButton").find("#buttonB").click(function(){
        if (self.coords[0]!=-1){
            var coordStr = self.coords[0] + "," + self.coords[1] + "," + self.coords[2] + "," + self.coords[3];
            var layerId = $("#roiIdRadioButton").find( "input:checked").val();
            var tmpUrl = url + "?action=delete&id=" + layerId + "&rectangles="+coordStr;
            $.get(tmpUrl,
                  function(data){
                      self.configSection.find("#commandStatus").html("Action Status: " + data);
                      $("#refreshSnapshotButton").click();
                  });
        }
    });

    // delete all
    $("#deleteAllButton").click(function(){
        var layerId = $("#roiIdRadioButton").find("input:checked").val();
        var tmpUrl = url + "?action=clear&id=" + layerId;
        $.get(tmpUrl,
              function(data){
                  self.configSection.find("#commandStatus").html("Action Status: " + data);
                  $("#refreshSnapshotButton").click();
              });
    });
}
