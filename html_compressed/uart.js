function changeUartconfig(g){g.preventDefault();
var c="/uart/config";
var b=$("#uart-baud").value;
var d=$("#uart-databits").value;
var f=$("#uart-parity").value;
var h=$("#uart-stopbits").value;
c+="?baud="+b;
c+="&databits="+d;
c+="&parity="+f;
c+="&stop="+h;
console.log(c);
hideWarning();
var a=$("#uartform-button");
addClass(a,"pure-button-disabled");
ajaxSpin("POST",c,function(e){removeClass(a,"pure-button-disabled")
},function(i,e){showWarning("Error: "+e);
removeClass(a,"pure-button-disabled");
getUartInfo()
})
}function showUartInfo(a){Object.keys(a).forEach(function(b){console.log("Item: "+b);
console.log("Data: "+a[b]);
el=$("#uart-"+b);
if(el!=null){el.value=a[b]
}})
}function getUartInfo(){ajaxJson("GET","/uart/info",showUartInfo,function(b,a){window.setTimeout(getUartInfo,1000)
})
}function isValidId(a){return a.match(/^#[a-f0-9]{6}$/i)!==null
}function changeSensorConfig(d){d.preventDefault();
var c="/sensor/config";
var b=$("#sensor-id").value;
c+="?sid="+b;
hideWarning();
var a=$("#sensorform-button");
addClass(a,"pure-button-disabled");
ajaxSpin("POST",c,function(e){removeClass(a,"pure-button-disabled")
},function(f,e){showWarning("Error: "+e);
removeClass(a,"pure-button-disabled");
getSensorInfo()
})
}function showSensorInfo(a){Object.keys(a).forEach(function(b){el=$("#sensor-"+b);
if(el!=null){if(el.nodeName==="INPUT"){el.value=a[b]
}}})
}function getSensorInfo(){ajaxJson("GET","/sensor/info",showSensorInfo,function(b,a){window.setTimeout(getSensorInfo,1000)
})
};