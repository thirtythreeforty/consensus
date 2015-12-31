var myAPIKey = 'cc0cec2cf4c3ec9a8199933f34ad71ec';

// Friendly names of Pebble app's icons, for KEY_WEATHER_ICON
var pebbleIcons = {
	"generic": 1,
	"sunny": 2,
	"cloudy": 3,
	"partly_cloudy": 4,
	"rain_light": 5,
	"rain_heavy": 6,
	"rain_and_snow": 7,
	"snow_light": 8,
	"snow_heavy": 9
};

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
