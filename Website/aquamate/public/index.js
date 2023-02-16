$(document).ready(function(){
  $("a").on('click', function(event) {
    if (this.hash !== "") {
      event.preventDefault();
      var hash = this.hash;
      $('body,html').animate({
      scrollTop: $(hash).offset().top
      }, 100, function(){
      window.location.hash = hash;
     });
     } 
    });
});

var width = $(window).width();

window.onscroll = function(){
if ((width >= 900)){
    if(document.body.scrollTop > 80 || document.documentElement.scrollTop > 80) {
        $("#middle").css("background-size","150% auto");
    }else{
        $("#middle").css("background-size","100% auto");
    }
}
};

setTimeout(function(){
    $("#loading").addClass("animated fadeOut");
    setTimeout(function(){
      $("#loading").removeClass("animated fadeOut");
      $("#loading").css("display","none");
    },800);
},1450);

var n = 0;
//window.localStorage.clear();
if(localStorage.getItem("tanks_no") == null){
  n=0;
}
else{
  n=localStorage.getItem("tanks_no");
}

var btnElem = document.getElementById("Removebtn");
btnElem.setAttribute('onclick', 'removeTank(n)');

for(var i=1; i<=n; i++) {
  var newDiv = document.createElement("div");
  newDiv.className = 'group';

  var tankElem = document.createElement("button");
  tankElem.className = 'btn_two'
  tankElem.type = "button";
  tankElem.id = 'tank'+i;
  tankElem.innerHTML = 'Tank '+i+"<br/><br/>"+localStorage.getItem('tank'+i);
  tankElem.setAttribute('onclick', 'openip('+i+')');
  td=document.createElement('td');
  newDiv.appendChild(td);
  newDiv.appendChild(tankElem);

  var btnElem = document.createElement("button");
  btnElem.className = 'btn_one'
  btnElem.type = "button";
  btnElem.id = 'set'+i;
  btnElem.textContent = "Set IP";
  btnElem.setAttribute('onclick', 'setIP('+i+')');
  td=document.createElement('td');
  newDiv.appendChild(td);
  newDiv.appendChild(btnElem);

  var element = document.getElementById("tagline");
  element.appendChild(newDiv);
}

function createInput() {
  n++;
  window.localStorage.setItem("tanks_no", n);
  
  var newDiv = document.createElement("div");
  newDiv.className = 'group';

  var tankElem = document.createElement("button");
  tankElem.className = 'btn_two'
  tankElem.type = "button";
  tankElem.id = 'tank'+n;
  tankElem.innerHTML = 'Tank '+n+"<br/><br/>"+localStorage.getItem('tank'+n);
  tankElem.setAttribute('onclick', 'openip('+n+')');
  td=document.createElement('td');
  newDiv.appendChild(td);
  newDiv.appendChild(tankElem);

  var btnElem = document.createElement("button");
  btnElem.className = 'btn_one'
  btnElem.type = "button";
  btnElem.id = 'set'+n;
  btnElem.textContent = "Set IP";
  btnElem.setAttribute('onclick', 'setIP('+n+')');
  td=document.createElement('td');
  newDiv.appendChild(td);
  newDiv.appendChild(btnElem);

  var element = document.getElementById("tagline");
  element.appendChild(newDiv);
}

function removeTank() {
  var t1 = document.getElementById('tank'+n);
  var t2 = document.getElementById('set'+n);
  t1.remove();
  t2.remove();
  n--;
  window.localStorage.setItem("tanks_no", n);
}

function openip(x) {
  var ip = localStorage.getItem('tank'+x);
  window.open('http://'+ip);
}

function setIP(x) {
  var my_text=prompt('Enter IP');
  var temp_tank = document.getElementById('tank'+x);
  if(my_text) {
    window.localStorage.setItem('tank'+x, my_text);
    temp_tank.innerHTML = 'Tank '+x+"<br/><br/>"+my_text;
  }
}