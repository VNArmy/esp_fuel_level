var bnd=function(c,b,a){c.addEventListener(b,a,false)
};
var m=function(f,d,g){d=document;
g=d.createElement("p");
g.innerHTML=f;
f=d.createDocumentFragment();
while(d=g.firstChild){f.appendChild(d)
}return f
};
var $=function(d,c){d=d.match(/^(\W)?(.*)/);
return(c||document)["getElement"+(d[1]?d[1]=="#"?"ById":"sByClassName":"sByTagName")](d[2])
};
var j=function(b){for(b=0;
b<4;
b++){try{return b?new ActiveXObject([,"Msxml2","Msxml3","Microsoft"][b]+".XMLHTTP"):new XMLHttpRequest
}catch(c){}}};
function domForEach(b,a){return Array.prototype.forEach.call(b,a)
}e=function(b){return document.createElement(b)
};
function onLoad(b){var a=window.onload;
if(typeof a!="function"){window.onload=b
}else{window.onload=function(){a();
b()
}
}}function addClass(b,a){b.className+=" "+a
}function removeClass(f,c){var b=f.className.split(/\s+/),a=b.length;
for(var d=0;
d<a;
d++){if(b[d]===c){b.splice(d,1)
}}f.className=b.join(" ");
return b.length!=a
}function toggleClass(b,a){if(!removeClass(b,a)){addClass(b,a)
}}function ajaxReq(h,a,g,c){var f=j();
f.open(h,a,true);
var d=setTimeout(function(){f.abort();
console.log("XHR abort:",h,a);
f.status=599;
f.responseText="request time-out"
},9000);
f.onreadystatechange=function(){if(f.readyState!=4){return
}clearTimeout(d);
if(f.status>=200&&f.status<300){g(f.responseText)
}else{console.log("XHR ERR :",h,a,"->",f.status,f.responseText,f);
c(f.status,f.responseText)
}};
try{f.send()
}catch(b){console.log("XHR EXC :",h,a,"->",b);
c(599,b)
}}function dispatchJson(f,d,c){var a;
try{a=JSON.parse(f)
}catch(b){console.log("JSON parse error: "+b+". In: "+f);
c(500,"JSON parse error: "+b);
return
}d(a)
}function ajaxJson(d,a,c,b){ajaxReq(d,a,function(f){dispatchJson(f,c,b)
},b)
}function ajaxSpin(d,a,c,b){$("#spinner").removeAttribute("hidden");
ajaxReq(d,a,function(f){$("#spinner").setAttribute("hidden","");
c(f)
},function(f,g){$("#spinner").setAttribute("hidden","");
b(f,g)
})
}function ajaxJsonSpin(d,a,c,b){ajaxSpin(d,a,function(f){dispatchJson(f,c,b)
},b)
}function hidePopup(a){addClass(a,"popup-hidden");
addClass(a.parentNode,"popup-target")
}onLoad(function(){var a=$("#layout");
var d=a.childNodes[0];
a.insertBefore(m('<div id="spinner" class="spinner" hidden></div>'),d);
a.insertBefore(m('<div id="messages"><div id="warning" hidden></div><div id="notification" hidden></div></div>'),d);
a.insertBefore(m('<a href="#menu" id="menuLink" class="menu-link"><span></span></a>'),d);
var c=m('<div id="menu">      <div class="pure-menu">        <a class="pure-menu-heading" href="https://github.com/jeelabs/esp-link">        <img src="/favicon.ico" height="32">&nbsp;esp-link</a>        <div class="pure-menu-heading system-name" style="padding: 0px 0.6em"></div>        <ul id="menu-list" class="pure-menu-list"></ul>      </div>    </div>    ');
a.insertBefore(c,d);
var f=$("#menuLink"),c=$("#menu");
bnd(f,"click",function(h){var g="active";
h.preventDefault();
toggleClass(a,g);
toggleClass(c,g);
toggleClass(f,g)
});
domForEach($(".popup"),function(g){hidePopup(g)
});
var b=function(){ajaxJson("GET","/menu",function(n){var l="",o=window.location.pathname;
for(var k=0;
k<n.menu.length;
k+=2){var h=n.menu[k+1];
l=l.concat(' <li class="pure-menu-item'+(o===h?" pure-menu-selected":"")+'"><a href="'+h+'" class="pure-menu-link">'+n.menu[k]+"</a></li>")
}$("#menu-list").innerHTML=l;
var g=$("#version");
if(g!=null){g.innerHTML=n.version
}setEditToClick("system-name",n.name)
},function(){setTimeout(b,1000)
})
};
b()
});
function showWifiInfo(b){Object.keys(b).forEach(function(c){el=$("#wifi-"+c);
if(el!=null){if(el.nodeName==="INPUT"){el.value=b[c]
}else{el.innerHTML=b[c]
}}});
var a=$("#dhcp-r"+b.dhcp);
if(a){a.click()
}$("#wifi-spinner").setAttribute("hidden","");
$("#wifi-table").removeAttribute("hidden");
currAp=b.ssid
}function getWifiInfo(){ajaxJson("GET","/wifi/info",showWifiInfo,function(b,a){window.setTimeout(getWifiInfo,1000)
})
}function setEditToClick(a,b){domForEach($("."+a),function(c){if(c.children.length>0){domForEach(c.children,function(d){if(d.nodeName==="INPUT"){d.value=b
}else{if(d.nodeName!=="DIV"){d.innerHTML=b
}}})
}else{c.innerHTML=b
}})
}function showSystemInfo(a){Object.keys(a).forEach(function(b){setEditToClick("system-"+b,a[b])
});
$("#system-spinner").setAttribute("hidden","");
$("#system-table").removeAttribute("hidden");
currAp=a.ssid
}function getSystemInfo(){ajaxJson("GET","/system/info",showSystemInfo,function(b,a){window.setTimeout(getSystemInfo,1000)
})
}function makeAjaxInput(a,b){domForEach($("."+a+"-"+b),function(i){var f=$(".edit-on",i);
var d=$(".edit-off",i)[0];
var c="/"+a+"/update?"+b;
if(d===undefined||f==undefined){return
}var h=function(){d.setAttribute("hidden","");
domForEach(f,function(k){k.removeAttribute("hidden")
});
f[0].select();
return false
};
var g=function(k){ajaxSpin("POST",c+"="+k,function(){domForEach(f,function(l){l.setAttribute("hidden","")
});
d.removeAttribute("hidden");
setEditToClick(a+"-"+b,k);
showNotification(b+" changed to "+k)
},function(){showWarning(b+" change failed")
});
return false
};
bnd(d,"click",function(){return h()
});
bnd(f[0],"blur",function(){return g(f[0].value)
});
bnd(f[0],"keyup",function(k){if((k||window.event).keyCode==13){return g(f[0].value)
}})
})
}function showWarning(b){var a=$("#warning");
a.innerHTML=b;
a.removeAttribute("hidden");
window.scrollTo(0,0)
}function hideWarning(){el=$("#warning").setAttribute("hidden","")
}var notifTimeout=null;
function showNotification(b){var a=$("#notification");
a.innerHTML=b;
a.removeAttribute("hidden");
if(notifTimeout!=null){clearTimeout(notifTimeout)
}notifTimout=setTimeout(function(){a.setAttribute("hidden","");
notifTimout=null
},4000)
};