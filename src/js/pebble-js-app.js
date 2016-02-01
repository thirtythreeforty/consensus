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
	"Weather": 2,
	"Date": 3,
	"Health": 4
}

function xhrRequest(url, type, callback) {
	var xhr = new XMLHttpRequest();
	xhr.onload = function () {
		callback(this.responseText);
	};
	xhr.open(type, url);
	xhr.send();
}

function getWeather() {
	function locationSuccess(pos) {
		// Construct URL
		var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
			pos.coords.latitude + "&lon=" + pos.coords.longitude + '&appid=' + myAPIKey;

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

	function locationError(err) {
		console.log("Error requesting location!");
	}

	navigator.geolocation.getCurrentPosition(
		locationSuccess,
		locationError,
		{timeout: 15000, maximumAge: 60000}
	);
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', function(e) {
	console.log("PebbleKit JS ready!");
});

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage', function(e) {
	console.log("AppMessage received!");
	console.log(JSON.stringify(e.payload))
	console.log(e.payload.KEY_WEATHER_ICON)

	if("KEY_WEATHER_REQUEST" in e.payload) {
		console.log("Got weather request");
		getWeather();
	}
});

Pebble.addEventListener('showConfiguration', function() {
	var url = 'https://thirtythreeforty.github.io/consensus/config/';
	console.log('Showing configuration page: ' + url);

	Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
	var configStr = decodeURIComponent(e.response);
	console.log('Configuration page returned: ' + configStr);
	var configData = JSON.parse(configStr);

	function toInt(val) { return val ? 1 : 0; }
	function complicationToInt(val) {
		return pebbleComplicationNames[val];
	}

	var dict = {
		KEY_PREF_SHOW_SECOND_HAND: toInt(configData['disp_second_hand']),
		KEY_PREF_SHOW_NO_CONNECTION: toInt(configData['disp_no_connection']),
		KEY_PREF_VIBRATE_ON_HOUR: toInt(configData['vibrate_on_hour']),
		KEY_PREF_VIBRATE_ON_DISCONNECT: toInt(configData['vibrate_on_disconnect']),
		KEY_PREF_LEFT_COMPLICATION: complicationToInt(configData['left_complication']),
		KEY_PREF_BOTTOM_COMPLICATION: complicationToInt(configData['bottom_complication']),
		KEY_PREF_RIGHT_COMPLICATION: complicationToInt(configData['right_complication'])
	};

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
