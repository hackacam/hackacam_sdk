
function CameraCtrl(controlName, controlId){
    var detailedCtrSectionRef = $("#detailedControlReference");
    var self = this;
    this.runCounter = 0;

    this.serverPrefix = "http://" + window.location.hostname + ":" + window.location.port + "/cgi-bin/";
    this.controlName = controlName;
    this.controlId = controlId;

    this.addLabel = function (label){
        var newLabel = $("#labelReference").clone();
        newLabel.attr("id", label.id);
        newLabel.text(label.value);
        newLabel.appendTo(this.configItems);
    }

    this.addInput = function (inputBox){
        var newInputBox = $("#inputReference").clone();
        newInputBox.attr("id", inputBox.id);
        newInputBox.attr("type", inputBox.subtype);
        newInputBox.val(inputBox.defaultText);
        newInputBox.appendTo(this.configItems);
    }

    this.addButton = function(button){
        var newButton = $("#buttonReference").clone();
        newButton.attr("id", button.id);        
        newButton.text(button.label);
        newButton.appendTo(this.configItems);
    }

    this.addToggle = function(toggle){
        var newToggle = $("#toggleReference").clone();
        newToggle.attr("id", toggle.id);
        newToggle.find("#opt1").attr("value", toggle.label1);
        newToggle.find("#opt1").text(toggle.label1);
        newToggle.find("#opt2").attr("value", toggle.label2);
        newToggle.find("#opt2").text(toggle.label2);
        newToggle.find("#toggleLabel").text(toggle.name);
        newToggle.appendTo(this.configItems);
    }

    this.addSlider = function(slider){
        var newSlider = $("#sliderReference").clone();
        newSlider.attr("id", slider.id);
        newSlider.attr("name", slider.id);
        newSlider.attr("value", slider.value);
        newSlider.attr("min", slider.min);
        newSlider.attr("max", slider.max);
        if (typeof slider.step != 'undefined'){
            newSlider.attr("step", slider.step);
        }
        newSlider.appendTo(this.configItems);
    }

    this.addCalendar = function(calendar){
        var newCalendar = $("#calendarReference").clone();
        newCalendar.attr("id", calendar.id);
        newCalendar.find("#calendarLabel").text(calendar.name);
        newCalendar.appendTo(this.configItems);
    }

    this.addLabelInput = function(labelInput){
        var newLabelInput = $("#labelInputReference").clone();
        newLabelInput.attr("id", labelInput.id);
        newLabelInput.find("#inputLabel").text(labelInput.name);
        newLabelInput.appendTo(this.configItems);
    }

    this.addDualButton = function(dualButton){
        var newDualButton = $("#dualButtonReference").clone();
        newDualButton.attr("id", dualButton.id);
        newDualButton.find("#buttonA").text(dualButton.label1);
        newDualButton.find("#buttonB").text(dualButton.label2);
        newDualButton.appendTo(this.configItems);
    }
    
    this.addRadioButton = function(radioButton){
        var newRadioButton = $("#radioReference").clone();
        newRadioButton.attr("id", radioButton.id);
        newRadioButton.find("legend").text(radioButton.name);
        var init = false;
        for (var i in radioButton.values){
            var newRadio = newRadioButton.find("#radio-choice-reference").clone();
            var newLabel = newRadioButton.find("#radio-label-reference").clone();
            newRadio.attr("id", radioButton.values[i]);
            newRadio.attr("value", radioButton.values[i]);  
            newLabel.attr("id", radioButton.values[i]+"Label");
            newLabel.attr("for", radioButton.values[i]);
            newLabel.text(radioButton.values[i]);
            newLabel.appendTo(newRadioButton);
            newRadio.appendTo(newRadioButton);
//          console.log(newRadioButton);
        }
        newRadioButton.find("#radio-choice-reference").remove();
        newRadioButton.find("#radio-label-reference").remove();
        newRadioButton.find("input").attr("name", "radio-choice-" + radioButton.id);
        newRadioButton.appendTo(this.configItems);
    }

    this.addFileUpload = function(fileUpload){
        var newFileUpload = $("#fileUploadReference").clone();
        newFileUpload.attr("id", fileUpload.id);
        newFileUpload.attr("action", this.serverPrefix + "firmware?file="+fileUpload.fileName);
//        newFileUpload.find("#theFileName").attr("value", fileUpload.fileName)
        newFileUpload.appendTo(this.configItems);
    }

    this.setConfigRef= function(){
        this.configSection = detailedCtrSectionRef.clone();     
        this.configSection.attr("id", this.controlId);

        // configure the logout button
        if (this.controlId=="loginLogout"){
            this.configSection.find("#headerButtonLogout").hide();
        }else{
            this.configSection.find("#headerButtonLogout").attr("id", this.controlId + "HeaderButtonLogout");
        }

        // if this is login page, disable back button
        if (this.controlId=="loginLogout"){
            this.configSection.attr("data-add-back-btn","false");
//            $("#" + this.controlId + "HeaderButtonLogout").remove();
        }
        this.configSection.find("h1").text(this.controlName);
        this.configSection.insertAfter(detailedCtrSectionRef);
        this.configItems = this.configSection.find("#configControlItemsReference");
        this.configItems.children().remove();
        this.configItems.attr("id", "configControlItems");
    }

    
    this.setupControls = function(){
        this.setConfigRef();
        for (var i in this.controlArray){
            switch (this.controlArray[i].type){
            case "label":
                this.addLabel(this.controlArray[i]);
                break;
            case "input":
                this.addInput(this.controlArray[i]);
                break;
            case "button":
                this.addButton(this.controlArray[i]);
                break;
            case "toggle":
                this.addToggle(this.controlArray[i]);
                break;
            case "slider":
                this.addSlider(this.controlArray[i]);
                break;
            case "calendar":
                this.addCalendar(this.controlArray[i]);
                break;
            case "labelInput":
                this.addLabelInput(this.controlArray[i]);
                break;
            case "dualButton":
                this.addDualButton(this.controlArray[i]);
                break;
            case "radioButton":
                this.addRadioButton(this.controlArray[i]);
                break;
            case "fileUpload":
                this.addFileUpload(this.controlArray[i]);
                break;
            default:
                console.log("don't understand the type.");      
            }
        }
    }

    this.setupControls();

    this.idSearch = function(idStr){
        for (i in this.controlArray){
            if (idStr == this.controlArray[i].id){
                return this.controlArray[i];
            }
        }
        return null;
    }
    
    this.currentStatusFill = function(statusData){
        statusData = statusData.replace(/\n/g, " ").replace(/\s+/g, " ").replace(/=/g," ");
        if (statusData.charAt(0)===" ") statusData = statusData.slice(1);
        if (statusData.charAt(statusData.length-1)===" ") {
            statusData = statusData.slice(0,-1);          
        }
        var statusArray = statusData.split(" ");
        for (var i=0; i<statusArray.length; i+=2){
            // looking for osd exceptions            
            if (statusArray[i]==="osd"){
                if (statusData.indexOf("brightness")!=-1){
                    statusArray[i]="image_osd";
                }else{
                    statusArray[i]="stream_osd";
                }                
            }

            if(this.idSearch(statusArray[i]+"Input")){
                $("#"+statusArray[i]+"Input").val(statusArray[i+1]);
            }
            if(this.idSearch(statusArray[i]+"Slider")){
                $("#"+statusArray[i]+"Slider").val(statusArray[i+1]);
                $("#"+statusArray[i]+"Slider").slider("refresh");
                // console.log(statusArray[i]);
            }
            if(this.idSearch(statusArray[i]+"Toggle")){
                var toggleVal;
                if (statusArray[i+1]=='0'){
                    toggleVal=this.idSearch(statusArray[i]+"Toggle").label2;
                }else{
                    toggleVal=this.idSearch(statusArray[i]+"Toggle").label1;
                }
                $("#"+statusArray[i]+"Toggle").find("#flip-1").val(toggleVal);
                $("#"+statusArray[i]+"Toggle").find("#flip-1").slider("refresh");
            }
            if(this.idSearch(statusArray[i]+"RadioButton")){
                // exception for the flip command
                if (statusArray[i]=="flip"){
                    statusArray[i+1]="none";
                }
                // end of exception for the flip command
                var searchStr = "input[value=" + statusArray[i+1] + "]";
                var radioCheckElement = $("#" + statusArray[i]+"RadioButton").find(searchStr);
                radioCheckElement.attr("checked", true);
                $("#" + statusArray[i]+"RadioButton").find("input").checkboxradio("refresh");
            }
            if(this.idSearch(statusArray[i]+"InfoLable")){
                if (statusArray[i]=="mac_address"){
                    $("#" + statusArray[i]+"InfoLable").text("Mac Address: " + statusArray[i+1]);
                }else if (statusArray[i]=="serial_number"){
                    $("#" + statusArray[i]+"InfoLable").text("Serial Number: " + statusArray[i+1]);
                }else if (statusArray[i]=="firmware"){
                    $("#" + statusArray[i]+"InfoLable").text("Firmware: " + statusArray[i+1]);
                }else if (statusArray[i]=="uptime"){
                    $("#" + statusArray[i]+"InfoLable").text("Uptime in seconds: " + statusArray[i+1]);
                }else{
                    $("#" + statusArray[i]+"InfoLable").text(statusArray[i+1]);
                }
            }
        }
    }

    this.createSetUrl = function(){
        var urlStr="?action=set";
        for (var i in this.controlArray){
            if(this.controlArray[i].id.indexOf("Input")!=-1 ||
               this.controlArray[i].id.indexOf("Slider")!=-1){
                var newId = this.controlArray[i].id.replace("Input", "").replace("Slider", "");
                urlStr+="&"+newId + "=" + $("#" + this.controlArray[i].id).val();
            }
            if(this.controlArray[i].id.indexOf("RadioButton")!=-1){
                var newId = this.controlArray[i].id.replace("RadioButton", "");
                urlStr+="&"+newId + "=" + $("#" + this.controlArray[i].id).find("input:checked").val();
            }
            if (this.controlArray[i].id.indexOf("Toggle")!=-1 ){
                var newId = this.controlArray[i].id.replace("Toggle", "");
                // check for the osd exception
                if (newId.indexOf("osd")!=-1){
                    newId = "osd";
                }
                urlStr+="&"+newId + "=" + $("#" + this.controlArray[i].id).find("#flip-1").val();
            }
        }
        return urlStr;
    }

    this.processGetCommand = function(element, self, url, doNotFill, extraOptions, postProcessFunc){
        if (typeof doNotFill == 'object'){
            extraOptions = doNotFill;
            doNotFill = false;
        }else if(typeof doNotFill == 'function'){
            postProcessFunc = doNotFill;
            doNotFill = false;      
        }
        if (typeof extraOptions == 'function'){
            postProcessFunc = extraOptions;
            extraOptions = null;
        }
        if (typeof postProcessFunc=='undefined'){
            postProcessFunc = function(data){
//                console.log(data);
                if (data.indexOf("Error:")!=-1){
                    self.configSection.find("#commandStatus").html("Action Status: " + data);                   
                }else{
                    self.configSection.find("#commandStatus").html("Action Status: " + "OK");
//                    console.log(data);

                    if (!doNotFill){
                        self.currentStatusFill(data);
                    }
                }                     
            };
        }
        var tempUrl = url + "?action=get";
        element.click(function(){
            var optionStr = "";
            if (typeof extraOptions!='undefined' && extraOptions!=null){
                for (var i=0; i<extraOptions.length; i++){
                    var newId = extraOptions[i].id.replace("Input", "").replace("Slider", "");
                    optionStr += "&"+newId+"="+$("#" + extraOptions[i].id).val();
                }
            }
            $.get(tempUrl + optionStr, postProcessFunc);
        });
    }

    this.processSetCommand = function(element, self, url, outputEcho){
        element.click(function(){        
            $.get(url + self.createSetUrl(),
                  function(data){
                      if (data.split(" ")[0]=="Error:"){
                          self.configSection.find("#commandStatus").html("Action Status: " + data);
                      }else{
                          if (typeof outputEcho=='undefined' || outputEcho==false){
                              self.configSection.find("#commandStatus").html("Action Status: " + "OK");
                          }else{
                              self.configSection.find("#commandStatus").html("Action Status: " + data);
                          }

                          //exception for flip
                          if (url.indexOf("image")!=-1){
                              $("#imageSetDualButton").find("#buttonB").click();
                          }
                      }               
                  });
//            console.log(url + self.createSetUrl());
        });
    }

    // console.log($("#" + this.controlId + "HeaderButtonLogout"));
    $("#" + this.controlId + "HeaderButtonLogout").click(function(){        
        // navigate to the login page
        var loginPage=$("#Login").find("a");
        loginPage.click();

        // send logout command
        $("#loginoutDualButton").find("#buttonB").click();
    });
};








