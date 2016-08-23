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

function xhrRequest(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(xhr.responseText);
  };
  xhr.open(type, url);
  xhr.send();
}

exports.getWeather = function() {
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

  var weatherSettingsStr = localStorage["weatherSettings"];
  var weatherSettings;
  if(weatherSettingsStr === undefined || weatherSettingsStr === null) {
    weatherSettings = {
      "location_type": "auto"
    };
  } else {
    weatherSettings = JSON.parse(localStorage["weatherSettings"]);
  }
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

exports.saveSettings = function(location_type, location) {
  localStorage["weatherSettings"] = JSON.stringify({
    'location_type': location_type,
    'location': location,
  });
  console.log("saved weather settings:" + localStorage["weatherSettings"]);
}
