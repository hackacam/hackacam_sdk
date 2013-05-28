
$(document).ready(function() {
    var configItems = [new CameraCtrlLogin(), 
                       new CameraCtrlDate(),
                       new CameraCtrlImage(),
                       new CameraCtrlRoi(),
                       new CameraCtrlStream(),
                       new CameraCtrlSnapshot(),
                       new CameraCtrlStatus(),
                       new CameraCtrlDeviceInfo(),
//                       new CameraCtrlFirmware(),
                      ];
    $("#linkReference").hide();
    for (var i in configItems){
        configObj = configItems[i];
        var linkItem = $("#linkReference").clone();
        linkItem.attr('id', configObj.controlName);
        linkItem.attr('index', i);
        linkItem.find('h3').text(configObj.controlName); 
        linkItem.find('a').attr("href", "index.html#"+configObj.controlId);
        if (i!=0){
            linkItem.show();
        }
        linkItem.appendTo($("#linkReference").parent());                
        linkItem.click({index:i}, function(event){
            $("#configItems").ready(function(){
                var configItem = configItems[event.data.index];                
                // process the controls
                if (configItem.runCounter==0){
                    configItem.runCounter=1;
                    configItem.process();
                }
            });
        });
    }
    
    // test if we are logged in.  if not navigate to login page
    serverPrefix = "http://" + window.location.hostname + ":" + window.location.port + "/cgi-bin/date";
    urlStr = serverPrefix+"?action=get";
    $.get(urlStr, function(data){
        if (data.split(" ")[0]==="Error:"){
            console.log(data);
            // navigate to login page
            var loginPage=$("#Login").find("a");
            loginPage.click();
        }
    });
});
