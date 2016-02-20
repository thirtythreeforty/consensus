var myAPIKey = 'cc0cec2cf4c3ec9a8199933f34ad71ec';

// Friendly names of Pebble app's icons, for KEY_WEATHER_ICON
var pebbleIcons = {
	"generic": 0,
	"sunny": 1,
	"cloudy": 2,
	"partly_cloudy": 3,
	"rain_light": 4,
	"rain_heavy": 5,
	"rain_and_snow": 6,
	"snow_light": 7,
	"snow_heavy": 8
};

// Map of configuration page's values for complications to integers to send
var pebbleComplicationNames = {
	"None": 0,
	"Battery": 1,
	"Date": 2,
	"Weather": 3,
	"Health": 4
}

function xhrRequest(url, type, callback) {
	var xhr = new XMLHttpRequest();
	xhr.onload = function () {
		callback(xhr.responseText);
	};
	xhr.open(type, url);
	xhr.send();
}

function getWeather(complicationPosition) {
	function makeWeatherRequest(pos) {
		// Construct URL
		var url = "http://api.openweathermap.org/data/2.5/weather?appid=" + myAPIKey + pos;

		// Send request to OpenWeatherMap
		xhrRequest(url, 'GET',
			function(responseText) {

				// Map OpenWeatherMap icons to the Pebble app's icon codes
				var iconMap = {
					1:  "sunny",
					2:  "partly_cloudy",
					3:  "cloudy",
					4:  "cloudy",
					9:  "rain_light",
					10: "rain_light",
					11: "rain_heavy",
					13: "snow_light",
				};

				// responseText contains a JSON object with weather info
				var json = JSON.parse(responseText);

				// Temperature in Kelvin requires adjustment
				var temperature = Math.round(json.main.temp - 273.15);
				console.log("Temperature is " + temperature);

				// Conditions
				var conditions = json.weather[0].main;
				console.log("Conditions are " + conditions);

				var humidity = json.main.humidity;
				console.log("Humidity is " + humidity + "%");

				// Icon
				var owmIcon = parseInt(json.weather[0].icon.substring(0, 2));
				var pebbleIcon;
				if(owmIcon in iconMap) {
					pebbleIcon = pebbleIcons[iconMap[owmIcon]];
				}
				else {
					pebbleIcon = pebbleIcons["generic"];
				}
				console.log("Icon is " + pebbleIcon)

				// Assemble dictionary using our keys
				var dictionary = {
					KEY_WEATHER_TEMP_C: temperature,
					KEY_WEATHER_CONDITIONS: conditions,
					KEY_WEATHER_HUMIDITY: humidity,
					KEY_WEATHER_ICON: pebbleIcon,
				};

				// Send to Pebble
				Pebble.sendAppMessage(dictionary,
					function(e) {
						console.log("Weather info sent to Pebble successfully!");
					},
					function(e) {
						console.log("Error sending weather info to Pebble!");
					}
				);
			}
		);
	}

	var weatherSettings = JSON.parse(localStorage["weatherSettings"]);
	if(weatherSettings !== undefined && weatherSettings !== null &&
	   weatherSettings["location_type"] === 'manual') {
		makeWeatherRequest("&zip=" + weatherSettings["location"]);
	} else {
		navigator.geolocation.getCurrentPosition(
			function locationSuccess(pos) {
				makeWeatherRequest(
					"&lat=" + pos.coords.latitude + "&lon=" + pos.coords.longitude
				);
			},
			function locationError(err) {
				console.log("Error requesting location!");
			},
			{timeout: 15000, maximumAge: 60000}
		);
	}
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', function(e) {
	console.log("PebbleKit JS ready!");
});

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage', function(e) {
	console.log("AppMessage received!");
	console.log(JSON.stringify(e.payload))

	if("KEY_WEATHER_REQUEST" in e.payload) {
		console.log("Got weather request");
		getWeather();
	}
});

Pebble.addEventListener('showConfiguration', function() {
	var url = 'https://thirtythreeforty.github.io/consensus/config/v3/';
	console.log('Showing configuration page: ' + url);

	Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
	var configStr = decodeURIComponent(e.response);
	console.log('Configuration page returned: ' + configStr);
	var configData = JSON.parse(configStr);

	// Save the needed bits to our storage
	localStorage["weatherSettings"] = JSON.stringify({
		'location_type': configData['location_type'],
		'location': configData['location'],
	});
	console.log("saved weather settings:" + localStorage["weatherSettings"]);

	// Build the config pack to send to Pebble
	function toInt(val) { return val ? 1 : 0; }
	function themeToInt(val) {
		return {
			"bold-dark": 0,
			"thin": 1
		}[val];
	}
	function complicationConfigToInts(dict, position) {
		var typeKey = {
			'Left': 'KEY_PREF_LEFT_COMPLICATION',
			'Bottom': 'KEY_PREF_BOTTOM_COMPLICATION',
			'Right': 'KEY_PREF_RIGHT_COMPLICATION'
		}[position];

		var cConfig = configData["complications"][position];
		var cType = cConfig["type"];
		var cOptions = cConfig["options"];
		dict[typeKey] = pebbleComplicationNames[cType];

		var opt1Key = typeKey + "_OPT1";
		var opt2Key = typeKey + "_OPT2";
		var opt3Key = typeKey + "_OPT3";
		var opt4Key = typeKey + "_OPT4";

		switch(cType) {
		case "None":
			break;
		case "Battery":
			break;
		case "Date":
			break;
		case "Weather":
			dict[opt1Key] = {
				"icon": 0,
				"degC": 1,
				"degF": 2,
				"relhum": 3
			}[cOptions["gadget"]];
			break;
		case "Health":
			if(cOptions["goal_type"] === "manual") {
				dict[opt1Key] = parseInt(cOptions["goal"], 10);
			} else {
				dict[opt1Key] = 0;
			}
			break;
		}
	}

	var dict = {
		KEY_PREF_THEME: themeToInt(configData['theme']),
		KEY_PREF_SHOW_SECOND_HAND: toInt(configData['show_second_hand']),
		KEY_PREF_SHOW_NO_CONNECTION: toInt(configData['show_no_connection']),
		KEY_PREF_VIBRATE_ON_HOUR: toInt(configData['vibrate_on_hour']),
		KEY_PREF_VIBRATE_ON_DISCONNECT: toInt(configData['vibrate_on_disconnect']),
		KEY_PREF_VIBRATE_ON_CONNECT: toInt(configData['vibrate_on_connect']),
		KEY_PREF_QUIET_DURING_SLEEP: toInt(configData['quiet_during_sleep']),
	};

	complicationConfigToInts(dict, 'Left');
	complicationConfigToInts(dict, 'Bottom');
	complicationConfigToInts(dict, 'Right');

	// Send to watchapp
	Pebble.sendAppMessage(dict,
		function() {
			console.log("Preferences updated successfully.");
		},
		function() {
			console.log('Preference update failed!');
		}
	);
});
