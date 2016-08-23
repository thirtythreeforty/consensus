var weather = require('./weather')

var Clay = require('./pebble-clay');
var config = require('./config');
var clay = new Clay(config.config, require('./customfn'), { autoHandleEvents: false });

// Map of configuration page's values for complications to integers to send
var pebbleComplicationNames = {
  "None": 0,
  "Battery": 1,
  "Date": 2,
  "Weather": 3,
  "Health": 4,
  "Time Zone": 5
}

function handleSettingsString(configStr) {
  var configData = JSON.parse(configStr);

  // Build the config pack to send to Pebble
  function toInt(val) { return val ? 1 : 0; }
  function themeToInt(val) {
    return {
      "bold-dark": 0,
      "thin": 1,
      "bold-white": 2,
      "minimal-dark": 3,
      "minimal-white": 4
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
      dict[opt1Key] = {
        "day": 0,
        "month": 1,
        "none": 2,
      }[cOptions["gadget"]];
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
      dict[opt2Key] = {
        "icon": 0,
        "distance": 2,
        "kcal_act": 3,
        "kcal_rest": 4,
        "kcal_tot": 5,
        "act_sec": 6
      }[cOptions["gadget"]];
      break;
    case "Time Zone":
      // Compute the number of seconds' offset
      // Because we send unsigned values to the Pebble, also have to send
      // a "negative" flag.  There are ways around this that would be
      // fragile and that I don't want to deal with.
      var hour_offset = parseFloat(cOptions["offset"], 10);
      if(hour_offset < 0) {
        dict[opt2Key] = 1;
        hour_offset = -hour_offset;
      }
      else {
        dict[opt2Key] = 0;
      }
      var second_offset = hour_offset * 60 * 60;
      dict[opt1Key] = second_offset;
    }
  }

  var dict = {
    KEY_PREF_THEME: themeToInt(configData['theme']),
    KEY_PREF_SHOW_SECOND_HAND: toInt(configData['show_second_hand']),
    KEY_PREF_SHOW_ANIMATIONS: toInt(configData['show_animations']),
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
    weather.getWeather();
  }

  if("KEY_PREFS_REQUEST" in e.payload) {
    console.log("Got first-run settings request");

    var configStr = localStorage["allSettings"];
    if(configStr === undefined || configStr === null) {
      console.log("Saved settings do not exist!");
      Pebble.sendAppMessage({ KEY_PREFS_DONT_EXIST: 0, }, null, null);
    } else {
      handleSettingsString(configStr);
    }
  }
});

Pebble.addEventListener('ready', function() {
  console.log(clay.generateUrl());
});

Pebble.addEventListener('showConfiguration', function() {
  var url = clay.generateUrl();
  console.log("Opening URL:")
  console.log(url)
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  console.log("Got settings: ", clay.getSettings(e.response));
  /*
  var configStr = decodeURIComponent(e.response);
  console.log('Configuration page returned: ' + configStr);
  var configData = JSON.parse(configStr);

  localStorage["allSettings"] = configStr;
  console.log("saved settings pack");

  // Save the needed bits to our storage
  weather.saveSettings(configData['location_type'], configData['location']);

  handleSettingsString(configStr);
  */
});
