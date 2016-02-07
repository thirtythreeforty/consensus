(function() {

var keenclient = new Keen({
	projectId: "56b3b7c3d2eaaa3152913416",
	writeKey: "190551833b9b0715e11d868e62488f0ef97a8b161ce34b2bd633d79afe38618ca2bd91aeba6bce58b355e5705be3d7f378efddc5bb3e9e72de98f8b1fde5363b969b0f1225339bedb4e2f751c93b660ca4d4701d8de1ed2b458a22608bb8695e",   // String (required for sending data)
	protocol: "https",            // String (optional: https | http | auto)
});

var reportBug;
(function() {
	var capturedLogs = [];
	(function(){
		var oldLog = console.log;
		console.log = function (message) {
		 capturedLogs.push(message);
		 oldLog.apply(console, arguments);
		};
	})();

	reportBug = function(bugType, bugDetails) {
		console.log("Reporting bug type " + bugType);
		var keenBug = {
			analytics_version: 1,
			logs: capturedLogs,
			bug_type: bugType,
			bug_details: bugDetails,
			user_agent: navigator.userAgent
		};
		keenclient.addEvent("error", keenBug, function(err, res) {
			console.log("Bug reported, (err, res) = ");
			console.log(err);
			console.log(res);
		});
	}

	/*
	window.onerror = function(msg, url, line, col, error) {
		console.log("Caught error, reporting!");
		reportBug("unhandled_error", {
			message: msg,
			url: url,
			line: line,
			column: col,
			error: error
		});
		var suppressErrorAlert = true;
		return suppressErrorAlert;
	};
	*/
})();

var configureApp;
riot.compile(function() {
  // here tags are compiled and riot.mount works synchronously
  configureApp = riot.mount('#configureroot');

  configureApp[0].on('save', onSave);
});

function getConfigMapping() {
	function cfgElement(name, attrib, defval) {
		var elem = document.getElementById(name);
		return {
			'name': name,
			'defval': defval,
			'get': function() { return elem[attrib]; },
			'set': function(value) { elem[attrib] = value; },
		};
	}

	return [
		cfgElement('disp_second_hand', 'checked', false),
		cfgElement('disp_no_connection', 'checked', true),
		cfgElement('vibrate_on_hour', 'checked', true),
		cfgElement('vibrate_on_disconnect', 'checked', true),
		cfgElement('left_complication', 'value', 'Battery'),
		cfgElement('bottom_complication', 'value', 'Weather'),
		cfgElement('right_complication', 'value', 'Date')
	];
}

function loadConfigData() {
	var options;
	if(localStorage["options"]) {
		options = JSON.parse(localStorage['options']);
	} else {
		options = {};
	}
	var configMapping = getConfigMapping();
}

function getQueryParam(variable, defaultValue) {
	var query = location.search.substring(1);
	var vars = query.split('&');
	for (var i = 0; i < vars.length; i++) {
		var pair = vars[i].split('=');
		if (pair[0] === variable) {
			return decodeURIComponent(pair[1]);
		}
	}
	return defaultValue || false;
}

// Set up the 'submit' button
function onSave(pack) {
	console.log('Submit');

	var options = getConfigData();
	var encodedOptions = JSON.stringify(options);
	localStorage['options'] = encodedOptions;

	// Set the return URL depending on the runtime environment
	var return_to = getQueryParam('return_to', 'pebblejs://close#');

	// Submit analytics
	console.log("Collecting Keen stats...");

	keenStats = {
		analytics_version: 1,
		return_to: return_to,
		selected_prefs: options,
		acct_token: (typeof Pebble !== 'undefined') ? Pebble.getAccountToken() : null,
		referrer: document.referrer,
		logs: capturedLogs
	};

	// If we don't leave by, say, 7 seconds, there's an issue, send the logs to Keen
	var delayTimer = window.setTimeout(function() {
		reportBug("didnt_submit", {});
	}, 10000);

	keenclient.addEvent("configuration", keenStats, function(err, res) {
		console.log("Stat collection finished, (err, res) = " + err + " " + res);
		window.clearTimeout(delayTimer);
		document.location.href = return_to + encodeURIComponent(encodedOptions);
	});

	console.log("Started Keen request");
}

// Load any previously saved configuration, if available
loadConfigData();

})();
