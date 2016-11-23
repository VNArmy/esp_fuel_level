function changeUartconfig(e) {
  e.preventDefault();
  var url = "/uart/config";
  var baud = $("#uart-baud").value;
  var dbit = $("#uart-databits").value;
  var p = $("#uart-parity").value;
  var sb = $("#uart-stopbits").value;

  url += "?baud=" + baud;
  url += "&databits=" + dbit;
  url += "&parity=" + p;
  url += "&stop=" + sb;
  console.log(url);
  hideWarning();
  var cb = $("#uartform-button");
  addClass(cb, 'pure-button-disabled');
  ajaxSpin("POST", url, function(resp) {
      removeClass(cb, 'pure-button-disabled');
      //getWifiInfo(); // it takes 1 second for new settings to be applied
    }, function(s, st) {
      showWarning("Error: "+st);
      removeClass(cb, 'pure-button-disabled');
      getUartInfo();
    });
}
function showUartInfo(data) {
  Object.keys(data).forEach(function(v) {
    console.log("Item: " + v);
    console.log("Data: " + data[v]);
    el = $("#uart-" + v);
    if (el != null) {
      el.value = data[v];            
    }
  });  
}

function getUartInfo() {
  ajaxJson('GET', "/uart/info", showUartInfo,
      function(s, st) { window.setTimeout(getUartInfo, 1000); });
}

function isValidId(str) {
    return str.match(/^#[a-f0-9]{6}$/i) !== null;
}
//sensor config

function changeSensorConfig(e) {
  e.preventDefault();
  var url = "/sensor/config";  
  var sensorId = $("#sensor-id").value;
  url += "?sid=" + sensorId;
//  if(isValidId(sensorId)!==null)
//  {
//  	url += "?sid=" + sensorId;
//  }
//  else
//  {
//  	return;
//  }
  hideWarning();
  var cb = $("#sensorform-button");
  addClass(cb, 'pure-button-disabled');
  ajaxSpin("POST", url, function(resp) {
      removeClass(cb, 'pure-button-disabled');
      //getWifiInfo(); // it takes 1 second for new settings to be applied
    }, function(s, st) {
      showWarning("Error: "+st);
      removeClass(cb, 'pure-button-disabled');
      getSensorInfo();
    });
}

function showSensorInfo(data) {
  Object.keys(data).forEach(function(v) {
    el = $("#sensor-" + v);
    if (el != null) {
      if (el.nodeName === "INPUT") el.value = data[v];      
    }
  });  
}

function getSensorInfo() {
  ajaxJson('GET', "/sensor/info", showSensorInfo,
      function(s, st) { window.setTimeout(getSensorInfo, 1000); });
}

