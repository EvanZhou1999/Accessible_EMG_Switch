// PROGMEM: Store data in flash (program) memory instead of SRAM, 

const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<html>
    <head>
        <Title>Accessible EMG Switch</Title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <link rel="icon" href="data:,">
        <style>
            html {
                font-family: Arial, Helvetica, sans-serif;
                text-align: center;
            }
            h1 {
              font-size: 1.8rem;
              color: white;
            }
            h2{
              font-size: 1.5rem;
              font-weight: bold;
              color: #046EC4;
            }
            .topnav {
                overflow: hidden;
                background-color: #333333;
            }
            body {
                margin: 0;
            }
            .content {
                padding: 30px;
                max-width: 600px;
                margin: 0 auto;
            }
            .card {
                background-color: #F8F7F9;;
                box-shadow: 2px 2px 12px 1px rgba(140, 140, 140, .5);
                padding-top: 10px;
                padding-bottom: 20px;
                border-radius: 30px;
            }
            .button {
                padding: 15px 50px;
                font-size: 24px;
                text-align: center;
                outline: none;
                color: #fff;
                background-color: #046EC4;
                border: none;
                border-radius: 15px;
                -webkit-touch-callout: none;
                -webkit-user-select: none;
                -khtml-user-select: none;
                -moz-user-select: none;
                -ms-user-select: none;
                user-select: none;
                -webkit-tap-highlight-color: rgba(0,0,0,0);
            }
            .button:active {
                background-color: #458fc4;
                box-shadow: 2 2px #CDCDCD;
                transform: translateY(2px);
            }
            .state {
                font-size: 1.5rem;
                color: #8c8c8c;
                font-weight: bold
            }
            
            .action {
                font-size: 1.5rem;
                color: #046EC4;
                font-weight: bold
            }
            
            input[type=number] {
                padding: 5px 25px;
                font-size: 24px;
                border: 2px solid grey;
                border-radius: 15px;
                box-shadow: 10 10px #000000;
            }
            
            a {
                text-decoration: none;
                color: #fff;
                
            }
            
        </style>
    </head>
        
    <body onload="updateSetting();">
        <!-- Div for the top title bar -->
        <div class="topnav">
            <h1>Accessible EMG Switch<h1>
        </div>
        <div style="background: #1AA7EC; height: 10px;"></div>
        <!-- Div for the body of the page -->
        <div class="content">
            <div class="card">
                <p class="state">
                    This page is for settings and configurations.
                </p>
            </div>
            <br>
            <br>
            <div class="card">
                <h2>EMG Sensor Settings</h2>
                <p class="state">Current Reading: <span id="emgreading">%READING%</span></p>
                <p class="state">Current Threshold: <span id="thresholdsetting">%THRESHOLD%</span></p>
                
                
                <label class="action" for="thresholdinput">Adjust Threshold Value:</label>
                <input type="number" id="thresholdinput" name="threshold" min="1" max="4095">
                <p>
                    <button id="button_UPDATE_THRESH" class="button">Update Threshold</button>
                </p>
                

            </div>
            <br>
            <br>
            <div class="card">
                <h2>LED Control</h2>
                <p>
                    <button id="button_pink" class="button" style="padding: 15px 30px; background-color: #FFAAAA;" onclick="changeColor(100, 10, 10);">
                        Pink
                    </button>
                    <button id="button_red" class="button" style="padding: 15px 30px; background-color: #FF3333;" onclick="changeColor(100, 0, 0);">
                        Red
                    </button>
                    <button id="button_orange" class="button" style="padding: 15px 20px; background-color: #FF7700;" onclick="changeColor(100, 20, 0);">
                        Orange
                    </button>
                </p>
                <p>
                    <button id="button_yellow" class="button" style="padding: 15px 20px; background-color: #FFBB00;" onclick="changeColor(100, 80, 0);">
                        Yellow
                    </button>
                    <button id="button_green" class="button" style="padding: 15px 20px; background-color: #00BB55;" onclick="changeColor(10, 100, 10);">
                        Green
                    </button>
                    <button id="button_cyan" class="button" style="padding: 15px 30px; background-color: #00DDDD;" onclick="changeColor(0, 40, 40);">
                        Cyan
                    </button>
                </p>
                <p>
                    <button id="button_blue" class="button" style="padding: 15px 30px; background-color: #046EC4;" onclick="changeColor(0, 0, 100);">
                        Blue
                    </button>
                    <button id="button_purple" class="button" style="padding: 15px 20px; background-color: #CC00FF;" onclick="changeColor(60, 0, 60);">
                        Purple
                    </button>
                    <button id="button_white" class="button" style="padding: 15px 20px; background-color: #FFFFFF; border: 2px solid grey; color: #555555;" onclick="changeColor(50, 50, 50);">
                        White
                    </button>
                </p>
                <p>
                    <button id="button_blue" class="button" style="padding: 15px 30px; background-color: #000000;" onclick="changeColor(0, 0, 0);">
                        Black (Off)
                    </button>
                </p>
            </div>
            <br>
            <br>
            <div class="card">
                <h2>Vibration Control</h2>
                <p class="state">Device Vibration: <span id="vibrationsetting">%VIBSET%</span></p>
                <p>
                    <button id="button_enable" class="button" style="padding: 15px 40px; background-color: #00BB55;">Enable</button>
                    <button id="button_disable" class="button" style="padding: 15px 40px; background-color: #FF3333;">Disable</button>
                </p>
            </div>
            <br>
            <br>
            <div class="card">
                <h2>Test Control</h2>
                <p class="state">Try here to test the sensors:</p>
                
                <p>
                    <button id="button_ACT" class="button">Activate Switch</button>
                </p>
                <p>
                    <button id="button_VIB" class="button">Vibration Motor </button>
                </p>

            </div>
            <br>
            <br>
            <div class="card">
                <h2>Device Documentations</h2>
                <p class="state">Get help here</p>
                
                
                <button id="button_help" class="button" onclick="window.location.href='https://drive.google.com/drive/folders/1nD83zP2qYV-7mvF2n2yecVb5Hgfm7Zpj?usp=share_link';">
                    User Manual
                </button>
                <br>
                <br>
                <button id="button_help" class="button" onclick="window.location.href='https://github.com/EvanZhou1999';">
                    Source Code
                </button>
                

            </div>
            
        </div>
        
        <script>
            setInterval(function() {
                        // Call a functin repeatedly with 2 Second interval
                        getEMGreading();
                    }, 250); // 500 milli Seconds update rate

            document.getElementById('button_ACT').addEventListener('click', switchClick);
            document.getElementById('button_VIB').addEventListener('click', activateMotor);
            document.getElementById('button_UPDATE_THRESH').addEventListener('click', updateThreshold);
            document.getElementById('button_enable').addEventListener('click', enableVibration);
            document.getElementById('button_disable').addEventListener('click', disableVibration);
            
            function updateSetting() {
                getThreshold();
                getVibrationStatus()
            }
            
            function getEMGreading() {
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function() {
                    if (this.readyState == 4 && this.status == 200) {
                        document.getElementById("emgreading").innerHTML = this.responseText;
                    }
                };
                xhttp.open("GET", "reading/emg", true);
                xhttp.send();
            }
            
            function switchClick() {
                // Send request
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function() {};
                const path = "control/click";
                xhttp.open("GET", path, true);
                xhttp.send();
              }
            
            function activateMotor() {
                // Send request
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function() {};
                const path = "control/motor";
                xhttp.open("GET", path, true);
                xhttp.send();
            }
            
            function updateThreshold() {
                if (document.getElementById('thresholdinput').checkValidity()){
                    var userinput = document.getElementById('thresholdinput').value;
                    if (userinput == ""){
                        alert("Field Empty, Please enter a number");
                    }else{
                        console.log(userinput);
                        // Send Request
                        var xhttp = new XMLHttpRequest();
                        xhttp.onreadystatechange = function() {
                            getThreshold();
                        };
                        xhttp.open("GET", "set/threshold?value="+userinput, true);
                        xhttp.send();
                    }
                    
                }else{
                    alert("Please enter threshold as a number between 1 to 4095");
                }
            }
            
            function getThreshold() {
                console.log("sending request for getting the threshold");
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function() {
                    if (this.readyState == 4 && this.status == 200) {
                        document.getElementById("thresholdsetting").innerHTML = this.responseText;
                    }
                };
                xhttp.open("GET", "getsetting/threshold", true);
                xhttp.send();
            }
            
            function getVibrationStatus() {
                console.log("sending request for getting the vibration status");
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function() {
                    if (this.readyState == 4 && this.status == 200) {
                        document.getElementById("vibrationsetting").innerHTML = this.responseText;
                    }
                };
                xhttp.open("GET", "getsetting/vibration", true);
                xhttp.send();
            }
            
            function enableVibration() {
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function() {
                    getVibrationStatus();
                };
                xhttp.open("GET", "set/vibration?state=1", true);
                xhttp.send();
            }
            
            function disableVibration() {
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function() {
                    getVibrationStatus();
                };
                xhttp.open("GET", "set/vibration?state=0", true);
                xhttp.send();
            }
            
            function changeColor(red, green, blue) {
                var url = "set/indicator?red="+red+"&green="+green+"&blue="+blue;
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function() {};
                xhttp.open("GET", url, true);
                xhttp.send();
            }

        </script>
    </body>
</html>


)=====";
