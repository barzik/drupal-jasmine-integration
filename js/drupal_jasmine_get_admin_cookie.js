var system = require('system');
/**
 * Wait until the test condition is true or a timeout occurs. Useful for waiting
 * on a server response or for a ui change (fadeIn, etc.) to occur.
 *
 * @param testFx javascript condition that evaluates to a boolean,
 * it can be passed in as a string (e.g.: "1 == 1" or "$('#bar').is(':visible')" or
 * as a callback function.
 * @param onReady what to do when testFx condition is fulfilled,
 * it can be passed in as a string (e.g.: "1 == 1" or "$('#bar').is(':visible')" or
 * as a callback function.
 * @param timeOutMillis the max amount of time to wait. If not specified, 3 sec is used.
 */
function waitFor(testFx, onReady, timeOutMillis) {
    var maxtimeOutMillis = timeOutMillis ? timeOutMillis : 1000000001, //< Default Max Timeout is 3s
        start = new Date().getTime(),
        condition = false,
        interval = setInterval(function() {
            if ( (new Date().getTime() - start < maxtimeOutMillis) && !condition ) {
                // If not time-out yet and condition not yet fulfilled
                condition = (typeof(testFx) === "string" ? eval(testFx) : testFx()); //< defensive code
            } else {
                if(!condition) {
                    // If condition still not fulfilled (timeout but condition is 'false')
                    console.log("'waitFor()' timeout");
                    phantom.exit(1);
                } else {
                    // Condition fulfilled (timeout and/or condition is 'true')
                    typeof(onReady) === "string" ? eval(onReady) : onReady(); //< Do what it's supposed to do once the condition is fulfilled
                    clearInterval(interval); //< Stop this interval
                }
            }
        }, 100); //< repeat check every 100ms
};

if (system.args.length !== 4) {
    console.log('Usage: hpln_jasmine_get_admin_cookie.js drupal_url + User + PWD');
    phantom.exit(1);
}

var page = require('webpage').create();


var drupal_url = system.args[1];
var drupal_username=  system.args[2];
var drupal_password=  system.args[3];
var page = new WebPage(), testindex = 0, loadInProgress = false;

page.onConsoleMessage = function(msg) {
  console.log(msg);
};

page.onLoadStarted = function() {
  loadInProgress = true;
  console.log("load started");
};

page.onLoadFinished = function() {
  loadInProgress = false;
  console.log("load finished");
};

/*
 * This function wraps WebPage.evaluate, and offers the possibility to pass
 * parameters into the webpage function. The PhantomJS issue is here:
 * 
 *   http://code.google.com/p/phantomjs/issues/detail?id=132
 * 
 * This is from comment #43.
 */
function evaluate(page, func) {
    var args = [].slice.call(arguments, 2);
    var fn = "function() { return (" + func.toString() + ").apply(this, " + JSON.stringify(args) + ");}";
    return page.evaluate(fn);
}

var steps = [
  function() {
    //Load Login Page
    page.open(drupal_url);
  },
  function() {
    //Login
    evaluate(page, function(drupal_username, drupal_password) {
		var form = document.getElementById('user-login');
		console.log("password : "+drupal_password);
		console.log("name : "+drupal_username);
		document.getElementById('edit-name').value=drupal_username;
		document.getElementById('edit-pass').value =drupal_password;
		form.submit(); 
    }, drupal_username, drupal_password);
  }, 
  function() {
    // If logged in - return 0. if not an error
    page.evaluate(function() {
   // console.log(document.querySelectorAll('html')[0].outerHTML);
                if (document.getElementsByClassName('logged-in').length == 1){
                    console.log('Session captured');
                    return 0;
                } else {
                    console.log('Session was not captured');
                    return 1;
                }
	  
	  
    });
  }
];


interval = setInterval(function() {
  if (!loadInProgress && typeof steps[testindex] == "function") {
    console.log("step " + (testindex + 1));
    steps[testindex]();
    testindex++;
  }
  if (typeof steps[testindex] != "function") {
    console.log("test complete!");
    phantom.exit();
  }
}, 50);
